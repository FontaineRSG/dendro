#include <iostream>
#include <fstream>
#include <cstdlib>
#include <regex>
#include <vector>
#include "Package.h"
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/parse.h>

#include "Buildsystem.h"

Package parse_package(const YAML::Node& node) {
    Package pkg;

    // Parse scalar fields
    pkg.name = node["name"].as<std::string>("");
    pkg.author = node["author"].as<std::string>("");
    pkg.arch = node["arch"].as<std::string>("");
    pkg.version = node["version"].as<std::string>("");  // Handles numeric version in YAML
    pkg.description = node["description"].as<std::string>("");
    pkg.license = node["license"].as<std::string>("");
    pkg.build_script = node["build_script"].as<std::string>("");
    pkg.anemo_script = node["anemo_script"].as<std::string>("");

    // Parse sequence fields
    pkg.build_deps = node["build_deps"].as<std::vector<std::string>>(std::vector<std::string>());
    pkg.sources = node["sources"].as<std::vector<std::string>>(std::vector<std::string>());
    pkg.provides = node["provides"].as<std::vector<std::string>>(std::vector<std::string>());
    pkg.conflicts = node["conflicts"].as<std::vector<std::string>>(std::vector<std::string>());
    pkg.replaces = node["replaces"].as<std::vector<std::string>>(std::vector<std::string>());
    pkg.deps = node["deps"].as<std::vector<std::string>>(std::vector<std::string>());
    pkg.validpgpkeys = node["validpgpkeys"].as<std::vector<std::string>>(std::vector<std::string>());

    // Parse boolean
    pkg.is_protected = node["protected"].as<bool>(false);

    // Parse variants (splits)
    if (node["splits"]) {
        for (YAML::const_iterator it = node["splits"].begin(); it != node["splits"].end(); ++it) {
            Package variant = parse_package(it->second);
            pkg.variants.push_back(variant);
        }
    }

    return pkg;
}

// Usage:
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: dendro-build <specfile>\n";
        return EXIT_FAILURE;
    }
    const YAML::Node root = YAML::LoadFile(argv[1]); // Replace with your YAML file path
    Package main_package = parse_package(root);
    //main_package.print(false);
    BuildSystem bs(main_package);
    //bs.create_directories();
    bs.build(false);

    return EXIT_SUCCESS;
}