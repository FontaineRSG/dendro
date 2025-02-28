//
// Created by cv2 on 28.02.2025.
//

#include <regex>
#include "Buildsystem.h"

void BuildSystem::handleGitSource(const std::string& sourceUrl,
                                  const std::vector<std::string>& validPgpKeys) {
    std::regex pattern(R"(git\+(https?://.+?)(\?signed)?#tag=([\w\.-]+))");
    std::smatch matches;

    if (!std::regex_match(sourceUrl, matches, pattern)) {
        throw std::runtime_error("Invalid Git source format: " + sourceUrl);
    }
    setenv("GNUPGHOME", keyring_dir.c_str(), 1);
    const std::string repoUrl = matches[1].str();
    const bool signedTag = matches[2].matched;
    const std::string tag = matches[3].str();

    try {
        // Import keys if needed
        if (signedTag && !validPgpKeys.empty()) {
            importGpgKeys(validPgpKeys);
        }

        // Clone with full history for tag verification
        std::string cloneCmd = "git clone --branch " + tag +
                              " " + repoUrl + " " + build_root.string();

        if (std::system(cloneCmd.c_str()) != 0) {
            throw std::runtime_error("Clone failed: " + repoUrl);
        }

        // Verify signed tag if requested
        // disable until anemo-keyring
        // if (signedTag) {
        //     setenv("GNUPGHOME", keyringDir.c_str(), 1);
        //     std::string verifyCmd = "git -C " + build_root.string() +
        //                            " verify-tag " + tag;
        //
        //     if (std::system(verifyCmd.c_str()) != 0) {
        //         throw std::runtime_error("Tag verification failed: " + tag);
        //     }
        // }
    }
    catch (...) {
        fs::remove_all(build_root);
        throw;
    }
}
