#include "Node.hpp"

std::ostream& operator<<(std::ostream& out, const NodeKind& kind) {
    switch (kind) {
        case NodeKind::NOP: out << "NOP"; break;
        case NodeKind::ADD: out << "ADD"; break;
        case NodeKind::SUB: out << "SUB"; break;
        case NodeKind::MUL: out << "MUL"; break;
        case NodeKind::DIV: out << "DIV"; break;
        case NodeKind::EQ: out << "EQ"; break;
        case NodeKind::NEQ: out << "NEQ"; break;
        case NodeKind::LT: out << "LT"; break;
        case NodeKind::LEQ: out << "LEQ"; break;
        case NodeKind::NUM: out << "NUM"; break;
        case NodeKind::BLOCK: out << "BLOCK"; break;
        case NodeKind::ASSIGN: out << "ASSIGN"; break;
        case NodeKind::LVAR: out << "LVAR"; break;
        case NodeKind::RETURN: out << "RETURN"; break;
        case NodeKind::IF: out << "IF"; break;
        case NodeKind::WHILE: out << "WHILE"; break;
        case NodeKind::FOR: out << "FOR"; break;
        case NodeKind::CALL: out << "CALL"; break;
        case NodeKind::DEFINE: out << "DEFINE"; break;
        case NodeKind::ADDR: out << "ADDR"; break;
        case NodeKind::DEREF: out << "DEREF"; break;
        case NodeKind::LET: out << "LET"; break;
    }
    return out;
};
