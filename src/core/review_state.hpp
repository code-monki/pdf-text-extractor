// File: src/core/review_state.hpp
// Purpose: Define review statuses and default release eligibility behavior.
// Architectural context: HLA-REVIEW, OCR-BND-001.
// Requirement references: FR-017, FR-018.
// Test references: TC-FR-017, TC-FR-018, TC-BND-OCR-001.
// Constraint: Candidate source output cannot make a page release eligible without accepted
// review state.

#pragma once

#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Checks whether a review status is part of the approved workflow vocabulary.
 *
 * @param status Candidate status string from review-state artifacts.
 * @return true when the status is known and allowed.
 * Side effects: None.
 * Determinism: Deterministic for a given status string.
 */
bool isAllowedReviewStatus(const std::string& status);

/**
 * @brief Checks whether a status is release eligible by default.
 *
 * @param status Candidate status string from review-state artifacts.
 * @return true only for accepted review state.
 * @post OCR or embedded candidates remain non-authoritative unless review state is accepted.
 * Side effects: None.
 * Determinism: Deterministic for a given status string.
 */
bool isDefaultReleaseEligibleStatus(const std::string& status);

/**
 * @brief Checks whether a review status transition is approved by the workflow rules.
 *
 * @param from Current review status.
 * @param to Requested target review status.
 * @return true when both statuses are allowed and the transition is permitted.
 * Side effects: None.
 * Determinism: Deterministic for a given status pair.
 */
bool isAllowedReviewTransition(const std::string& from, const std::string& to);

/**
 * @brief Release eligibility result for one reviewed page state.
 */
struct ReleaseEligibility {
    bool eligible = false;
    std::string reason;
};

/**
 * @brief Counts default release eligibility across review statuses.
 */
struct ReleaseEligibilitySummary {
    int eligibleCount = 0;
    int ineligibleCount = 0;
    int invalidStatusCount = 0;
};

/**
 * @brief Computes default release eligibility for one review status.
 *
 * @param status Review status to evaluate.
 * @return Eligibility result with safe reason code.
 * Side effects: None.
 * Determinism: Deterministic for a given status.
 */
ReleaseEligibility computeReleaseEligibility(const std::string& status);

/**
 * @brief Summarizes default release eligibility for a collection of review statuses.
 *
 * @param statuses Review statuses to summarize.
 * @return Counts of eligible, ineligible, and invalid statuses.
 * Side effects: None.
 * Determinism: Deterministic for a given status collection.
 */
ReleaseEligibilitySummary summarizeReleaseEligibility(
    const std::vector<std::string>& statuses);

} // namespace pte::core
