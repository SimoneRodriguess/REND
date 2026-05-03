#include "lexer.h"
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"camera", TokenType::KW_CAMERA}, {"island", TokenType::KW_ISLAND},
    {"house",  TokenType::KW_HOUSE},  {"tree",   TokenType::KW_TREE},
    {"sphere", TokenType::KW_SPHERE}, {"plane",  TokenType::KW_PLANE},
    {"for",    TokenType::KW_FOR},    {"in",     TokenType::KW_IN},
    {"random", TokenType::KW_RANDOM},
};

Lexer::Lexer(std::string source) : src(std::move(source)) {}

char Lexer::peek(int offset) const {
    size_t idx = pos + offset;
    return idx < src.size() ? src[idx] : '\0';
}

char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') line++;
    return c;
}

void Lexer::skipWhitespaceAndComments() {
    while (pos < src.size()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '#') {
            while (pos < src.size() && peek() != '\n') advance();
        } else {
            break;
        }
    }
}

Token Lexer::readNumber() {
    std::string num;
    if (peek() == '-') num += advance();
    while (pos < src.size() && (isdigit(peek()) || peek() == '.')) {
        if (peek() == '.' && peek(1) == '.') break;
        num += advance();
    }
    return Token(TokenType::NUMBER, num, line);
}

Token Lexer::readString() {
    advance();
    std::string val;
    while (pos < src.size() && peek() != '"') {
        if (peek() == '\n')
            throw std::runtime_error("Unterminated string on line " + std::to_string(line));
        val += advance();
    }
    if (pos >= src.size())
        throw std::runtime_error("Unterminated string at EOF");
    advance();
    return Token(TokenType::STRING, val, line);
}

Token Lexer::readIdentOrKeyword() {
    std::string word;
    while (pos < src.size() && (isalnum(peek()) || peek() == '_')) {
        word += advance();
    }
    return Token(keywordOrIdent(word), word, line);
}

TokenType Lexer::keywordOrIdent(const std::string& word) {
    auto it = KEYWORDS.find(word);
    return it != KEYWORDS.end() ? it->second : TokenType::IDENT;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        skipWhitespaceAndComments();
        if (pos >= src.size()) {
            tokens.emplace_back(TokenType::END_OF_FILE, "", line);
            break;
        }
        char c = peek();
        if (c == '{') { advance(); tokens.emplace_back(TokenType::LBRACE,  "{", line); continue; }
        if (c == '}') { advance(); tokens.emplace_back(TokenType::RBRACE,  "}", line); continue; }
        if (c == '(') { advance(); tokens.emplace_back(TokenType::LPAREN,  "(", line); continue; }
        if (c == ')') { advance(); tokens.emplace_back(TokenType::RPAREN,  ")", line); continue; }
        if (c == ':') { advance(); tokens.emplace_back(TokenType::COLON,   ":", line); continue; }
        if (c == ',') { advance(); tokens.emplace_back(TokenType::COMMA,   ",", line); continue; }
        if (c == '.') {
            if (peek(1) == '.') { advance(); advance(); tokens.emplace_back(TokenType::DOTDOT, "..", line); }
            else                { advance();             tokens.emplace_back(TokenType::DOT,    ".",  line); }
            continue;
        }
        if (isdigit(c) || (c == '-' && isdigit(peek(1)))) { tokens.push_back(readNumber());         continue; }
        if (c == '"')                                       { tokens.push_back(readString());         continue; }
        if (isalpha(c) || c == '_')                         { tokens.push_back(readIdentOrKeyword()); continue; }
        tokens.emplace_back(TokenType::UNKNOWN, std::string(1, c), line);
        advance();
    }
    return tokens;
}
