#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "usage: rend <scene.rend>\n"; return 1; }
    std::ifstream file(argv[1]);
    if (!file)    { std::cerr << "cannot open '" << argv[1] << "'\n"; return 1; }
    std::ostringstream ss;
    ss << file.rdbuf();

    try {
        Lexer lexer(ss.str());
        Parser parser(lexer.tokenize());
        SceneDescription scene = parser.parse();

        std::string base = std::string(argv[1]);
        base = base.substr(0, base.find_last_of('.'));
        generateOBJ(scene, base + ".obj", base + ".mtl");

        std::cout << "wrote " << base << ".obj + .mtl\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
