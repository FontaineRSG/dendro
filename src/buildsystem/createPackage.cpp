//
// Created by cv2 on 28.02.2025.
//

#include "Buildsystem.h"

void BuildSystem::createPackage() {

    // Create tarball
    std::string tar_cmd = "tar czf " + final_pkg_path.string() + " -C " +
                         anemo_pack_dir.string() + " .";
    int result = std::system(tar_cmd.c_str());
    if (result != 0) {
        throw std::runtime_error("Failed to create package tarball");
    }

    for (const auto& variant : pkg.variants) {
        this->final_pkg_path = fs::current_path() / (variant.name + "-" + variant.version + ".apkg");
        this->anemo_pack_dir = fs::path("/var/dendro") / variant.name / "anemopack";

        // Create tarball
        tar_cmd = "tar czf " + final_pkg_path.string() + " -C " +
                             anemo_pack_dir.string() + " .";
        result = std::system(tar_cmd.c_str());
        if (result != 0) {
            throw std::runtime_error("Failed to create package tarball");
        }

    }
}
