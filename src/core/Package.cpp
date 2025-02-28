//
// Created by cv2 on 2/25/25.
//

#include "Package.h"
#include <sstream>
#define GRAY    "\033[90m"
#define WHITE   "\033[97m"
#include <yaml-cpp/yaml.h>

#include "colors.h"

void printTree(const std::vector<std::string>& packages, const std::string& title, const std::string& color, const std::string& icon, const std::string& prefix = "  ") {
    if (packages.empty()) return;

    std::cout << ANSI_BOLD << color << prefix << icon << " " << title << RESET << "\n";
    for (size_t i = 0; i < packages.size(); ++i) {
        const bool isLast = i == packages.size() - 1;
        std::cout << prefix << (isLast ? " ╰── " : " ├── ") << GREEN << " " << packages[i] << RESET << "\n";
    }
}

void Package::print(bool child){

    if (!child) {
        std::cout << "\n";
        std::cout << ANSI_BOLD << CYAN << "╭────────────────────────────────────────────────╮\n";
        std::cout << ANSI_BOLD << CYAN << "│ " << RESET << ANSI_BOLD << name << RESET << "\n";
        std::cout << ANSI_BOLD << CYAN << "│ ├── " << RESET << ANSI_BLUE << "Version: " << RESET << version << "\n";
        std::cout << ANSI_BOLD << CYAN << "│ ├── " << RESET << ANSI_MAGENTA << "Architecture: " << RESET << arch << "\n";
        std::cout << ANSI_BOLD << CYAN << "│ ├── " << RESET << ANSI_YELLOW << "Author: " << RESET << author << "\n";
        std::cout << ANSI_BOLD << CYAN << "│ ╰── " << RESET << ANSI_CYAN << "Description: " << RESET << description << "\n";
        std::cout << ANSI_BOLD << CYAN << "╰────────────────────────────────────────────────╯" << RESET << "\n";
    }
    else {
        std::cout << "\n";
        std::cout << ANSI_BOLD << name << RESET << "\n";
        std::cout << ANSI_BLUE << "Version: " << RESET << version << "\n";
        std::cout << ANSI_MAGENTA << "Architecture: " << RESET << arch << "\n";
        std::cout << ANSI_YELLOW << "Author: " << RESET << author << "\n";
        std::cout << ANSI_CYAN << "Description: " << RESET << description << "\n";
    }

    printTree(provides, "Provides", ANSI_GREEN, "[P]");
    printTree(deps, "Dependencies", ANSI_YELLOW, "[D]");
    printTree(conflicts, "Conflicts", ANSI_RED, "[C]");
    printTree(replaces, "Replaces", ANSI_MAGENTA, "[R]");
    printTree(build_deps, "Build dependencies", ANSI_BLUE, "[B]");
    if (!child) std::cout << "Package variants: " << std::endl;
    for (auto pkg : variants ) {
        pkg.print(true);
    }
}
