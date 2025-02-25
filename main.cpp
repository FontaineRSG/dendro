#include <iostream>
#include <Parser.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <spec-file>\n";
        return 1;
    }

    Parser::parseSpec(argv[1]).print();
}
