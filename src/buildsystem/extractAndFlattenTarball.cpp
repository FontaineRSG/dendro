//
// Created by cv2 on 28.02.2025.
//

#include "Buildsystem.h"

void BuildSystem::extractAndFlattenTarball() const {
    // Extract tarball
    std::system(("tar xf temp_source.tar -C " + build_root.string()).c_str());
    fs::remove("temp_source.tar.gz");

    // Flatten directory structure
    auto dir_iter = fs::directory_iterator(build_root);
    std::vector<fs::path> dir_contents;

    // Find all extracted items
    for (const auto& entry : dir_iter) {
        dir_contents.push_back(entry.path());
    }

    // If we only have one directory, move its contents up
    if (dir_contents.size() == 1 && fs::is_directory(dir_contents[0])) {
        const auto& temp_dir = dir_contents[0];
        for (const auto& entry : fs::directory_iterator(temp_dir)) {
            fs::rename(entry.path(), build_root / entry.path().filename());
        }
        fs::remove(temp_dir);
    }
}
