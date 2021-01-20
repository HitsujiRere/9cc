#pragma once

enum class TypeKind { 
    INT, PTR
};

struct Type {
    TypeKind ty;
    std::shared_ptr<Type> ptr_to;

    Type(TypeKind ty)
        : ty(ty) { }

    Type(TypeKind ty, std::shared_ptr<Type> ptr_to)
        : ty(ty), ptr_to(ptr_to) { }
};
