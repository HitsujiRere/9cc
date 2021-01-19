#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "Token.hpp"
#include "Node.hpp"
#include "LVar.hpp"

bool debug_mode = false;

// コード
std::string code;
// コードの見ている場所
std::string::iterator code_itr;

void _error_at() { }

template <class T, class... Args>
void _error_at(const T& value, const Args&... args) {
    std::cerr << value;
    _error_at(args...);
}

// エラー箇所を報告する
template <class... Args>
void error_at(size_t pos, const Args&... args) {
    std::cerr << code << std::endl;
    std::cerr << std::string(pos, ' ');
    std::cerr << "^ ";
    _error_at(args...);
    std::cerr << std::endl;
    exit(1);
}

// 現在着目しているトークン
std::shared_ptr<Token> token;

// program    = define*
// define     = ident ("(" ident ("," ident)* ")")? stmt
// stmt       = expr ";"
//            | "{" stmt* "}"
//            | "return" expr ";"
//            | "if" "(" expr ")" stmt ("else" stmt)?
//            | "while" "(" expr ")" stmt
//            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = primary
//            | "+" primary
//            | "-" primary
//            | "*" primary
//            | "&" primary
// primary    = num
//            | ident ("(" expr ("," expr)* ")")?
//            | "(" expr ")"

std::shared_ptr<Node> program();
std::shared_ptr<Node> define();
std::shared_ptr<Node> stmt();
std::shared_ptr<Node> expr();
std::shared_ptr<Node> assign();
std::shared_ptr<Node> equality();
std::shared_ptr<Node> relational();
std::shared_ptr<Node> add();
std::shared_ptr<Node> mul();
std::shared_ptr<Node> unary();
std::shared_ptr<Node> primary();

// ローカル変数
std::shared_ptr<LVar> locals;

int LIf = 0;
int LWhile = 0;
int LFor = 0;

std::vector<std::string> func_args = {
    "edi",
    "esi",
    "edx",
    "ecx",
    "r8d",
    "r9d",
};
