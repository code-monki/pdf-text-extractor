// File: src/core/review_workflow.cpp
// Purpose: Implement review transition and filtered next-page workflow helpers.
// Architectural context: HLA-REVIEW, HLA-APP, HLA-SAFE.
// Requirement references: FR-017, FR-018, FR-021, FR-022, FR-027, NFR-005,
// NFR-007, NFR-012.
// Test references: TC-FR-017, TC-FR-018, TC-FR-021, TC-FR-022, TC-FR-027,
// TC-NFR-005, TC-NFR-007, TC-NFR-012.
// Constraint: Safe messages omit reviewed page text and candidate text.

#include "core/review_workflow.hpp"

#include "core/review_state.hpp"

#include <algorithm>

namespace pte::core {

namespace {

/**
 * @brief Reports whether a status is included by a filter.
 */
bool matchesFilter(const ReviewFilter& filter, const std::string& status) {
    if (filter.statuses.empty()) {
        return true;
    }
    return std::find(filter.statuses.begin(), filter.statuses.end(), status)
        != filter.statuses.end();
}

} // namespace

/**
 * @brief Applies an allowed review transition to an in-memory state.
 */
ReviewWorkflowResult ReviewWorkflowService::transition(
    const ReviewPageState& current,
    const std::string& targetStatus) const {
    ReviewWorkflowResult result;
    result.state = current;
    if (!isAllowedReviewTransition(current.status, targetStatus)) {
        result.safeMessage = "review transition is not allowed";
        return result;
    }

    result.state.status = targetStatus;
    result.state.dirtyState = "dirty";
    result.success = true;
    result.safeMessage = "review state transitioned";
    return result;
}

/**
 * @brief Finds the next page after the supplied page ID that matches a status filter.
 */
std::optional<std::string> ReviewWorkflowService::nextPage(
    const std::vector<ReviewPageState>& pages,
    const ReviewFilter& filter,
    const std::string& fromPageId) const {
    bool passedStart = fromPageId.empty();
    for (const auto& page : pages) {
        if (!passedStart) {
            passedStart = page.pageId == fromPageId;
            continue;
        }
        if (matchesFilter(filter, page.status)) {
            return page.pageId;
        }
    }
    return std::nullopt;
}

} // namespace pte::core
