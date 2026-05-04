// File: tests/tc_fr028_probe.cpp
// Purpose: TC-FR-028 — verify native pte_shell binary exists (not a browser-hosted workflow).
// Requirement references: FR-028.
// Test references: TC-FR-028.

#include <cstdlib>
#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: tc_fr028_probe <path-to-pte_shell-binary>\n";
        return 2;
    }
    std::error_code ec;
    const std::filesystem::path path(argv[1]);
    if (!std::filesystem::exists(path, ec)) {
        std::cerr << "pte_shell not found at " << path << '\n';
        return 1;
    }
    return 0;
}
