// File: src/core/validation_repair.cpp
// Purpose: Implement non-destructive repair planning for validation findings.
// Architectural context: HLA-VALID with HLA-WORK, HLA-SAFE.
// Requirement references: FR-025, FR-026, NFR-007, NFR-012.
// Test references: TC-FR-025, TC-FR-026, TC-NFR-007, TC-NFR-012.
// Assumption: Repair execution requires later explicit user confirmation workflow.
// Constraint: This slice plans dry-run repair actions only and does not modify artifacts.

#include "core/validation_repair.hpp"

namespace pte::core {

namespace {

/**
 * @brief Maps validation finding codes to safe repair action codes.
 *
 * @param findingCode Stable validation finding code.
 * @return Stable action code, or empty string when no automatic repair is planned.
 */
std::string actionCodeForFinding(const std::string& findingCode) {
    if (findingCode == "missing-page-file") {
        return "create-empty-page-file";
    }
    if (findingCode == "missing-review-entry") {
        return "add-default-review-entry";
    }
    if (findingCode == "missing-quality-entry") {
        return "add-default-quality-entry";
    }
    if (findingCode == "missing-page-map-entry") {
        return "add-page-map-entry";
    }
    if (findingCode == "missing-pages-directory") {
        return "create-pages-directory";
    }
    if (findingCode == "missing-artifact") {
        return "create-missing-artifact-shell";
    }
    return {};
}

/**
 * @brief Builds a safe repair action message.
 *
 * @param actionCode Stable repair action code.
 * @return Human-readable safe message.
 */
std::string safeActionMessage(const std::string& actionCode) {
    if (actionCode == "create-empty-page-file") {
        return "would create missing empty reviewed page text file";
    }
    if (actionCode == "add-default-review-entry") {
        return "would add missing default review-state entry";
    }
    if (actionCode == "add-default-quality-entry") {
        return "would add missing default page-quality entry";
    }
    if (actionCode == "add-page-map-entry") {
        return "would add missing page-map entry";
    }
    if (actionCode == "create-pages-directory") {
        return "would create missing pages directory";
    }
    if (actionCode == "create-missing-artifact-shell") {
        return "would create missing artifact shell";
    }
    return "would perform structural repair";
}

} // namespace

/**
 * @brief Reports whether the dry-run plan contains proposed actions.
 */
bool RepairPlan::hasActions() const {
    return !actions.empty();
}

/**
 * @brief Plans repair actions from repairable validation findings.
 */
RepairPlan ValidationRepairPlanner::planRepair(const ValidationReport& report) const {
    RepairPlan plan;
    for (const auto& finding : report.findings) {
        const auto actionCode = actionCodeForFinding(finding.code);
        if (finding.repairable && !actionCode.empty()) {
            plan.actions.push_back(RepairAction{
                actionCode,
                finding.code,
                finding.artifact,
                finding.pageId,
                safeActionMessage(actionCode)
            });
        } else if (finding.severity == Severity::Error || finding.severity == Severity::Blocker) {
            ++plan.nonRepairableCount;
        }
    }
    return plan;
}

} // namespace pte::core
