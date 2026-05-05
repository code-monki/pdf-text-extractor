// File: src/core/text_normalization.hpp
// Purpose: Normalize reviewed/candidate text into plain search-surrogate text.
// Architectural context: HLA-TEXT, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-016, FR-024, NFR-007, NFR-011, NFR-012.
// Test references: TC-FR-016, TC-FR-024, TC-NFR-007, TC-NFR-011, TC-NFR-012.
// Constraint: Normalization results must not be used in safe diagnostics to expose page text.

#pragma once

#include <string>
#include <vector>

namespace pte::core {

/**
 * @brief Deterministic text normalization policy.
 */
struct NormalizationProfile {
    bool normalizeLineEndings = true;
    bool removeUnsafeControls = true;
    bool trimTrailingWhitespace = true;
};

/**
 * @brief Normalized text plus safe quality flags.
 */
struct NormalizationResult {
    std::string text;
    std::vector<std::string> flags;
    bool changed = false;
    std::string safeMessage;
};

/**
 * @brief Service for deterministic local text normalization.
 */
class TextNormalizationService {
public:
    /**
     * @brief Normalizes local text according to a deterministic profile.
     *
     * @param text Local reviewed or candidate text.
     * @param profile Normalization policy.
     * @return Normalized text, change flag, and safe diagnostic flags.
     * @post safeMessage and flags do not include substantive input text.
     * Side effects: None.
     * Determinism: Deterministic for supplied text and profile.
     */
    NormalizationResult normalize(const std::string& text,
                                  const NormalizationProfile& profile) const;
};

} // namespace pte::core
