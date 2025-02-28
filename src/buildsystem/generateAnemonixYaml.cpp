//
// Created by cv2 on 28.02.2025.
//

#include <fstream>

#include "Buildsystem.h"

void BuildSystem::generateAnemonixYaml() {
    std::ofstream yaml(anemo_pack_dir / "anemonix.yaml");
    yaml << "name: " << pkg.name << "\n"
         << "version: " << pkg.version << "\n"
         << "arch: " << pkg.arch << "\n"
         << "description: " << pkg.description << "\n"
         << "dependencies:\n";
    for (const auto& dep : pkg.deps) {
        yaml << "  - " << dep << "\n";
    }
    yaml << "provides:";
    for (const auto& provide : pkg.provides) {
        yaml << "  - " << provide << "\n";
    }



    yaml << "conflicts:\n";
    for (const auto& confl : pkg.conflicts) {
        yaml << "  - " << confl << "\n";
    }

    yaml.close();
}
