#pragma once
#include "lexer.h"
#include <string>
#include <vector>

struct ScalarVal {
    bool  is_random = false;
    float value = 0.f;
    float lo = 0.f, hi = 0.f;
};

struct Vec3Val {
    ScalarVal x, y, z;
};

enum class PropKind { SCALAR, VEC3 };

struct Property {
    std::string name;
    PropKind    kind;
    ScalarVal   scalar;
    Vec3Val     vec3;
};

enum class ObjectType { CAMERA, ISLAND, HOUSE, TREE, SPHERE, PLANE };

struct SceneObject {
    ObjectType            type;
    std::vector<Property> props;
};

struct SceneDescription {
    std::vector<SceneObject> objects;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    SceneDescription parse();

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    Token&     peek(int offset = 0);
    Token      consume();
    Token      expect(TokenType t, const std::string& ctx);
    bool       check(TokenType t) const;
    bool       isObjectKeyword(TokenType t) const;
    ObjectType tokenToObjectType(TokenType t);

    SceneObject parseObject(ObjectType type);
    Property    parseProperty();
    ScalarVal   parseScalar();
    Vec3Val     parseVec3();
    void        parseFor(SceneDescription& scene);
};
