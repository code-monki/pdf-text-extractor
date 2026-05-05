// File: src/core/ocr_routing.hpp
// Purpose: Determine review-gated OCR candidate routing decisions.
// Architectural context: HLA-OCR, HLA-EXT, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-014, FR-024, FR-031, NFR-004, NFR-010, NFR-012.
// Test references: TC-FR-014, TC-FR-024, TC-FR-031, TC-NFR-004, TC-NFR-010,
// TC-NFR-012, TC-BND-OCR-003.
// Constraint: Routing schedules candidate generation only; it does not run OCR or accept pages.

#pragma once

#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Source PDF classification relevant to OCR routing.
 */
struct OcrRoutingDocumentProfile {
    bool paperCapture = false;
    bool ocrToolAvailable = true;
};

/**
 * @brief Per-page embedded extraction signal used for OCR routing.
 */
struct OcrRoutingPageSignal {
    std::string pageId;
    int pdfPage = 0;
    bool embeddedAvailable = false;
    std::size_t embeddedCharCount = 0;
};

/**
 * @brief One OCR candidate scheduling decision.
 */
struct OcrRoutingDecision {
    std::string pageId;
    int pdfPage = 0;
    bool scheduleOcr = false;
    std::string reason;
};

/**
 * @brief OCR routing plan for a selected source PDF/work folder.
 */
struct OcrRoutingPlan {
    bool success = false;
    bool reviewGated = true;
    std::vector<OcrRoutingDecision> decisions;
    std::string safeMessage;
};

/**
 * @brief Determines which pages should receive OCR candidate generation.
 */
class OcrRoutingService {
public:
    /**
     * @brief Builds a deterministic OCR candidate routing plan.
     *
     * @param profile Source PDF profile and OCR capability availability.
     * @param pages Per-page embedded extraction signals.
     * @return Safe routing plan with page IDs, PDF pages, and reasons.
     * @post The plan cannot change review status or release eligibility.
     * Side effects: None.
     * Determinism: Deterministic for supplied profile and page signals.
     */
    OcrRoutingPlan plan(const OcrRoutingDocumentProfile& profile,
                        const std::vector<OcrRoutingPageSignal>& pages) const;
};

} // namespace pte::core
