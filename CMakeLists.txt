cmake_minimum_required(VERSION 3.10)
project(dendro)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define source files
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)
file(GLOB_RECURSE SOURCES src/core/*.cpp)
file(GLOB_RECURSE SOURCES src/buildsystem/.cpp)
list(APPEND SOURCES src/main.cpp)

# Find dependencies
find_package(yaml-cpp REQUIRED)
find_package(CURL REQUIRED)

# Create executable
add_executable(dendro ${SOURCES} ${HEADERS}
        src/buildsystem/importGpgKeys.cpp
        src/buildsystem/downloadURL.cpp
        src/buildsystem/handleGitSource.cpp
        src/buildsystem/processSource.cpp
        src/buildsystem/extractAndFlattenTarball.cpp
        src/buildsystem/generateAnemonixYaml.cpp
        src/buildsystem/createPackage.cpp
        src/buildsystem/build.cpp
        src/buildsystem/checkDependencies.cpp
        src/buildsystem/executeScript.cpp
        src/core/Package.cpp)

# For macOS Homebrew installations
if(APPLE)
    # These paths are typically not needed if find_package works correctly
    # Include yaml-cpp headers
    include_directories(/opt/homebrew/opt/yaml-cpp/include)
    # Link yaml-cpp library
    link_directories(/opt/homebrew/opt/yaml-cpp/lib)
endif()

# Link libraries using modern CMake targets
target_link_libraries(dendro PRIVATE
        CURL::libcurl
        yaml-cpp::yaml-cpp  # Use the official target name
)

# Include directories
target_include_directories(dendro PRIVATE
        src/
        src/core
        src/utils
        src/buildsystem
)