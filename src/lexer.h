#pragma once
#include <string>
#include <vector>

enum class TokenType {
    NUMBER, STRING, IDENT,
    KW_CAMERA, KW_ISLAND, KW_HOUSE, KW_TREE, KW_SPHERE, KW_PLANE,
    KW_FOR, KW_IN, KW_RANDOM,
    LBRACE, RBRACE, LPAREN, RPAREN, COLON, COMMA, DOTDOT, DOT,
    END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType   type;
    std::string value;
    int         line;
    Token(TokenType t, std::string v, int l)
        : type(t), value(std::move(v)), line(l) {}
};

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();
private:
    std::string src;
    size_t      pos  = 0;
    int         line = 1;
    char        peek(int offset = 0) const;
    char        advance();
    void        skipWhitespaceAndComments();
    Token       readNumber();
    Token       readString();
    Token       readIdentOrKeyword();
    TokenType   keywordOrIdent(const std::string& word);
};
