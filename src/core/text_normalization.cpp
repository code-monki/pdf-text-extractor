// File: src/core/text_normalization.cpp
// Purpose: Implement deterministic plain-text normalization and safe quality flags.
// Architectural context: HLA-TEXT, HLA-DIAG, HLA-SAFE.
// Requirement references: FR-016, FR-024, NFR-007, NFR-011, NFR-012.
// Test references: TC-FR-016, TC-FR-024, TC-NFR-007, TC-NFR-011, TC-NFR-012.
// Constraint: Diagnostics expose flags only and never quote normalized text.

#include "core/text_normalization.hpp"

#include <cctype>

namespace pte::core {

namespace {

/**
 * @brief Adds a flag if absent.
 */
void addFlag(std::vector<std::string>& flags, const std::string& flag) {
    for (const auto& existing : flags) {
        if (existing == flag) {
            return;
        }
    }
    flags.push_back(flag);
}

/**
 * @brief Reports whether a byte is an unsafe ASCII control for plain reviewed text.
 */
bool unsafeControl(unsigned char c) {
    return c < 0x20 && c != '\n' && c != '\t';
}

/**
 * @brief Removes trailing spaces and tabs before line endings and end of text.
 */
std::string trimTrailingWhitespace(const std::string& text) {
    std::string output;
    std::string pending;
    for (const char c : text) {
        if (c == ' ' || c == '\t') {
            pending.push_back(c);
            continue;
        }
        if (c == '\n') {
            pending.clear();
            output.push_back(c);
            continue;
        }
        output += pending;
        pending.clear();
        output.push_back(c);
    }
    return output;
}

} // namespace

/**
 * @brief Normalizes text into a plain search-surrogate representation.
 */
NormalizationResult TextNormalizationService::normalize(
    const std::string& text,
    const NormalizationProfile& profile) const {
    NormalizationResult result;
    std::string normalized;
    normalized.reserve(text.size());

    std::size_t digits = 0;
    std::size_t symbols = 0;
    std::size_t letters = 0;

    for (std::size_t index = 0; index < text.size(); ++index) {
        const unsigned char c = static_cast<unsigned char>(text[index]);
        if (profile.normalizeLineEndings && c == '\r') {
            if (index + 1 < text.size() && text[index + 1] == '\n') {
                continue;
            }
            normalized.push_back('\n');
            addFlag(result.flags, "line-endings-normalized");
            continue;
        }
        if (profile.removeUnsafeControls && unsafeControl(c)) {
            addFlag(result.flags, "control-characters-removed");
            continue;
        }
        if (std::isdigit(c)) {
            ++digits;
        } else if (std::isalpha(c)) {
            ++letters;
        } else if (!std::isspace(c)) {
            ++symbols;
        }
        normalized.push_back(static_cast<char>(c));
    }

    if (profile.trimTrailingWhitespace) {
        const auto trimmed = trimTrailingWhitespace(normalized);
        if (trimmed != normalized) {
            addFlag(result.flags, "trailing-whitespace-trimmed");
            normalized = trimmed;
        }
    }

    const auto signal = digits + symbols + letters;
    if (signal > 0 && digits * 2 > signal) {
        addFlag(result.flags, "high-numeric-ratio");
    }
    if (signal > 0 && symbols * 2 > signal) {
        addFlag(result.flags, "high-symbol-ratio");
    }
    if (normalized.empty()) {
        addFlag(result.flags, "empty");
    } else if (normalized.size() < 20) {
        addFlag(result.flags, "very-short");
    }
    if (normalized.find("\xef\xbf\xbd") != std::string::npos) {
        addFlag(result.flags, "unicode-suspect");
    }

    result.changed = normalized != text;
    result.text = std::move(normalized);
    result.safeMessage = "text normalization completed";
    return result;
}

} // namespace pte::core
