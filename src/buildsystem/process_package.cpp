//
// Created by cv2 on 28.02.2025.
//


#include "Buildsystem.h"
#include "Package.h"

void BuildSystem::process_package(const Package& pkg) {
    // Process main package
    BuildSystem main_build(pkg);
    std::cout << "Building main package: " << pkg.name << "\n";
    execute_script(pkg, main_build);

    // Process variants using parent's buildroot
    for (const auto& variant : pkg.variants) {
        std::cout << "\nBuilding variant: " << variant.name << "\n";
        BuildSystem variant_build(variant, main_build.get_build_root(), pkg.name);
        execute_package_scripts(variant, variant_build);
    }
}
