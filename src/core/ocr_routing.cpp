// File: src/core/ocr_routing.cpp
// Purpose: Implement deterministic OCR candidate routing policy.
// Architectural context: HLA-OCR, HLA-EXT, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-014, FR-024, FR-031, NFR-004, NFR-010, NFR-012.
// Test references: TC-FR-014, TC-FR-024, TC-FR-031, TC-NFR-004, TC-NFR-010,
// TC-NFR-012, TC-BND-OCR-003.
// Constraint: Routing reports decisions only and does not execute OCR tooling.

#include "core/ocr_routing.hpp"

namespace pte::core {

namespace {

constexpr std::size_t veryShortEmbeddedThreshold = 20;

/**
 * @brief Builds one OCR routing decision for a page.
 */
OcrRoutingDecision decidePage(const OcrRoutingDocumentProfile& profile,
                              const OcrRoutingPageSignal& page) {
    OcrRoutingDecision decision;
    decision.pageId = page.pageId;
    decision.pdfPage = page.pdfPage;

    if (!profile.ocrToolAvailable) {
        decision.scheduleOcr = false;
        decision.reason = "ocr-tool-unavailable";
        return decision;
    }
    if (profile.paperCapture) {
        decision.scheduleOcr = true;
        decision.reason = "paper-capture-default";
        return decision;
    }
    if (!page.embeddedAvailable) {
        decision.scheduleOcr = true;
        decision.reason = "missing-embedded-text";
        return decision;
    }
    if (page.embeddedCharCount <= veryShortEmbeddedThreshold) {
        decision.scheduleOcr = true;
        decision.reason = "very-short-embedded-text";
        return decision;
    }

    decision.scheduleOcr = false;
    decision.reason = "embedded-text-available";
    return decision;
}

} // namespace

/**
 * @brief Builds a review-gated OCR candidate generation plan.
 */
OcrRoutingPlan OcrRoutingService::plan(
    const OcrRoutingDocumentProfile& profile,
    const std::vector<OcrRoutingPageSignal>& pages) const {
    OcrRoutingPlan plan;
    plan.reviewGated = true;

    if (pages.empty()) {
        plan.safeMessage = "no pages available for OCR routing";
        return plan;
    }

    for (const auto& page : pages) {
        plan.decisions.push_back(decidePage(profile, page));
    }

    plan.success = true;
    plan.safeMessage = "OCR routing plan created";
    return plan;
}

} // namespace pte::core
