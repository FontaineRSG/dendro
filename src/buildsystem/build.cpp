//
// Created by cv2 on 28.02.2025.
//

#include "Buildsystem.h"

void BuildSystem::build(bool use_fakeroot) {
    try {
        if (!checkDependencies()) return;
        create_directories();
        downloadAndPrepareSource();
        generateAnemonixYaml();
        // execute prebuild script
        if(!execute_script(pkg.build_script, "prepare", use_fakeroot, {""})) {
            throw std::runtime_error("err: prepare function failed");
        };

        if(!execute_script(pkg.build_script, "build", use_fakeroot, {""})) {
            throw std::runtime_error("err: build function failed");
        };

        if(!execute_script(pkg.build_script, "install", use_fakeroot, {""})) {
            throw std::runtime_error("err: install function failed");
        };

        for(auto& variant : pkg.variants) {
            this->anemo_pack_dir = fs::path("/var/dendro") / variant.name / "anemopack";
            // execute prebuild script
            if(!execute_script(variant.build_script, "prepare", use_fakeroot, {""})) {
                throw std::runtime_error("err: prepare function failed");
            };

            if(!execute_script(variant.build_script, "build", use_fakeroot, {""})) {
                throw std::runtime_error("err: build function failed");
            };

            if(!execute_script(variant.build_script, "install", use_fakeroot, {""})) {
                throw std::runtime_error("err: install function failed");
            };
        }

        createPackage();
        //cleanUp();

        std::cout << "Package built successfully: " << final_pkg_path << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Build failed: " << e.what() << "\n";
        //cleanUp();
        exit(EXIT_FAILURE);
    }
}
