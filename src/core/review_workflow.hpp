// File: src/core/review_workflow.hpp
// Purpose: Manage review page state transitions and filtered next-page lookup.
// Architectural context: HLA-REVIEW, HLA-APP, HLA-SAFE.
// Requirement references: FR-017, FR-018, FR-021, FR-022, FR-027, NFR-005,
// NFR-007, NFR-012.
// Test references: TC-FR-017, TC-FR-018, TC-FR-021, TC-FR-022, TC-FR-027,
// TC-NFR-005, TC-NFR-007, TC-NFR-012.
// Constraint: Review workflow changes review metadata only and never reads page text content.

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief In-memory review state for one page.
 */
struct ReviewPageState {
    std::string pageId;
    int pdfPage = 0;
    std::string status = "raw";
    std::string selectedSource = "empty";
    std::string dirtyState = "clean";
    std::string notes;
};

/**
 * @brief Review status filter for next-page lookup.
 */
struct ReviewFilter {
    std::vector<std::string> statuses;
};

/**
 * @brief Result for review state mutations.
 */
struct ReviewWorkflowResult {
    bool success = false;
    ReviewPageState state;
    std::string safeMessage;
};

/**
 * @brief Service for deterministic review-state workflow helpers.
 */
class ReviewWorkflowService {
public:
    /**
     * @brief Applies an allowed review status transition to an in-memory page state.
     *
     * @param current Current page review state.
     * @param targetStatus Desired target status.
     * @return Updated state or safe failure.
     * @post Candidate source names cannot be used as review statuses.
     * Side effects: None.
     * Determinism: Deterministic for supplied state and target.
     */
    ReviewWorkflowResult transition(const ReviewPageState& current,
                                    const std::string& targetStatus) const;

    /**
     * @brief Finds the next page matching a review filter.
     *
     * @param pages Ordered page review states.
     * @param filter Status filter. Empty filter matches all pages.
     * @param fromPageId Page ID to search after. Empty starts before first page.
     * @return Matching page ID, or empty optional when no later page matches.
     * Side effects: None.
     * Determinism: Deterministic for supplied page order and filter.
     */
    std::optional<std::string> nextPage(const std::vector<ReviewPageState>& pages,
                                        const ReviewFilter& filter,
                                        const std::string& fromPageId) const;
};

} // namespace pte::core
