//
// Created by cv2 on 28.02.2025.
//

#include "Buildsystem.h"
#include <curl/curl.h>

void BuildSystem::processSource(const std::string& source, std::vector<std::string>& validPgpKeys) {
    // Handle Git repositories with signed tags
    if (source.find("git+") == 0) {
        handleGitSource(source, validPgpKeys);
        return;
    }

    // Handle tar archives
    if (source.find("tar+") == 0) {
        std::string tarUrl = source.substr(4);
        if (downloadUrl(tarUrl, "temp_source.tar") != CURLE_OK) {
            throw std::runtime_error("Failed to download tar archive: " + tarUrl);
        }
        extractAndFlattenTarball();
        fs::remove("temp_source.tar");
        return;
    }

    // Handle local files
    if (source.find("local+") == 0) {
        fs::path sourcePath = source.substr(6);

        if (!fs::exists(sourcePath)) {
            throw std::runtime_error("Local path does not exist: " + sourcePath.string());
        }

        if (fs::is_directory(sourcePath)) {
            fs::copy(sourcePath, build_root, fs::copy_options::recursive);
        } else {
            fs::copy(sourcePath, build_root / sourcePath.filename());
        }
        return;
    }

    // Existing handling for regular URLs and local directories
    if (fs::exists(source) && fs::is_directory(source)) {
        fs::copy(source, build_root, fs::copy_options::recursive);
        return;
    }

    // Handle regular URLs
    if (!downloadUrl(source, "temp_source.tar")) {
        throw std::runtime_error("Failed to download source: " + source);
    }
    extractAndFlattenTarball();
    fs::remove("temp_source.tar");
}

void BuildSystem::downloadAndPrepareSource() {
    if (pkg.sources.empty()) return;
    std::vector<std::string> &validPgpKeys = pkg.validpgpkeys;

    for (const auto& source : pkg.sources) {
        processSource(source, validPgpKeys);
    }
}