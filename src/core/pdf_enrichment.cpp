// File: src/core/pdf_enrichment.cpp
// Purpose: Parse enrichment sidecars and write derived-PDF prototype outputs.

#include "core/pdf_enrichment.hpp"

#include "core/json.hpp"
#include "core/process_runner.hpp"

#include <fstream>
#include <sstream>
#include <unordered_set>

namespace pte::core {
namespace {

std::optional<std::string> readUtf8File(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

bool parseOutlineMap(const std::filesystem::path& path,
                     PdfEnrichmentResult& result,
                     int& manualCount,
                     std::unordered_set<std::string>& destinationIds) {
    const auto text = readUtf8File(path);
    if (!text.has_value()) {
        result.safeMessage = "outline-map not readable";
        return false;
    }
    const JsonParseResult parsed = parseJson(*text);
    if (!parsed.value.has_value()) {
        result.safeMessage = "outline-map JSON parse failed";
        return false;
    }

    const JsonValue* schemaVersion = parsed.value->find("schemaVersion");
    if (schemaVersion == nullptr || !schemaVersion->isNumber()
        || static_cast<int>(*schemaVersion->asNumber()) != 1) {
        result.safeMessage = "outline-map schemaVersion must be 1";
        return false;
    }

    const JsonValue* nodes = parsed.value->find("nodes");
    if (nodes == nullptr || !nodes->isArray()) {
        result.safeMessage = "outline-map missing nodes array";
        return false;
    }

    for (const JsonValue& node : *nodes->asArray()) {
        if (node.asObject() == nullptr) {
            result.safeMessage = "outline-map node must be object";
            return false;
        }
        const JsonValue* level = node.find("level");
        const JsonValue* title = node.find("title");
        const JsonValue* destination = node.find("destination");
        if (level == nullptr || !level->isNumber() || title == nullptr || !title->isString()
            || destination == nullptr || !destination->isObject()) {
            result.safeMessage = "outline-map node missing required fields";
            return false;
        }
        ++result.outlineNodeCount;
        const JsonValue* id = node.find("id");
        if (id != nullptr && id->isString()) {
            destinationIds.insert(*id->asString());
        }
        const JsonValue* manual = node.find("manual");
        if (manual != nullptr && manual->isBool() && *manual->asBool()) {
            ++manualCount;
        }
    }
    return true;
}

bool parseLinkMap(const std::filesystem::path& path,
                  PdfEnrichmentResult& result,
                  int& manualCount,
                  const std::unordered_set<std::string>& destinationIds) {
    const auto text = readUtf8File(path);
    if (!text.has_value()) {
        result.safeMessage = "link-map not readable";
        return false;
    }
    const JsonParseResult parsed = parseJson(*text);
    if (!parsed.value.has_value()) {
        result.safeMessage = "link-map JSON parse failed";
        return false;
    }

    const JsonValue* schemaVersion = parsed.value->find("schemaVersion");
    if (schemaVersion == nullptr || !schemaVersion->isNumber()
        || static_cast<int>(*schemaVersion->asNumber()) != 1) {
        result.safeMessage = "link-map schemaVersion must be 1";
        return false;
    }

    const JsonValue* links = parsed.value->find("links");
    if (links == nullptr || !links->isArray()) {
        result.safeMessage = "link-map missing links array";
        return false;
    }

    for (const JsonValue& link : *links->asArray()) {
        if (link.asObject() == nullptr) {
            result.safeMessage = "link-map link must be object";
            return false;
        }
        const JsonValue* rect = link.find("rect");
        const JsonValue* target = link.find("target");
        if (rect == nullptr || !rect->isArray() || target == nullptr || !target->isObject()) {
            result.safeMessage = "link-map link missing required fields";
            return false;
        }
        if (rect->asArray()->size() != 4) {
            result.safeMessage = "link-map rect must have four numbers";
            return false;
        }
        for (const JsonValue& coord : *rect->asArray()) {
            if (!coord.isNumber()) {
                result.safeMessage = "link-map rect coordinates must be numeric";
                return false;
            }
        }
        const JsonValue* type = target->find("type");
        if (type == nullptr || !type->isString()) {
            result.safeMessage = "link-map target missing type";
            return false;
        }
        const std::string targetType = *type->asString();
        if (targetType == "intra") {
            const JsonValue* destinationId = target->find("destinationId");
            const JsonValue* destination = target->find("destination");
            if (destination != nullptr && !destination->isObject()) {
                result.safeMessage = "link-map intra destination must be object when provided";
                return false;
            }
            if (destination == nullptr) {
                if (destinationId == nullptr || !destinationId->isString()
                    || destinationId->asString()->empty()) {
                    result.safeMessage =
                        "link-map intra target requires destinationId or destination object";
                    return false;
                }
                if (!destinationIds.empty()
                    && destinationIds.find(*destinationId->asString()) == destinationIds.end()) {
                    result.safeMessage = "link-map intra destinationId not found in outline-map ids";
                    return false;
                }
            }
        } else if (targetType == "inter") {
            const JsonValue* destinationId = target->find("destinationId");
            const JsonValue* pathOrVolume = target->find("path");
            const JsonValue* volumeId = target->find("volumeId");
            const bool hasPath = pathOrVolume != nullptr && pathOrVolume->isString()
                                 && !pathOrVolume->asString()->empty();
            const bool hasVolume = volumeId != nullptr && volumeId->isString()
                                   && !volumeId->asString()->empty();
            if (!hasPath && !hasVolume) {
                result.safeMessage = "link-map inter target requires path or volumeId";
                return false;
            }
            if (destinationId == nullptr || !destinationId->isString()
                || destinationId->asString()->empty()) {
                result.safeMessage = "link-map inter target requires destinationId";
                return false;
            }
        } else if (targetType == "url") {
            const JsonValue* url = target->find("url");
            if (url == nullptr || !url->isString() || url->asString()->empty()) {
                result.safeMessage = "link-map url target requires url";
                return false;
            }
            const std::string& urlText = *url->asString();
            if (!(urlText.rfind("https://", 0) == 0 || urlText.rfind("http://", 0) == 0)) {
                result.safeMessage = "link-map url target must start with http:// or https://";
                return false;
            }
        } else {
            result.safeMessage = "link-map target type must be intra, inter, or url";
            return false;
        }
        ++result.linkCount;
        const JsonValue* manual = link.find("manual");
        if (manual != nullptr && manual->isBool() && *manual->asBool()) {
            ++manualCount;
        }
    }
    return true;
}

bool writeReport(const std::filesystem::path& path, const PdfEnrichmentRequest& req,
                 const PdfEnrichmentResult& result) {
    JsonValue::Object root;
    root["schemaVersion"] = JsonValue(1.0);
    root["sourcePdfPath"] = JsonValue(req.sourcePdfPath.generic_string());
    root["derivedPdfPath"] = JsonValue(req.derivedPdfPath.generic_string());
    root["outlineNodeCount"] = JsonValue(static_cast<double>(result.outlineNodeCount));
    root["linkCount"] = JsonValue(static_cast<double>(result.linkCount));
    root["manualOverrideCount"] = JsonValue(static_cast<double>(result.manualOverrideCount));
    JsonValue::Array warnings;
    for (const std::string& warning : result.warnings) {
        warnings.emplace_back(warning);
    }
    root["warnings"] = JsonValue(std::move(warnings));
    root["prototypeStatus"] =
        JsonValue("Derived copy created; bookmark/link annotation injection pending.");

    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }
    out << formatJsonCompact(JsonValue(std::move(root)));
    return static_cast<bool>(out);
}

bool runPythonAnnotationInjection(const PdfEnrichmentRequest& request, PdfEnrichmentResult& result) {
    if (!request.pythonScriptPath.has_value()) {
        result.safeMessage = "annotation injection requested but python script path is missing";
        return false;
    }
    std::vector<std::string> argv{
        "python3",
        request.pythonScriptPath->string(),
        "--source",
        request.sourcePdfPath.string(),
        "--output",
        request.derivedPdfPath.string(),
    };
    if (request.outlineMapPath.has_value()) {
        argv.insert(argv.end(), {"--outline-map", request.outlineMapPath->string()});
    }
    if (request.linkMapPath.has_value()) {
        argv.insert(argv.end(), {"--link-map", request.linkMapPath->string()});
    }

    ProcessRunOptions options;
    options.mergeStderrIntoStdout = true;
    const auto run = runProcessArgv(request.pythonExecutable, argv, options);
    if (run.spawnFailed || run.exitCode != 0) {
        result.safeMessage = "annotation injection failed";
        if (!run.stdoutText.empty()) {
            result.warnings.push_back("enrichment tool output: " + run.stdoutText.substr(0, 200));
        }
        return false;
    }
    return true;
}

} // namespace

PdfEnrichmentResult PdfEnrichmentService::run(const PdfEnrichmentRequest& request) const {
    PdfEnrichmentResult result;
    if (request.sourcePdfPath.empty()) {
        result.safeMessage = "source PDF path is required";
        return result;
    }
    if (!request.lintOnly && request.derivedPdfPath.empty()) {
        result.safeMessage = "derived PDF path is required unless lint-only";
        return result;
    }
    if (!std::filesystem::exists(request.sourcePdfPath)) {
        result.safeMessage = "source PDF not found";
        return result;
    }
    if (!request.lintOnly
        && request.sourcePdfPath.lexically_normal() == request.derivedPdfPath.lexically_normal()) {
        result.safeMessage = "derived output path must differ from source PDF path";
        return result;
    }

    int manualCount = 0;
    std::unordered_set<std::string> destinationIds;
    if (request.outlineMapPath.has_value()
        && !parseOutlineMap(*request.outlineMapPath, result, manualCount, destinationIds)) {
        return result;
    }
    if (request.linkMapPath.has_value()
        && !parseLinkMap(*request.linkMapPath, result, manualCount, destinationIds)) {
        return result;
    }
    result.manualOverrideCount = manualCount;

    if (request.lintOnly) {
        result.success = true;
        result.safeMessage = "enrichment lint passed";
        return result;
    }

    std::error_code ec;
    std::filesystem::create_directories(request.derivedPdfPath.parent_path(), ec);

    const bool hasMaps = request.outlineMapPath.has_value() || request.linkMapPath.has_value();
    if (request.enableAnnotationInjection && hasMaps) {
        if (!runPythonAnnotationInjection(request, result)) {
            return result;
        }
    } else {
        ec.clear();
        std::filesystem::copy_file(
            request.sourcePdfPath,
            request.derivedPdfPath,
            std::filesystem::copy_options::overwrite_existing,
            ec);
        if (ec) {
            result.safeMessage = "failed to write derived PDF";
            return result;
        }
    }
    result.derivedPdfWritten = true;

    const std::filesystem::path reportPath = request.reportPath.value_or(
        request.derivedPdfPath.parent_path() / "enrichment-report.json");
    std::filesystem::create_directories(reportPath.parent_path(), ec);
    if (!writeReport(reportPath, request, result)) {
        result.safeMessage = "failed to write enrichment report";
        return result;
    }

    result.success = true;
    result.safeMessage = (request.enableAnnotationInjection && hasMaps)
                             ? "derived PDF created with annotation injection"
                             : "derived PDF prototype created";
    return result;
}

EnrichmentLinkMapPreviewLoadResult loadLinkMapForPreview(const std::filesystem::path& path) {
    EnrichmentLinkMapPreviewLoadResult result;
    const auto text = readUtf8File(path);
    if (!text.has_value()) {
        result.safeMessage = "link-map not readable";
        return result;
    }
    const JsonParseResult parsed = parseJson(*text);
    if (!parsed.value.has_value()) {
        result.safeMessage = "link-map JSON parse failed";
        return result;
    }

    const JsonValue* schemaVersion = parsed.value->find("schemaVersion");
    if (schemaVersion == nullptr || !schemaVersion->isNumber()
        || static_cast<int>(*schemaVersion->asNumber()) != 1) {
        result.safeMessage = "link-map schemaVersion must be 1";
        return result;
    }

    const JsonValue* links = parsed.value->find("links");
    if (links == nullptr || !links->isArray()) {
        result.safeMessage = "link-map missing links array";
        return result;
    }

    for (const JsonValue& link : *links->asArray()) {
        if (link.asObject() == nullptr) {
            result.safeMessage = "link-map link must be object";
            return result;
        }
        const JsonValue* pageIndex = link.find("pageIndex");
        const JsonValue* rect = link.find("rect");
        const JsonValue* target = link.find("target");
        if (pageIndex == nullptr || !pageIndex->isNumber() || rect == nullptr || !rect->isArray()
            || target == nullptr || !target->isObject()) {
            result.safeMessage = "link-map link missing required fields";
            return result;
        }
        if (rect->asArray()->size() != 4) {
            result.safeMessage = "link-map rect must have four numbers";
            return result;
        }
        EnrichmentLinkPreviewEntry entry;
        entry.pageIndex = static_cast<int>(*pageIndex->asNumber());
        for (std::size_t i = 0; i < 4; ++i) {
            const JsonValue& coord = rect->asArray()->at(i);
            if (!coord.isNumber()) {
                result.safeMessage = "link-map rect coordinates must be numeric";
                return result;
            }
            entry.rect[i] = *coord.asNumber();
        }
        const JsonValue* type = target->find("type");
        if (type != nullptr && type->isString()) {
            entry.targetType = *type->asString();
        }
        const JsonValue* manual = link.find("manual");
        if (manual != nullptr && manual->isBool()) {
            entry.manual = *manual->asBool();
        }
        result.links.push_back(entry);
    }

    result.success = true;
    result.safeMessage = "link-map preview loaded";
    return result;
}

} // namespace pte::core
