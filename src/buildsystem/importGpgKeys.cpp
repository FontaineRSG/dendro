//
// Created by cv2 on 28.02.2025.
//

#include "Buildsystem.h"

void BuildSystem::importGpgKeys(const std::vector<std::string>& keyIds) const {
    // Create temporary keyring directory
    fs::create_directories(keyring_dir);

    // Set custom keyring location
    const std::string keyringOpt = "--no-default-keyring --keyring " +
                                  (keyring_dir / "trustedkeys.gpg").string();

    for (const auto& keyId : keyIds) {
        // First try to receive the key from keyserver
        std::vector<std::string> keyservers = {
            "hkps://keyserver.ubuntu.com",
            "hkps://keys.openpgp.org",
            "hkps://pgp.mit.edu"
        };

        bool keyFetched = false;
        for (const auto& keyserver : keyservers) {
            std::string fetchCmd = "gpg " + keyringOpt +
                                 " --keyserver " + keyserver +
                                 " --recv-keys " + keyId + " 2>&1";
            if (std::system(fetchCmd.c_str()) == 0) {
                keyFetched = true;
                break;
            }
        }

        if (!keyFetched) {
            fs::remove_all(keyring_dir);
            throw std::runtime_error("Failed to fetch key from all keyservers: " + keyId);
        }

        // Trust key ultimately in temporary keyring
        std::string trustCmd = "echo \"" + keyId +
                             ":6:\" | gpg " + keyringOpt +
                             " --import-ownertrust 2>/dev/null";
        if (std::system(trustCmd.c_str()) != 0) {
            fs::remove_all(keyring_dir);
            throw std::runtime_error("Failed to trust key: " + keyId);
        }
    }

    // Set environment variable for git to use our temporary keyring
    setenv("GNUPGHOME", keyring_dir.c_str(), 1);
}
