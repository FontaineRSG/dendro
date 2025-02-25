//
// Created by cv2 on 2/25/25.
//

#include "Package.h"
#include <sstream>
#define GRAY    "\033[90m"
#define WHITE   "\033[97m"
#include "colors.h"

// Check if a word is a Bash built-in or common command
bool isCommand(const std::string& word) {
    static const std::vector<std::string> commands = {
        "echo", "cd", "ls", "cp", "mv", "rm", "mkdir", "rmdir", "chmod", "chown", "touch",
        "find", "grep", "sed", "awk", "tar", "make", "gcc", "g++", "clang", "ld", "ar",
        "pkg-config", "install", "sudo", "ln", "export", "unset", "kill", "ps", "jobs",
        "fg", "bg", "wait", "xargs", "pwd", "which", "basename", "dirname"
    };
    return std::find(commands.begin(), commands.end(), word) != commands.end();
}

// Print a Bash script with syntax highlighting
void printColoredScript(const std::vector<std::string>& script) {
    for (const auto& line : script) {
        std::istringstream iss(line);
        std::string word;
        bool isComment = false;

        while (iss >> word) {
            if (word[0] == '#') {  // If a word starts with '#', it's a comment
                isComment = true;
                std::cout << GRAY << word;
            } else if (isComment) {  // Print the rest of the line as a comment
                std::cout << " " << word;
            } else if (word[0] == '"' || word[0] == '\'') {  // Strings
                std::cout << GREEN << word;
            } else if (isCommand(word)) {  // Commands
                std::cout << ANSI_BOLD << ANSI_BLUE << word;
            } else {  // Arguments, options, or unknown words
                std::cout << word;
            }
            std::cout << " " << RESET;  // Reset after each word
        }
        std::cout << "\n";  // New line after each script line
    }
}

void printTree(const std::vector<std::string>& packages, const std::string& title, const std::string& color, const std::string& icon, const std::string& prefix = "  ") {
    if (packages.empty()) return;

    std::cout << ANSI_BOLD << color << prefix << icon << " " << title << RESET << "\n";
    for (size_t i = 0; i < packages.size(); ++i) {
        const bool isLast = i == packages.size() - 1;
        std::cout << prefix << (isLast ? " ╰── " : " ├── ") << GREEN << " " << packages[i] << RESET << "\n";
    }
}

void Package::print(){
    std::cout << "\n";
    std::cout << ANSI_BOLD << CYAN << "╭────────────────────────────────────────────────╮\n";
    std::cout << ANSI_BOLD << CYAN << "│ " << RESET << ANSI_BOLD << name << RESET << "\n";
    std::cout << ANSI_BOLD << CYAN << "│ ├── " << RESET << ANSI_BLUE << "Version: " << RESET << version << "\n";
    std::cout << ANSI_BOLD << CYAN << "│ ├── " << RESET << ANSI_MAGENTA << "Architecture: " << RESET << arch << "\n";
    std::cout << ANSI_BOLD << CYAN << "│ ├── " << RESET << ANSI_YELLOW << "Author: " << RESET << author << "\n";
    std::cout << ANSI_BOLD << CYAN << "│ ╰── " << RESET << ANSI_CYAN << "Description: " << RESET << description << "\n";
    std::cout << ANSI_BOLD << CYAN << "╰────────────────────────────────────────────────╯" << RESET << "\n";

    printTree(provides, "Provides", ANSI_GREEN, "[P]");
    printTree(deps, "Dependencies", ANSI_YELLOW, "[D]");
    printTree(conflicts, "Conflicts", ANSI_RED, "[C]");
    printTree(replaces, "Replaces", ANSI_MAGENTA, "[R]");
    printTree(build_deps, "Build dependencies", ANSI_BLUE, "[B]");
    std::cout << std::endl << "Pre-build script: " << std::endl;
    printColoredScript(pre_build_commands);
    std::cout << std::endl << "Post-build script: " << std::endl;
    printColoredScript(post_build_commands);
    std::cout << std::endl << "Pre-install script: " << std::endl;
    printColoredScript(pre_install_script);
    std::cout << std::endl << "Post-install script: " << std::endl;
    printColoredScript(post_install_script);
    std::cout << std::endl << "Pre-remove script: " << std::endl;
    printColoredScript(pre_remove_script);
    std::cout << std::endl << "Post-remove script: " << std::endl;
    printColoredScript(post_remove_script);
    std::cout << "\n";
}