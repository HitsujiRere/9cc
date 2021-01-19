#pragma once

#include <string>
#include <memory>

// ローカル変数
struct LVar {
    std::shared_ptr<LVar> next; // 次の変数
    std::string name;           // 変数名
    int offset;                 // RBPからのオフセット

    LVar() = default;

    LVar(std::shared_ptr<LVar> next, std::string name, int offset)
        : next(next), name(name), offset(offset) { }
};
