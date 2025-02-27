//
// Created by cv2 on 2/25/25.
//

#include "Parser.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include "colors.h"
#include <algorithm>
#include <random>
#include <regex>

bool isKeyword(const std::string& s) {
    return !s.empty() && s.starts_with('&');
}

// Substitute keywords in a string
std::string substituteKeywords(const std::string& input, const std::string& current_pkg_name) {
    std::regex keyword_regex(R"(&(\w+)(?:\(([^)]*)\))?)");
    std::sregex_iterator it(input.begin(), input.end(), keyword_regex);
    std::sregex_iterator end;

    std::string result;
    size_t last_pos = 0;

    for (; it != end; ++it) {
        const std::smatch& match = *it;
        size_t match_pos = match.position();
        size_t match_length = match.length();

        // Append part before the match
        result += input.substr(last_pos, match_pos - last_pos);

        std::string keyword = match[1];
        std::string arg = match[2];
        std::string replacement;

        if (keyword == "dendro_package") {
            if (!arg.empty()) {
                throw std::runtime_error("Keyword &dendro_package does not accept arguments");
            }
            replacement = "/var/dendro/packages/" + current_pkg_name;
        } else if (keyword == "dendro_buildroot") {
            std::string package = arg.empty() ? current_pkg_name : arg;
            replacement = "/var/dendro/" + package + "/buildroot";
        } else if (keyword == "dendro_temp") {
            std::string package = arg.empty() ? current_pkg_name : arg;
            replacement = "/var/dendro/" + package + "/temp";
        } else if (keyword == "dendro_package") {
            std::string package = arg.empty() ? current_pkg_name : arg;
            replacement = "/var/dendro/" + package + "/anemopack";
        } else {
            throw std::runtime_error("Unknown keyword: &" + keyword);
        }

        result += replacement;
        last_pos = match_pos + match_length;
    }

    // Append remaining part
    result += input.substr(last_pos);

    return result;
}

std::string resolveKeyword(const std::string &value, const std::string &current_package) {
    if (value.starts_with("&dendro_buildroot")) {
        size_t start = value.find('(');
        if (start != std::string::npos) {
            size_t end = value.find(')', start);
            if (end != std::string::npos) {
                std::string pkg_name = value.substr(start + 1, end - start - 1);
                return "/var/dendro/" + pkg_name + "/buildroot";
            }
        }
        return "/var/dendro/" + current_package + "/buildroot";
    }

    if (value.starts_with("&dendro_temp")) {
        size_t start = value.find('(');
        if (start != std::string::npos) {
            size_t end = value.find(')', start);
            if (end != std::string::npos) {
                std::string pkg_name = value.substr(start + 1, end - start - 1);
                return "/var/dendro/" + pkg_name + "/temp";
            }
        }
        return "/var/dendro/" + current_package + "/temp";
    }

    if (value == "&dendro_package") {
        return "/var/dendro/" + current_package;
    }

    return value;
}

// Process a vector of strings for keywords
void processVector(std::vector<std::string>& vec, const std::string& current_pkg_name) {
    for (auto& str : vec) {
        str = substituteKeywords(str, current_pkg_name);
    }
}

// Process all fields in Package for keywords
void processKeywords(Package& pkg) {
    pkg.source = substituteKeywords(pkg.source, pkg.name);

    processVector(pkg.build_commands, pkg.name);
    processVector(pkg.pre_build_commands, pkg.name);
    processVector(pkg.post_build_commands, pkg.name);
    processVector(pkg.pre_install_script, pkg.name);
    processVector(pkg.post_install_script, pkg.name);
    processVector(pkg.pre_remove_script, pkg.name);
    processVector(pkg.post_remove_script, pkg.name);
}

// Trim leading whitespaces
std::string ltrim(const std::string &s) {
    auto start = std::ranges::find_if(s, [](const unsigned char ch) {
        return !std::isspace(ch);
    });
    return {start, s.end()};
}

// Check if a string is an integer
bool isInteger(const std::string &s) {
    if (s.empty()) return false;
    if (s[0] == '-' || s[0] == '+') return s.size() > 1 && std::all_of(s.begin() + 1, s.end(), ::isdigit);
    return std::ranges::all_of(s, ::isdigit);
}

// Check if a string is a floating-point number
bool isFloat(const std::string &s) {
    std::istringstream iss(s);
    float f;
    iss >> std::noskipws >> f;  // Try parsing float
    return iss.eof() && !iss.fail();
}

// Split comma-separated values inside `{}` brackets
std::vector<std::string> splitIfBraced(const std::string &s, const int line) {
    if (s.empty()) return {};

    if (s.front() == '{' && s.back() != '}') {
        std::cerr << "Error on line " + std::to_string(line) + ": Missing closing '}' in list" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (s.front() == '{' && s.back() == '}') {
        std::vector<std::string> result;
        size_t start = 1, end;
        while ((end = s.find(',', start)) != std::string::npos && end < s.size() - 1) {
            result.push_back(s.substr(start, end - start));
            start = end + 1;
        }
        result.push_back(ltrim(s.substr(start, s.size() - start - 1))); // Last element, ignore `}`
        return result;
    }
    return {s}; // Not a braced list, return single-element vector
}

// Parse metadata
void parseMetadata(const std::vector<std::string>& data, Package& pkg) {
    std::cout << "Parsing metadata...\n";

    std::map<std::string, std::string> metadata;
    std::map<std::string, std::vector<std::string>> vectorMetadata;

    int line_num = 0;
    for (const auto& item : data) {
        line_num++;

        // Skip comments and empty lines
        if (item.empty() || item[0] == '#' || item[0] == '\n') {
            continue;
        }

        // Find key-value separator
        const size_t colon = item.find(':');
        if (colon == std::string::npos) {
            throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Missing ':' in key-value pair.");
        }

        std::string key = item.substr(0, colon);
        std::string value = ltrim(item.substr(colon + 1));

        // Trim key to avoid " name: " issues
        key = ltrim(key);

        // Type deduction & error checking
        if (value.front() == '"') { // String
            if (value.back() != '"') {
                throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Missing closing '\"' for string value.");
            }
            metadata[key] = value.substr(1, value.size() - 2); // Remove quotes
        } else if (value.front() == '\'') { // Char
            if (value.size() != 3 || value.back() != '\'') {
                throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Invalid char format, should be 'X'.");
            }
            metadata[key] = std::string(1, value[1]); // Extract character
        } else if (value.front() == '{') { // Vector
            if (value.back() != '}') {
                throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Missing closing '}' for vector.");
            }
            vectorMetadata[key] = splitIfBraced(value, line_num);
        } else if (isInteger(value)) { // Integer
            metadata[key] = value;
        } else if (isFloat(value)) { // Float
            metadata[key] = value;
        } else { // Default: String
            metadata[key] = value;
        }
    }

    // Assign values to Package struct
    pkg.name = metadata["name"];
    pkg.author = metadata["author"];
    pkg.arch = metadata["arch"];
    pkg.version = metadata["version"];
    pkg.description = metadata["description"];
    pkg.is_protected = metadata["protected"] == "true" || metadata["protected"] == "1";

    // Assign vectors properly
    pkg.build_deps = vectorMetadata["build_deps"];
    pkg.provides = vectorMetadata["provides"];
    pkg.deps = vectorMetadata["deps"];
    pkg.replaces = vectorMetadata["replaces"];
    pkg.source = metadata["source"];
    pkg.validpgpkeys = vectorMetadata["validpgpkeys"];
}

Package Parser::parseSpec(const std::filesystem::path& path) {
    Package pkg;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << std::endl;
        std::cout << ANSI_BOLD << RED << "╭────────────────────────────────────────────────╮" << RESET << std::endl;
        std::cout << ANSI_BOLD << RED << "│           Failed to open .spec file!           |" << RESET << std::endl;
        std::cout << ANSI_BOLD << RED << "╰────────────────────────────────────────────────╯" << RESET << std::endl;
        std::cout << std::endl;
        exit(EXIT_FAILURE);
    }



    std::vector<std::string> metadata_contents;
    std::vector<std::string> pre_build_commands, post_build_commands, build_commands;
    std::vector<std::string> pre_install_script, post_install_script;
    std::vector<std::string> pre_remove_script, post_remove_script;

    bool metadata_flag = false, pre_flag = false, post_flag = false;
    bool pre_install_flag = false, post_install_flag = false;
    bool pre_remove_flag = false, post_remove_flag = false, build_flag = false;

    std::string line;
    int line_num = 0;

    while (std::getline(file, line)) {
        line_num++;

        // Metadata parsing
        if (line.contains("[metadata]")) {
            metadata_flag = true;
            continue;
        }
        if (line.contains("[/metadata]")) {
            if (!metadata_flag) {
                throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/metadata]");
            }
            parseMetadata(metadata_contents, pkg);
            metadata_flag = false;
            continue;
        }
        if (metadata_flag) {
            metadata_contents.emplace_back(line);
            continue;
        }

        // Handle [build-commands]
        if (line.contains("[build-commands]")) {
            build_flag = true;
            continue;
        }
        if (line.contains("[/build-commands]")) {
            if (!build_flag) {
                throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/build-commands]");
            }
            build_flag = false;
            pkg.build_commands = build_commands;
            continue;
        }
        if (build_flag) {
            build_commands.emplace_back(line);
            continue;
        }

        // Pre-build commands
        if (line.contains("[pre-build-commands]")) {
            pre_flag = true;
            continue;
        }
        if (line.contains("[/pre-build-commands]")) {
            if (!pre_flag) throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/pre-build-commands]");
            pre_flag = false;
            pkg.pre_build_commands = pre_build_commands;
            continue;
        }
        if (pre_flag) {
            pre_build_commands.emplace_back(line);
            continue;
        }

        // Post-build commands
        if (line.contains("[post-build-commands]")) {
            post_flag = true;
            continue;
        }
        if (line.contains("[/post-build-commands]")) {
            if (!post_flag) throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/post-build-commands]");
            post_flag = false;
            pkg.post_build_commands = post_build_commands;
            continue;
        }
        if (post_flag) {
            post_build_commands.emplace_back(line);
            continue;
        }

        // Pre-install script
        if (line.contains("[pre-install-script]")) {
            pre_install_flag = true;
            continue;
        }
        if (line.contains("[/pre-install-script]")) {
            if (!pre_install_flag) throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/pre-install-script]");
            pre_install_flag = false;
            pkg.pre_install_script = pre_install_script;
            continue;
        }
        if (pre_install_flag) {
            pre_install_script.emplace_back(line);
            continue;
        }

        // Post-install script
        if (line.contains("[post-install-script]")) {
            post_install_flag = true;
            continue;
        }
        if (line.contains("[/post-install-script]")) {
            if (!post_install_flag) throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/post-install-script]");
            post_install_flag = false;
            pkg.post_install_script = post_install_script;
            continue;
        }
        if (post_install_flag) {
            post_install_script.emplace_back(line);
            continue;
        }

        // Pre-remove script
        if (line.contains("[pre-remove-script]")) {
            pre_remove_flag = true;
            continue;
        }
        if (line.contains("[/pre-remove-script]")) {
            if (!pre_remove_flag) throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/pre-remove-script]");
            pre_remove_flag = false;
            pkg.pre_remove_script = pre_remove_script;
            continue;
        }
        if (pre_remove_flag) {
            pre_remove_script.emplace_back(line);
            continue;
        }

        // Post-remove script
        if (line.contains("[post-remove-script]")) {
            post_remove_flag = true;
            continue;
        }
        if (line.contains("[/post-remove-script]")) {
            if (!post_remove_flag) throw std::runtime_error("Error on line " + std::to_string(line_num) + ": Unmatched [/post-remove-script]");
            post_remove_flag = false;
            pkg.post_remove_script = post_remove_script;
            continue;
        }
        if (post_remove_flag) {
            post_remove_script.emplace_back(line);
            continue;
        }
    }

    // After the while loop ends, check for any unclosed sections
    std::vector<std::pair<bool*, std::string>> section_flags = {
        {&metadata_flag, "[metadata]"},
        {&build_flag, "[build-commands]"},
        {&pre_flag, "[pre-build-commands]"},
        {&post_flag, "[post-build-commands]"},
        {&pre_install_flag, "[pre-install-script]"},
        {&post_install_flag, "[post-install-script]"},
        {&pre_remove_flag, "[pre-remove-script]"},
        {&post_remove_flag, "[post-remove-script]"}
    };

    for (const auto& [flag, name] : section_flags) {
        if (*flag) {
            throw std::runtime_error("Unclosed " + name + " section");
        }
    }

    file.close();

    processKeywords(pkg);

    return pkg;
}


