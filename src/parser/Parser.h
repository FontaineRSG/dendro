//
// Created by cv2 on 2/25/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <filesystem>
#include "Package.h"

class Parser {
public:
    static Package parseSpec(const std::filesystem::path &path);
};

#endif //PARSER_H
