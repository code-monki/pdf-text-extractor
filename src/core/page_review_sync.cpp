// File: src/core/page_review_sync.cpp
// Purpose: JSON-only snapshot for sequential review synchronization.

#include "core/page_review_sync.hpp"

#include "core/json.hpp"

#include <fstream>
#include <sstream>

namespace pte::core {

namespace {

bool readTextFile(const std::filesystem::path& path, std::string& output) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    output = buffer.str();
    return true;
}

bool parseRootObject(const std::filesystem::path& path, JsonValue& out) {
    std::string content;
    if (!readTextFile(path, content)) {
        return false;
    }
    const auto parsed = parseJson(content);
    if (!parsed.value || !parsed.value->isObject()) {
        return false;
    }
    out = *parsed.value;
    return true;
}

std::string stringField(const JsonValue& object, std::string_view key) {
    const auto* field = object.find(key);
    if (!field) {
        return {};
    }
    const auto* text = field->asString();
    return text ? *text : std::string{};
}

} // namespace

PageReviewSyncSnapshot loadPageReviewSyncSnapshot(const std::filesystem::path& workFolder,
                                                  const std::string& pageId) {
    PageReviewSyncSnapshot snap;

    JsonValue reviewRoot;
    if (!parseRootObject(workFolder / "review-state.json", reviewRoot)) {
        snap.safeMessage = "review-state.json unavailable";
        return snap;
    }

    const auto* pagesVal = reviewRoot.find("pages");
    const auto* pagesObj = pagesVal ? pagesVal->asObject() : nullptr;
    if (!pagesObj) {
        snap.safeMessage = "review-state pages missing";
        return snap;
    }

    const auto entryIter = pagesObj->find(pageId);
    if (entryIter == pagesObj->end()) {
        snap.safeMessage = "review-state has no entry for page";
        return snap;
    }

    const JsonValue& entry = entryIter->second;
    snap.reviewStatus = stringField(entry, "status");
    snap.selectedSource = stringField(entry, "selectedSource");
    snap.dirtyState = stringField(entry, "dirtyState");

    JsonValue volumeRoot;
    if (parseRootObject(workFolder / "volume.json", volumeRoot)) {
        snap.volumeTitle = stringField(volumeRoot, "title");
        const auto* mapVal = volumeRoot.find("pageMap");
        const auto* mapObj = mapVal ? mapVal->asObject() : nullptr;
        if (mapObj) {
            const auto mapIter = mapObj->find(pageId);
            if (mapIter != mapObj->end()) {
                snap.printedPageLabel = stringField(mapIter->second, "printedPageLabel");
            }
        }
    }

    snap.loaded = true;
    snap.safeMessage = "page review sync loaded";
    return snap;
}

} // namespace pte::core
