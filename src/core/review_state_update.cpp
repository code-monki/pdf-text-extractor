// File: src/core/review_state_update.cpp
// Purpose: Implement deterministic patches to review-state.json.

#include "core/review_state_update.hpp"

#include "core/json.hpp"
#include "core/review_state.hpp"

#include <fstream>
#include <sstream>

namespace pte::core {

namespace {

std::optional<std::string> readUtf8File(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

bool writeTextAtomic(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    const auto temp = path.string() + ".tmp";
    {
        std::ofstream output(temp, std::ios::binary | std::ios::trunc);
        if (!output) {
            return false;
        }
        output << content;
        if (!output) {
            return false;
        }
    }
    std::filesystem::rename(temp, path);
    return true;
}

} // namespace

ReviewStateUpdateResult applyReviewStateAfterPageTextSave(
    const std::filesystem::path& workFolder,
    const std::string& pageId,
    const ReviewStatePageSavePatch& patch) {
    ReviewStateUpdateResult result;
    const auto path = workFolder / "review-state.json";
    const auto bytes = readUtf8File(path);
    if (!bytes) {
        result.safeMessage = "review-state.json could not be read";
        return result;
    }

    auto parsed = parseJson(*bytes);
    if (!parsed.value || !parsed.value->isObject()) {
        result.safeMessage = "review-state.json is not a valid JSON object";
        return result;
    }

    JsonValue document = std::move(*parsed.value);
    JsonValue::Object* root = document.asObjectForUpdate();
    if (root == nullptr) {
        result.safeMessage = "review-state root is not an object";
        return result;
    }

    auto pagesIt = root->find("pages");
    if (pagesIt == root->end()) {
        result.safeMessage = "review-state.json has no pages object";
        return result;
    }

    JsonValue::Object* pages = pagesIt->second.asObjectForUpdate();
    if (pages == nullptr) {
        result.safeMessage = "review-state pages is not an object";
        return result;
    }

    auto entryIt = pages->find(pageId);
    if (entryIt == pages->end()) {
        result.safeMessage = "review-state has no entry for page";
        return result;
    }

    JsonValue::Object* entry = entryIt->second.asObjectForUpdate();
    if (entry == nullptr) {
        result.safeMessage = "review-state page entry is not an object";
        return result;
    }

    std::string priorStatus;
    const auto statusIt = entry->find("status");
    if (statusIt != entry->end()) {
        const auto* sp = statusIt->second.asString();
        if (sp != nullptr) {
            priorStatus = *sp;
        }
    }

    (*entry)["selectedSource"] = JsonValue(patch.selectedSource);
    (*entry)["dirtyState"] = JsonValue(std::string("dirty"));

    if (patch.advanceRawToEditing && priorStatus == "raw") {
        if (isAllowedReviewTransition("raw", "editing")) {
            (*entry)["status"] = JsonValue(std::string("editing"));
        }
    }

    const std::string serialized = formatJsonCompact(document);
    if (!writeTextAtomic(path, serialized)) {
        result.safeMessage = "review-state.json could not be written";
        return result;
    }

    result.success = true;
    result.safeMessage = "review-state.json updated";
    return result;
}

} // namespace pte::core
