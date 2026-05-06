#include "parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

Token& Parser::peek(int offset) {
    size_t idx = pos + offset;
    if (idx >= tokens.size()) return tokens.back();
    return tokens[idx];
}

Token Parser::consume() { return tokens[pos++]; }

Token Parser::expect(TokenType t, const std::string& ctx) {
    if (peek().type != t)
        throw std::runtime_error("line " + std::to_string(peek().line) +
            " [" + ctx + "]: unexpected '" + peek().value + "'");
    return consume();
}

bool Parser::check(TokenType t) const {
    return pos < tokens.size() && tokens[pos].type == t;
}

bool Parser::isObjectKeyword(TokenType t) const {
    return t == TokenType::KW_CAMERA || t == TokenType::KW_ISLAND ||
           t == TokenType::KW_HOUSE  || t == TokenType::KW_TREE   ||
           t == TokenType::KW_SPHERE || t == TokenType::KW_PLANE;
}

ObjectType Parser::tokenToObjectType(TokenType t) {
    switch (t) {
        case TokenType::KW_CAMERA: return ObjectType::CAMERA;
        case TokenType::KW_ISLAND: return ObjectType::ISLAND;
        case TokenType::KW_HOUSE:  return ObjectType::HOUSE;
        case TokenType::KW_TREE:   return ObjectType::TREE;
        case TokenType::KW_SPHERE: return ObjectType::SPHERE;
        case TokenType::KW_PLANE:  return ObjectType::PLANE;
        default: throw std::runtime_error("not an object keyword");
    }
}

ScalarVal Parser::parseScalar() {
    ScalarVal s;
    if (peek().type == TokenType::KW_RANDOM) {
        consume();
        expect(TokenType::LPAREN, "random");
        s.lo = std::stof(expect(TokenType::NUMBER, "random lo").value);
        expect(TokenType::COMMA, "random");
        s.hi = std::stof(expect(TokenType::NUMBER, "random hi").value);
        expect(TokenType::RPAREN, "random");
        s.is_random = true;
    } else {
        s.value = std::stof(expect(TokenType::NUMBER, "scalar").value);
    }
    return s;
}

Vec3Val Parser::parseVec3() {
    expect(TokenType::LPAREN, "vec3");
    Vec3Val v;
    v.x = parseScalar(); expect(TokenType::COMMA, "vec3");
    v.y = parseScalar(); expect(TokenType::COMMA, "vec3");
    v.z = parseScalar();
    expect(TokenType::RPAREN, "vec3");
    return v;
}

Property Parser::parseProperty() {
    Property prop;
    prop.name = expect(TokenType::IDENT, "prop name").value;
    expect(TokenType::COLON, "prop");
    if (peek().type == TokenType::LPAREN) {
        prop.kind = PropKind::VEC3;
        prop.vec3 = parseVec3();
    } else {
        prop.kind   = PropKind::SCALAR;
        prop.scalar = parseScalar();
    }
    return prop;
}

SceneObject Parser::parseObject(ObjectType type) {
    SceneObject obj;
    obj.type = type;
    expect(TokenType::LBRACE, "object");
    while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE))
        obj.props.push_back(parseProperty());
    expect(TokenType::RBRACE, "object");
    return obj;
}

void Parser::parseFor(SceneDescription& scene) {
    expect(TokenType::IDENT,  "for var");
    expect(TokenType::KW_IN,  "for in");
    int start = std::stoi(expect(TokenType::NUMBER, "for start").value);
    expect(TokenType::DOTDOT, "for ..");
    int end   = std::stoi(expect(TokenType::NUMBER, "for end").value);
    expect(TokenType::LBRACE, "for body");

    size_t bodyStart = pos;
    int depth = 1;
    while (depth > 0 && !check(TokenType::END_OF_FILE)) {
        if (peek().type == TokenType::LBRACE) depth++;
        else if (peek().type == TokenType::RBRACE) depth--;
        consume();
    }
    size_t bodyEnd = pos;

    for (int i = start; i < end; i++) {
        pos = bodyStart;
        while (pos < bodyEnd - 1) {
            TokenType t = peek().type;
            if (isObjectKeyword(t))
                scene.objects.push_back(parseObject(tokenToObjectType(consume().type)));
            else
                consume();
        }
    }
    pos = bodyEnd;
}

SceneDescription Parser::parse() {
    SceneDescription scene;
    while (!check(TokenType::END_OF_FILE)) {
        TokenType t = peek().type;
        if (isObjectKeyword(t))
            scene.objects.push_back(parseObject(tokenToObjectType(consume().type)));
        else if (t == TokenType::KW_FOR) { consume(); parseFor(scene); }
        else consume();
    }
    return scene;
}
