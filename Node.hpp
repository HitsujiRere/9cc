#pragma once

#include <memory>
#include <vector>
#include <ostream>

// 抽象構文木のノードの種類
enum class NodeKind {
    NOP,    // 何もしない
    ADD,    // +
    SUB,    // -
    MUL,    // *
    DIV,    // /
    EQ,     // ==
    NEQ,    // !=
    LT,     // <
    LEQ,    // <=
    NUM,    // 整数
    BLOCK,  // { ... }
    ASSIGN, // =
    LVAR,   // ローカル変数
    RETURN, // return
    IF,     // if
    WHILE,  // while
    FOR,    // for
    CALL,   // func(...)
    DEFINE, // func(...) ...
    ADDR,   // &
    DEREF,  // *var
    LET,    // int var
};

std::ostream& operator<<(std::ostream& out, const NodeKind& kind);

// 抽象構文木のノードの型
struct Node {
    NodeKind kind;             // ノードの型
    std::shared_ptr<Node> lhs; // 左辺
    std::shared_ptr<Node> rhs; // 右辺
    int val;                   // kindがND_NUMの場合，数値
    std::vector<std::shared_ptr<Node>> args; // 子
    std::string str;           // 名前

    Node(NodeKind kind)
        : kind(kind) { }

    Node(NodeKind kind,
        std::shared_ptr<Node> lhs,
        std::shared_ptr<Node> rhs)
        : kind(kind), lhs(lhs), rhs(rhs) { }

    Node(NodeKind kind, int val)
        : kind(kind), val(val) { }

    Node(NodeKind kind, std::string str)
        : kind(kind), str(str) { }
};
