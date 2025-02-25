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
    std::vector<std::string> build_commands;
    bool is_protected;

    std::vector<std::string> pre_build_commands;
    std::vector<std::string> post_build_commands;
    std::vector<std::string> pre_install_script, post_install_script;
    std::vector<std::string> pre_remove_script, post_remove_script;
    std::string source;
    Package(
        std::string name,
        std::string author,
        std::string arch,
        std::string version,
        std::vector<std::string> build_deps,
        std::string description,
        std::string license,
        std::vector<std::string> provides,
        std::vector<std::string> conflicts,
        std::vector<std::string> replaces,
        std::vector<std::string> deps,
        bool is_protected,
        std::vector<std::string> pre_build_commands,
        std::vector<std::string> post_build_commands,
        std::vector<std::string> pre_install_script,
        std::vector<std::string> post_install_script,
        std::vector<std::string> pre_remove_script,
        std::vector<std::string> post_remove_script,
        std::string source
    );
    Package(){};

    ~Package() {
        provides.clear();
        conflicts.clear();
        replaces.clear();
        deps.clear();
        pre_build_commands.clear();
        post_build_commands.clear();
        build_deps.clear();
    }

    void print();

};



#endif //PACKAGE_H
