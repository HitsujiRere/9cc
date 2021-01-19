#pragma once

#include <string>
#include <vector>
#include <memory>

// トークンの種類
enum class TokenKind {
    RESERVED, // 記号
    IDENT,    // 識別子
    NUM,      // 整数トークン
    END,      // 入力の終わりを表すトークン
};

// 記号
const std::vector<std::string> reserves = {
    "==", "!=", "<=", ">=",
    "(", ")", "{", "}", "<", ">",
    ";", ",", "=", "+", "-", "*", "/", "&",
};

// 識別子のような記号
const std::vector<std::string> reserves_ident = {
    "return",
    "if",
    "else",
    "while",
    "for",
};

// トークン型
struct Token {
    TokenKind kind;              // トークンの型
    std::shared_ptr<Token> next; // 次の入力トークン
    int val;                     // kind==NUM : 数値
    std::string str;             // トークン文字列
    size_t pos;                  // 位置

    Token() = default;

    Token(TokenKind kind, size_t pos)
        : kind(kind), pos(pos) { }

    // from->next = nextとなるTokenを作成する
    static std::shared_ptr<Token> makeNext(TokenKind kind, std::shared_ptr<Token> from, size_t pos) {
        std::shared_ptr<Token> next(new Token(kind, pos));
        from->next = next;
        return next;
    }

    // 指定した記号かどうか
    bool reserved(const std::string& op) const {
        return kind == TokenKind::RESERVED && str == op;
    }
};
