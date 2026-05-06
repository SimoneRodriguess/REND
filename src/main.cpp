#include "lexer.h"
#include "parser.h"
#include <fstream>
#include <iostream>
#include <sstream>

static const char* objName(ObjectType t) {
    switch (t) {
        case ObjectType::CAMERA: return "camera";
        case ObjectType::ISLAND: return "island";
        case ObjectType::HOUSE:  return "house";
        case ObjectType::TREE:   return "tree";
        case ObjectType::SPHERE: return "sphere";
        case ObjectType::PLANE:  return "plane";
        default:                 return "unknown";
    }
}

static void printScalar(const ScalarVal& s) {
    if (s.is_random) std::printf("random(%.2f, %.2f)", s.lo, s.hi);
    else             std::printf("%.3f", s.value);
}

static void printScene(const SceneDescription& scene) {
    std::printf("REND Scene — %zu objects\n", scene.objects.size());
    for (auto& obj : scene.objects) {
        std::printf("\n[%s]\n", objName(obj.type));
        for (auto& prop : obj.props) {
            std::printf("  %-16s = ", prop.name.c_str());
            if (prop.kind == PropKind::VEC3) {
                std::printf("(");
                printScalar(prop.vec3.x); std::printf(", ");
                printScalar(prop.vec3.y); std::printf(", ");
                printScalar(prop.vec3.z); std::printf(")");
            } else {
                printScalar(prop.scalar);
            }
            std::printf("\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "usage: rend <scene.rend>\n"; return 1; }
    std::ifstream file(argv[1]);
    if (!file)    { std::cerr << "cannot open '" << argv[1] << "'\n"; return 1; }
    std::ostringstream ss;
    ss << file.rdbuf();

    try {
        Lexer lexer(ss.str());
        Parser parser(lexer.tokenize());
        printScene(parser.parse());
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
