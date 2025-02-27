#include <iostream>
#include <Parser.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <regex>
#include <vector>
#include "Package.h"
#include "Parser.h"
#include <curl/curl.h>

namespace fs = std::filesystem;

class BuildSystem {
    Package pkg;
    fs::path build_root;
    fs::path anemo_pack_dir;
    fs::path final_pkg_path;

public:
    explicit BuildSystem(const Package& package) : pkg(package) {
        build_root = fs::path("/var/dendro") / pkg.name / "buildroot";
        anemo_pack_dir = fs::path("/var/dendro") / pkg.name / "anemopack";
        final_pkg_path = fs::current_path() / (pkg.name + "-" + pkg.version + ".apkg");
    }
    // TODO implement anemo list in Anemo pkg manager
    bool checkDependencies() {
        // std::cout << "Checking build dependencies...\n";
        // for (const auto& dep : pkg.build_deps) {
        //     int result = std::system(("anemo info " + dep + " > /dev/null 2>&1").c_str());
        //     if (result != 0) {
        //         std::cerr << "Missing build dependency: " << dep << "\n";
        //         return false;
        //     }
        // }
        return true;
    }

    void createBuildDirs() {
        fs::create_directories(build_root);
        fs::create_directories(anemo_pack_dir);
        fs::create_directories(anemo_pack_dir / "package");
    }

    static size_t writeData(void* ptr, size_t size, size_t nmemb, FILE* stream) {
        return fwrite(ptr, size, nmemb, stream);
    }

    void importGpgKeys(const std::vector<std::string>& keyIds) {
        // Create temporary keyring directory
        const fs::path keyringDir = build_root / "temp_keyring";
        fs::create_directories(keyringDir);

        // Set custom keyring location
        const std::string keyringOpt = "--no-default-keyring --keyring " +
                                      (keyringDir / "trustedkeys.gpg").string();

        for (const auto& keyId : keyIds) {
            // First try to receive the key from keyserver
            std::string fetchCmd = "gpg " + keyringOpt +
                                 " --keyserver hkps://keyserver.ubuntu.com" +
                                 " --recv-keys " + keyId + " 2>&1";

            // Execute and check result
            if (std::system(fetchCmd.c_str()) != 0) {
                fs::remove_all(keyringDir);
                throw std::runtime_error("Failed to fetch key: " + keyId);
            }

            // Trust key ultimately in temporary keyring
            std::string trustCmd = "echo \"" + keyId +
                                 ":6:\" | gpg " + keyringOpt +
                                 " --import-ownertrust 2>/dev/null";
            if (std::system(trustCmd.c_str()) != 0) {
                fs::remove_all(keyringDir);
                throw std::runtime_error("Failed to trust key: " + keyId);
            }
        }

        // Set environment variable for git to use our temporary keyring
        setenv("GNUPGHOME", keyringDir.c_str(), 1);
    }


static bool downloadUrl(const std::string& url, const std::string& outputFile) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    FILE* fp = fopen(outputFile.c_str(), "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: DendroBuildSystem");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    fclose(fp);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}

    void handleGitSource(const std::string& sourceUrl,
                    const std::vector<std::string>& validPgpKeys) {
        std::regex pattern(R"(git\+(https?://.+?)(\?signed)?#tag=([\w\.-]+))");
        std::smatch matches;

        if (!std::regex_match(sourceUrl, matches, pattern)) {
            throw std::runtime_error("Invalid Git source format: " + sourceUrl);
        }

        const std::string repoUrl = matches[1].str();
        const bool signedTag = matches[2].matched;
        const std::string tag = matches[3].str();

        try {
            // Import keys if needed
            if (signedTag && !validPgpKeys.empty()) {
                importGpgKeys(validPgpKeys);
            }

            // Clone with full history for tag verification
            std::string cloneCmd = "git clone --branch " + tag +
                                  " " + repoUrl + " " + build_root.string();

            if (std::system(cloneCmd.c_str()) != 0) {
                throw std::runtime_error("Clone failed: " + repoUrl);
            }

            // Verify signed tag if requested
            if (signedTag) {
                std::string verifyCmd = "git -C " + build_root.string() +
                                       " verify-tag " + tag;

                if (std::system(verifyCmd.c_str()) != 0) {
                    throw std::runtime_error("Tag verification failed: " + tag);
                }
            }
        }
        catch (...) {
            fs::remove_all(build_root);
            throw;
        }
    }

void downloadAndPrepareSource() {
    if (pkg.source.empty()) return;
        const std::vector<std::string>& validPgpKeys = pkg.validpgpkeys;

    // Handle Git repositories with signed tags
    if (pkg.source.find("git+") == 0) {
        handleGitSource(pkg.source, validPgpKeys);
        return;
    }


    // Handle tar archives
    if (pkg.source.find("tar+") == 0) {
        std::string tarUrl = pkg.source.substr(4);
        if (!downloadUrl(tarUrl, "temp_source.tar")) {
            throw std::runtime_error("Failed to download tar archive: " + tarUrl);
        }
        extractAndFlattenTarball();
        fs::remove("temp_source.tar");
        return;
    }

    // Handle local files
    if (pkg.source.find("local+") == 0) {
        fs::path sourcePath = pkg.source.substr(6);

        if (!fs::exists(sourcePath)) {
            throw std::runtime_error("Local path does not exist: " + sourcePath.string());
        }

        if (fs::is_directory(sourcePath)) {
            fs::copy(sourcePath, build_root, fs::copy_options::recursive);
        } else {
            fs::copy(sourcePath, build_root / sourcePath.filename());
        }
        return;
    }

    // Existing handling for regular URLs and local directories
    if (fs::exists(pkg.source) && fs::is_directory(pkg.source)) {
        fs::copy(pkg.source, build_root, fs::copy_options::recursive);
        return;
    }

    // Handle regular URLs
    if (!downloadUrl(pkg.source, "temp_source.tar")) {
        throw std::runtime_error("Failed to download source: " + pkg.source);
    }
    extractAndFlattenTarball();
    fs::remove("temp_source.tar");
}


    void extractAndFlattenTarball() {
        // Extract tarball
        std::system(("tar xf temp_source.tar -C " + build_root.string()).c_str());
        fs::remove("temp_source.tar.gz");

        // Flatten directory structure
        auto dir_iter = fs::directory_iterator(build_root);
        std::vector<fs::path> dir_contents;

        // Find all extracted items
        for (const auto& entry : dir_iter) {
            dir_contents.push_back(entry.path());
        }

        // If we only have one directory, move its contents up
        if (dir_contents.size() == 1 && fs::is_directory(dir_contents[0])) {
            const auto temp_dir = dir_contents[0];
            for (const auto& entry : fs::directory_iterator(temp_dir)) {
                fs::rename(entry.path(), build_root / entry.path().filename());
            }
            fs::remove(temp_dir);
        }
    }

    void executeCommands(const std::vector<std::string>& commands, const std::string& phase) {
        if (commands.empty()) return;

        std::cout << "Executing " << phase << " commands...\n";

        // Create temporary script in build root
        const auto script_path = build_root / "dendro_build_script.sh";
        {
            std::ofstream script(script_path);
            script << "#!/bin/sh\n"
               << "set -e\n"
               << "cd \"" << build_root.string() << "\"\n"
               << "echo '=== Build environment ==='\n"
               << "ls -lah\n"    // Properly terminated command
               << "pwd\n"        // Properly terminated command
               << "echo '========================='\n";

            for (const auto& cmd : commands) {
                script << "echo '▶▶ Running: " << cmd << "'\n"
                       << cmd << "\n";
            }
        }

        // Make script executable
        fs::permissions(script_path,
            fs::perms::owner_all | fs::perms::group_read | fs::perms::others_read,
            fs::perm_options::replace);

        // Execute with fakeroot in clean environment
        const std::string full_cmd = "fakeroot -- sh \"" + script_path.string() + "\"";
        const int result = std::system(full_cmd.c_str());

        // Cleanup script
        fs::remove(script_path);

        if (result != 0) {
            throw std::runtime_error(phase + " commands failed with exit code " + std::to_string(result));
        }
    }

    void generateAnemonixYaml() {
        std::ofstream yaml(anemo_pack_dir / "anemonix.yaml");
        yaml << "name: " << pkg.name << "\n"
             << "version: " << pkg.version << "\n"
             << "arch: " << pkg.arch << "\n"
             << "description: " << pkg.description << "\n"
             << "dependencies:\n";
        for (const auto& dep : pkg.deps) {
            yaml << "  - " << dep << "\n";
        }
        yaml.close();
    }

    void writeScripts() {
        auto writeScript = [this](const std::vector<std::string>& lines, const std::string& name) {
            if (lines.empty()) return;
            std::ofstream script(anemo_pack_dir / (name + ".anemonix"));
            for (const auto& line : lines) {
                script << line << "\n";
            }
        };

        writeScript(pkg.pre_install_script, "preinstall");
        writeScript(pkg.post_install_script, "postinstall");
        writeScript(pkg.pre_remove_script, "preremove");
        writeScript(pkg.post_remove_script, "postremove");
    }

    void createPackage() {
        // Copy built files
        fs::copy(build_root / "install", anemo_pack_dir / "package", fs::copy_options::recursive);

        // Create tarball
        std::string tar_cmd = "tar czf " + final_pkg_path.string() + " -C " +
                             anemo_pack_dir.string() + " .";
        int result = std::system(tar_cmd.c_str());
        if (result != 0) {
            throw std::runtime_error("Failed to create package tarball");
        }
    }

    void cleanUp() {
        fs::remove_all(build_root);
        fs::remove_all(anemo_pack_dir);
    }

    void build() {
        try {
            if (!checkDependencies()) return;

            createBuildDirs();
            downloadAndPrepareSource();

            executeCommands(pkg.pre_build_commands, "pre-build");
            executeCommands(pkg.build_commands, "build");
            executeCommands(pkg.post_build_commands, "post-build");

            generateAnemonixYaml();
            writeScripts();
            createPackage();
            //cleanUp();

            std::cout << "Package built successfully: " << final_pkg_path << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Build failed: " << e.what() << "\n";
            cleanUp();
            exit(EXIT_FAILURE);
        }
    }
};

// Usage:
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: dendro-build <specfile>\n";
        return EXIT_FAILURE;
    }

    Parser parser;
    Package pkg = Parser::parseSpec(argv[1]);
    pkg.print();

    BuildSystem builder(pkg);
    builder.build();

    return EXIT_SUCCESS;
}
