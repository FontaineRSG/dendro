//
// Created by cv2 on 28.02.2025.
//

#ifndef BUILDSYSTEM_H
#define BUILDSYSTEM_H
#include <filesystem>
#include <utility>
#include <vector>
#include <string>
#include "Package.h"

namespace fs = std::filesystem;

class BuildSystem {
    Package pkg;
    fs::path build_root;
    fs::path anemo_pack_dir;
    fs::path final_pkg_path;
    fs::path keyring_dir;
    bool is_variant;

public:
    // Constructor for main package
    explicit BuildSystem(const Package& package)
        : is_variant(false),
          build_root(fs::path("/var/dendro") / package.name / "buildroot"),
          anemo_pack_dir(fs::path("/var/dendro") / package.name / "anemopack"),
          final_pkg_path(fs::current_path() / (package.name + "-" + package.version + ".apkg")),
          keyring_dir(fs::path("/var/dendro/keyrings") / package.name / "temp_keyring"),
          pkg(package)
    {
    }

    void create_directories() {
        for (auto const& pkg : pkg.variants) {
            std::cout << "processing: " << pkg.name << "\n";
            fs::create_directories(fs::path("/var/dendro") / pkg.name / "anemopack/package");
        }
        pkg.print(false);
        fs::create_directories(build_root);
        // Create packaging directories for all
        fs::create_directories(final_pkg_path.parent_path());
        fs::create_directories(keyring_dir);
    }

    // Getters
    [[nodiscard]] const fs::path& get_build_root() const { return build_root; }
    [[nodiscard]] const fs::path& get_anemo_pack_dir() const { return anemo_pack_dir; }
    [[nodiscard]] const fs::path& get_keyring_dir() const { return keyring_dir; }
    [[nodiscard]] const fs::path& get_final_pkg_path() const { return final_pkg_path; }

    void importGpgKeys(const std::vector<std::string> &keyIds) const;

    void handleGitSource(const std::string &sourceUrl, const std::vector<std::string> &validPgpKeys);

    void processSource(const std::string &source, std::vector<std::string> &validPgpKeys);

    void extractAndFlattenTarball() const;

    void generateAnemonixYaml();

    void createPackage();

    void build(bool use_fakeroot);

    void downloadAndPrepareSource();

    bool downloadUrl(const std::string &url, const std::string &outputFile);

    void process_package(const Package &pkg);

    int execute_script(const std::string &script_path, const std::string &function_name,
                       bool use_fakeroot, const std::vector<std::string> &args) const;

    bool checkDependencies();
};


#endif //BUILDSYSTEM_H
