//
// Created by cv2 on 2/25/25.
//

#ifndef PACKAGE_H
#define PACKAGE_H
#include <string>
#include <vector>
#include <iostream>

class Package {
  public:
    std::string name;
    std::string author;
    std::string arch;
    std::string version;
    std::vector<std::string> build_deps;
    std::string description;
    std::string license;
    std::vector<std::string> provides;
    std::vector<std::string> conflicts;
    std::vector<std::string> replaces;
    std::vector<std::string> deps;
    bool is_protected = false;
    std::string build_script;
    std::string anemo_script;
    std::vector<std::string> sources;
    std::vector<std::string> validpgpkeys;
    std::vector<Package> variants;

    Package(std::string name, std::string author, std::string arch, std::string version,
        std::vector<std::string> build_deps, std::string description, std::string license,
        std::vector<std::string> provides, std::vector<std::string> conflicts, std::vector<std::string> replaces,
        std::vector<std::string> deps, bool is_protected, std::string build_script, std::string anemo_script,
        std::vector<std::string> sources, std::vector<std::string> validpgpkeys, std::vector<Package> variants)
        : name(std::move(name)),
          author(std::move(author)),
          arch(std::move(arch)),
          version(std::move(version)),
          build_deps(std::move(build_deps)),
          description(std::move(description)),
          license(std::move(license)),
          provides(std::move(provides)),
          conflicts(std::move(conflicts)),
          replaces(std::move(replaces)),
          deps(std::move(deps)),
          is_protected(is_protected),
          build_script(std::move(build_script)),
          anemo_script(std::move(anemo_script)),
          sources(std::move(sources)),
          validpgpkeys(std::move(validpgpkeys)),
          variants(std::move(variants)) {
    }

    Package()= default;

    ~Package() {
        provides.clear();
        conflicts.clear();
        replaces.clear();
        deps.clear();
        build_deps.clear();
        sources.clear();
        validpgpkeys.clear();
        variants.clear();
    }

    void print(bool child);

};



#endif //PACKAGE_H
