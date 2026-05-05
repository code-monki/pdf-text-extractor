// File: src/core/validation_repair.hpp
// Purpose: Plan safe structural repairs from validation findings.
// Architectural context: HLA-VALID with HLA-WORK, HLA-SAFE.
// Requirement references: FR-025, FR-026, NFR-007, NFR-012.
// Test references: TC-FR-025, TC-FR-026, TC-NFR-007, TC-NFR-012.
// Constraint: This slice plans dry-run repair actions only and does not modify artifacts.

#pragma once

#include "core/work_folder_validator.hpp"

#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Non-destructive repair action proposed from one validation finding.
 */
struct RepairAction {
    std::string actionCode;
    std::string findingCode;
    std::string artifact;
    std::string pageId;
    std::string safeMessage;
};

/**
 * @brief Dry-run repair plan derived from a validation report.
 */
struct RepairPlan {
    std::vector<RepairAction> actions;
    int nonRepairableCount = 0;
    bool dryRunOnly = true;

    /**
     * @brief Reports whether the plan contains at least one proposed action.
     *
     * @return true when actions is not empty.
     * Side effects: None.
     * Determinism: Deterministic for the stored plan.
     */
    bool hasActions() const;
};

/**
 * @brief Creates safe dry-run repair plans from validation findings.
 */
class ValidationRepairPlanner {
public:
    /**
     * @brief Plans safe structural repair actions without modifying files.
     *
     * @param report Validation report to inspect.
     * @return Dry-run repair plan containing safe metadata-only actions.
     * @post The returned plan does not include page text or source PDF text.
     * Side effects: None.
     * Determinism: Deterministic for a given validation report.
     */
    RepairPlan planRepair(const ValidationReport& report) const;
};

} // namespace pte::core
