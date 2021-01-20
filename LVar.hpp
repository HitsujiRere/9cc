#pragma once

#include <string>
#include <memory>

#include "Type.hpp"

// ローカル変数
struct LVar {
    std::shared_ptr<LVar> next; // 次の変数
    std::string name;           // 変数名
    int offset;                 // RBPからのオフセット
    std::shared_ptr<Type> type; // 型

    LVar() = default;

    LVar(std::shared_ptr<LVar> next, std::string name, int offset, std::shared_ptr<Type> type)
        : next(next), name(name), offset(offset), type(type) { }
};
