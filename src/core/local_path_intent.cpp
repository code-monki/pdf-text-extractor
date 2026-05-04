// File: src/core/local_path_intent.cpp
// Purpose: Hosted / remote URI scheme detection for local-only operation (NFR-001).

#include "core/local_path_intent.hpp"

namespace pte::core {

bool pathTextLooksLikeHostedUriScheme(std::string_view pathText) {
    if (pathText.size() >= 7) {
        static constexpr char kFile[] = "file://";
        bool isFile = true;
        for (std::size_t i = 0; i < 7; ++i) {
            if (pathText[i] != kFile[i]) {
                isFile = false;
                break;
            }
        }
        if (isFile) {
            return false;
        }
    }

    const auto schemeSep = pathText.find("://");
    if (schemeSep == std::string_view::npos || schemeSep == 0) {
        return false;
    }
    return true;
}

} // namespace pte::core
