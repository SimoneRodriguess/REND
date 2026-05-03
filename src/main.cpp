#include "lexer.h"
#include <fstream>
#include <iostream>
#include <sstream>

static const char* typeName(TokenType t) {
    switch (t) {
        case TokenType::NUMBER:      return "NUMBER";
        case TokenType::STRING:      return "STRING";
        case TokenType::IDENT:       return "IDENT";
        case TokenType::KW_CAMERA:   return "KW_CAMERA";
        case TokenType::KW_ISLAND:   return "KW_ISLAND";
        case TokenType::KW_HOUSE:    return "KW_HOUSE";
        case TokenType::KW_TREE:     return "KW_TREE";
        case TokenType::KW_SPHERE:   return "KW_SPHERE";
        case TokenType::KW_PLANE:    return "KW_PLANE";
        case TokenType::KW_FOR:      return "KW_FOR";
        case TokenType::KW_IN:       return "KW_IN";
        case TokenType::KW_RANDOM:   return "KW_RANDOM";
        case TokenType::LBRACE:      return "LBRACE";
        case TokenType::RBRACE:      return "RBRACE";
        case TokenType::LPAREN:      return "LPAREN";
        case TokenType::RPAREN:      return "RPAREN";
        case TokenType::COLON:       return "COLON";
        case TokenType::COMMA:       return "COMMA";
        case TokenType::DOTDOT:      return "DOTDOT";
        case TokenType::DOT:         return "DOT";
        case TokenType::END_OF_FILE: return "EOF";
        default:                     return "UNKNOWN";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "Usage: rend <scene.rend>\n"; return 1; }
    std::ifstream file(argv[1]);
    if (!file) { std::cerr << "Error: cannot open '" << argv[1] << "'\n"; return 1; }
    std::ostringstream ss;
    ss << file.rdbuf();
    try {
        Lexer lexer(ss.str());
        auto tokens = lexer.tokenize();
        std::cout << "── REND Lexer (" << tokens.size() << " tokens) ──\n";
        for (auto& tok : tokens)
            std::printf("  [line %2d]  %-14s  %s\n", tok.line, typeName(tok.type), tok.value.c_str());
    } catch (const std::exception& e) {
        std::cerr << "Lex error: " << e.what() << "\n"; return 1;
    }
    return 0;
}
