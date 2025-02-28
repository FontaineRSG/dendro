//
// Created by cv2 on 28.02.2025.
//

#include <string>
#include <unistd.h>
#include "Buildsystem.h"
#include <sys/wait.h>

// Modified execute_script with environment variables
int BuildSystem::execute_script(const std::string &script_path, const std::string &function_name,
    bool use_fakeroot, const std::vector<std::string> &args) const {
    std::vector<std::string> cmd_args;

    // Add fakeroot if needed
    if (use_fakeroot) {
        cmd_args.emplace_back("fakeroot");
    }

    // Build the command structure
    if (function_name.empty()) {
        // Regular script execution
        cmd_args.emplace_back("bash");
        cmd_args.push_back(script_path);
    } else {
        // Function call execution
        cmd_args.emplace_back("bash");
        cmd_args.emplace_back("-c");

        // Build the source-and-execute command
        std::string command = "source " + script_path + " && " + function_name;
        std::cout << "sourcing: " << command << std::endl;
        for (const auto& arg : args) {
            command += " " + arg;
        }
        cmd_args.push_back(command);
    }

    // Add additional arguments for regular script execution
    if (function_name.empty()) {
        for (const auto& arg : args) {
            cmd_args.push_back(arg);
        }
    }

    // Convert to C-style arguments
    std::vector<char*> argv;
    argv.reserve(cmd_args.size());
    for (auto& arg : cmd_args) {
        argv.push_back(&arg[0]);
    }
    argv.push_back(nullptr);

    // Fork and execute
    pid_t pid = fork();
    if (pid == 0) { // Child process
        setenv("dendro_buildroot", get_build_root().c_str(), 1);
        setenv("dendro_installloc", (get_anemo_pack_dir() / "package").c_str(), 1);
        setenv("dendro_rootdir", get_build_root().parent_path().c_str(), 1);
        setenv("dendro_keyring", get_keyring_dir().c_str(), 1);
        setenv("dendro_package", get_final_pkg_path().c_str(), 1);

        execvp(argv[0], argv.data());
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;  // Child didn't exit normally
    }
}