// File: src/core/review_state.cpp
// Purpose: Implement deterministic review status gates.
// Architectural context: HLA-REVIEW, OCR-BND-001.
// Requirement references: FR-017, FR-018.
// Test references: TC-FR-017, TC-FR-018, TC-BND-OCR-001.

#include "core/review_state.hpp"

#include <array>

namespace pte::core {

/**
 * @brief Checks the review status vocabulary approved for current workflow artifacts.
 *
 * Unknown statuses are rejected so raw OCR or malformed state cannot silently enter release flow.
 */
bool isAllowedReviewStatus(const std::string& status) {
    static constexpr std::array<const char*, 6> allowed = {
        "raw", "editing", "edited", "accepted", "skip", "quarantined"
    };
    for (const char* candidate : allowed) {
        if (status == candidate) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Identifies statuses that are release eligible without additional override logic.
 *
 * Only accepted pages are eligible by default, preserving OCR-BND-001 review containment.
 */
bool isDefaultReleaseEligibleStatus(const std::string& status) {
    return status == "accepted";
}

/**
 * @brief Checks whether a review transition follows the approved workflow.
 */
bool isAllowedReviewTransition(const std::string& from, const std::string& to) {
    if (!isAllowedReviewStatus(from) || !isAllowedReviewStatus(to)) {
        return false;
    }
    if (from == "raw") {
        return to == "editing" || to == "accepted" || to == "skip" || to == "quarantined";
    }
    if (from == "editing") {
        return to == "edited" || to == "accepted" || to == "skip" || to == "quarantined";
    }
    if (from == "edited") {
        return to == "accepted" || to == "editing" || to == "skip" || to == "quarantined";
    }
    if (from == "accepted") {
        return to == "editing" || to == "skip" || to == "quarantined";
    }
    if (from == "skip") {
        return to == "editing" || to == "accepted" || to == "quarantined";
    }
    if (from == "quarantined") {
        return to == "editing" || to == "skip";
    }
    return false;
}

/**
 * @brief Computes default release eligibility for one status.
 */
ReleaseEligibility computeReleaseEligibility(const std::string& status) {
    if (!isAllowedReviewStatus(status)) {
        return {false, "invalid-status"};
    }
    if (isDefaultReleaseEligibleStatus(status)) {
        return {true, "accepted"};
    }
    return {false, "not-accepted"};
}

/**
 * @brief Summarizes default release eligibility across status values.
 */
ReleaseEligibilitySummary summarizeReleaseEligibility(
    const std::vector<std::string>& statuses) {
    ReleaseEligibilitySummary summary;
    for (const auto& status : statuses) {
        const auto eligibility = computeReleaseEligibility(status);
        if (!isAllowedReviewStatus(status)) {
            ++summary.invalidStatusCount;
        } else if (eligibility.eligible) {
            ++summary.eligibleCount;
        } else {
            ++summary.ineligibleCount;
        }
    }
    return summary;
}

} // namespace pte::core
