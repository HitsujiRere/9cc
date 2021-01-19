#include "9cc.hpp"

#define debug(val) if (debug_mode) { std::cerr << #val << " : " << val << std::endl; }

size_t search_int(const std::string::iterator& itr) {
    size_t len = 0;
    for (; '0' <= *(itr + len) && *(itr + len) <= '9'; len++) {
    }
    return len;
}

int parseToInt(const std::string::iterator& itr) {
    int num = 0;
    for (size_t i = 0; '0' <= *(itr + i) && *(itr + i) <= '9'; i++) {
        num *= 10;
        num += *(itr + i) - '0';
    }
    return num;
}

bool is_ident_char(char ch) {
    return ('a' <= ch && ch <= 'z')
        || ('A' <= ch && ch <= 'Z')
        || ('0' <= ch && ch <= '9')
        || (ch == '_');
}

bool is_ident_head_char(char ch) {
    return ('a' <= ch && ch <= 'z')
        || ('A' <= ch && ch <= 'Z')
        || (ch == '_');
}

size_t search_ident(const std::string::iterator& itr) {
    size_t len = 0;
    for (; is_ident_char(*(itr + len)); len++) {
    }
    return len;
}

std::string substr(const std::string::iterator& itr, size_t len) {
    std::string text;
    for (size_t i = 0; i < len; i++) {
        text += *(itr + i);
        // itr++;
    }
    return text;
}

size_t search_reserves(const std::string::iterator& itr) {
    for (const auto& reserve : reserves) {
        if (reserve == substr(itr, reserve.size())) {
            return reserve.size();
        }
    }
    for (const auto& reserve : reserves_ident) {
        if (reserve == substr(itr, reserve.size()) && !is_ident_char(*(itr + reserve.size()))) {
            return reserve.size();
        }
    }
    return 0;
}

std::shared_ptr<LVar> find_lvar(const std::string& name) {
    for (auto var = locals; var != nullptr; var = var->next) {
        if (var->name == name) {
            return var;
        }
    }
    return nullptr;
}

int make_lvar(const std::string& name) {
    auto lvar = std::shared_ptr<LVar>(new LVar(locals, name, locals->offset + 8));
    locals = lvar;
    return lvar->offset;
}

int get_offset(const std::string& name) {
    auto lvar = find_lvar(name);

    if (lvar == nullptr) {
        error_at(token->pos, "存在しない変数です");
    }

    return lvar->offset;
}

std::shared_ptr<Token> tokenize() {
    std::shared_ptr<Token> head = std::shared_ptr<Token>(new Token());
    std::shared_ptr<Token> cur = head;

    while (code_itr != code.end()) {
        // debug((code_itr - code.begin()));

        if (isspace(*code_itr)) {
            code_itr++;
            continue;
        }

        size_t len = search_reserves(code_itr);
        if (len > 0) {
            cur = Token::makeNext(TokenKind::RESERVED, cur, code_itr - code.begin());
            cur->str = substr(code_itr, len);
            code_itr += len;
            continue;
        }

        if (is_ident_head_char(*code_itr)) {
            size_t len = search_ident(code_itr);
            cur = Token::makeNext(TokenKind::IDENT, cur, code_itr - code.begin());
            cur->str = substr(code_itr, len);
            code_itr += len;
            continue;
        }

        if ('0' <= *code_itr && *code_itr <= '9') {
            size_t len = search_int(code_itr);
            cur = Token::makeNext(TokenKind::NUM, cur, code_itr - code.begin());
            cur->val = parseToInt(code_itr);
            code_itr += len;
            continue;
        }

        error_at(code_itr - code.begin(), "トークナイズできません");
    }

    Token::makeNext(TokenKind::END, cur, code_itr - code.begin());

    return head->next;
}

std::shared_ptr<Node> program() {
    debug("program0");
    std::shared_ptr<Node> head = std::shared_ptr<Node>(new Node(NodeKind::BLOCK));

    while(token->kind != TokenKind::END) {
        head->args.push_back(define());
    }

    return head;
}

std::shared_ptr<Node> define() {
    debug("define0");

    std::shared_ptr<Node> node;

    if(!token->reserved("int"))
        error_at(token->pos, "'int'ではありません");
    token = token->next;

    if(token->kind != TokenKind::IDENT)
        error_at(token->pos, "関数名ではありません");
    node = std::shared_ptr<Node>(new Node(NodeKind::DEFINE, token->str));
    token = token->next;

    if(!token->reserved("("))
        error_at(token->pos, "'('ではありません");
    token = token->next;

    if(!token->reserved(")")) {
        if(!token->reserved("int"))
            error_at(token->pos, "'int'ではありません");
        token = token->next;

        if(token->kind != TokenKind::IDENT)
            error_at(token->pos, "変数名ではありません");
        node->args.push_back(
            std::shared_ptr<Node>(new Node(NodeKind::LVAR, make_lvar(token->str)))
        );
        token = token->next;

        while(!token->reserved(")")) {
            if(!token->reserved(","))
                error_at(token->pos, "','ではありません");
            token = token->next;

            if(!token->reserved("int"))
                error_at(token->pos, "'int'ではありません");
            token = token->next;

            if(token->kind != TokenKind::IDENT)
                error_at(token->pos, "変数名ではありません");
            node->args.push_back(
                std::shared_ptr<Node>(new Node(NodeKind::LVAR, make_lvar(token->str)))
            );
            token = token->next;
        }
    }
    token = token->next;

    node->args.push_back(stmt());

    return node;
}

std::shared_ptr<Node> stmt() {
    debug("stmt0");
    std::shared_ptr<Node> node;

    debug(token->str);

    if (token->reserved("return")) {
        token = token->next;
        node = std::shared_ptr<Node>(new Node(NodeKind::RETURN, expr(), nullptr));

        if (!token->reserved(";"))
            error_at(token->pos, "';'がありません");
        token = token->next;
    } else if (token->reserved("{")) {
        token = token->next;

        node = std::shared_ptr<Node>(new Node(NodeKind::BLOCK));

        while(!token->reserved("}")) {
            node->args.push_back(stmt());
        }

        token = token->next;
    } else if (token->reserved("if")) {
        token = token->next;

        node = std::shared_ptr<Node>(new Node(NodeKind::IF));

        if (!token->reserved("("))
            error_at(token->pos, "'('がありません");
        token = token->next;

        node->args.push_back(expr());

        if (!token->reserved(")"))
            error_at(token->pos, "')'がありません");
        token = token->next;

        node->args.push_back(stmt());

        if (token->reserved("else")) {
            token = token->next;
            node->args.push_back(stmt());
        }
    } else if (token->reserved("while")) {
        token = token->next;

        node = std::shared_ptr<Node>(new Node(NodeKind::WHILE));

        if (!token->reserved("("))
            error_at(token->pos, "'('がありません");
        token = token->next;

        node->args.push_back(expr());

        if (!token->reserved(")"))
            error_at(token->pos, "')'がありません");
        token = token->next;

        node->args.push_back(stmt());
    } else if (token->reserved("for")) {
        token = token->next;

        node = std::shared_ptr<Node>(new Node(NodeKind::FOR));

        if (!token->reserved("("))
            error_at(token->pos, "'('がありません");
        token = token->next;

        node->args.push_back(
            !token->reserved(";")
            ? expr()
            : std::shared_ptr<Node>(new Node(NodeKind::NOP))
        );

        if (!token->reserved(";"))
            error_at(token->pos, "';'がありません");
        token = token->next;

        node->args.push_back(
            !token->reserved(";")
            ? expr()
            : std::shared_ptr<Node>(new Node(NodeKind::NOP))
        );

        if (!token->reserved(";"))
            error_at(token->pos, "';'がありません");
        token = token->next;

        node->args.push_back(
            !token->reserved(")")
            ? expr()
            : std::shared_ptr<Node>(new Node(NodeKind::NOP))
        );

        if (!token->reserved(")"))
            error_at(token->pos, "')'がありません");
        token = token->next;

        node->args.push_back(stmt());
    } else if (token->reserved("int")) {
        token = token->next;

        if(token->kind != TokenKind::IDENT)
            error_at(token->pos, "変数名ではありません");
        node = std::shared_ptr<Node>(new Node(NodeKind::LVAR, make_lvar(token->str)));
        token = token->next;

        if (!token->reserved(";"))
            error_at(token->pos, "';'がありません");
        token = token->next;
    } else {
        node = expr();

        if (!token->reserved(";"))
            error_at(token->pos, "';'がありません");
        token = token->next;
    }

    return node;
}

std::shared_ptr<Node> expr() {
    debug("expr0");
    return assign();
}

std::shared_ptr<Node> assign() {
    debug("assign0");
    std::shared_ptr<Node> node = equality();
    if (token->reserved("=")) {
        token = token->next;
        node = std::shared_ptr<Node>(new Node(NodeKind::ASSIGN, node, assign()));
    }
    return node;
}

std::shared_ptr<Node> equality() {
    debug("equality()0");
    std::shared_ptr<Node> node = relational();

    while(true) {
        if (token->reserved("==")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::EQ, node, relational()));
        } else if (token->reserved("!=")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::NEQ, node, relational()));
        } else {
            break;
        }
    }

    return node;
}

std::shared_ptr<Node> relational() {
    debug("relational()0");
    std::shared_ptr<Node> node = add();

    while(true) {
        if (token->reserved("<=")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::LEQ, node, add()));
        } else if (token->reserved("<")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::LT, node, add()));
        } else if (token->reserved(">=")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::LEQ, add(), node));
        } else if (token->reserved(">")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::LT, add(), node));
        } else {
            break;
        }
    }

    return node;
}

std::shared_ptr<Node> add() {
    debug("add()0");
    std::shared_ptr<Node> node = mul();

    while(true) {
        if (token->reserved("+")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::ADD, node, mul()));
        } else if (token->reserved("-")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::SUB, node, mul()));
        } else {
            break;
        }
    }

    return node;
}

std::shared_ptr<Node> mul() {
    debug("mul()0");
    std::shared_ptr<Node> node = unary();

    while(true) {
        debug("mul()1");

        if (token->reserved("*")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::MUL, node, unary()));
        } else if (token->reserved("/")) {
            token = token->next;
            node = std::shared_ptr<Node>(new Node(NodeKind::DIV, node, unary()));
        } else {
            break;
        }
    }

    debug("mul()2");

    return node;
}

std::shared_ptr<Node> unary() {
    if (token->reserved("+")) {
        token = token->next;
        return primary();
    }

    if (token->reserved("-")) {
        token = token->next;
        return std::shared_ptr<Node>(new Node(
            NodeKind::SUB,
            std::shared_ptr<Node>(new Node(NodeKind::NUM, 0)),
            primary()
        ));
    }

    if (token->reserved("*")) {
        token = token->next;
        return std::shared_ptr<Node>(new Node(NodeKind::DEREF, primary(), nullptr));
    }

    if (token->reserved("&")) {
        token = token->next;
        return std::shared_ptr<Node>(new Node(NodeKind::ADDR, primary(), nullptr));
    }

    return primary();
}

std::shared_ptr<Node> primary() {
    debug("primary()0");
    std::shared_ptr<Node> node;

    if (token->reserved("(")) {
        token = token->next;
        node = expr();
        // token = token->next;
        if (!token->reserved(")"))
            error_at(token->pos, "')'ではありません");
        token = token->next;
    } else if (token->kind == TokenKind::IDENT) {
        if (token->next->reserved("(")) {
            node = std::shared_ptr<Node>(new Node(NodeKind::CALL, token->str));
            token = token->next;

            token = token->next;

            if(!token->reserved(")")) {
                node->args.push_back(expr());

                while(!token->reserved(")")) {
                    if(!token->reserved(","))
                        error_at(token->pos, "','ではありません");
                    token = token->next;

                    node->args.push_back(expr());
                }
            }
            token = token->next;
        } else {
            node = std::shared_ptr<Node>(new Node(NodeKind::LVAR, get_offset(token->str)));
            token = token->next;
        }
    } else {
        if (token->kind != TokenKind::NUM)
            error_at(token->pos, "数字ではありません");
        node = std::shared_ptr<Node>(new Node(NodeKind::NUM,token->val));
        token = token->next;
    }

    return node;
}

void gen_lval(std::shared_ptr<Node> node) {
    if (node->kind != NodeKind::LVAR)
        error_at(0, "代入の左辺値が変数ではありません");

    std::cout << "  mov rax, rbp" << std::endl;
    std::cout << "  sub rax, " << node->val << std::endl;
    std::cout << "  push rax" << std::endl;
}

void gen(std::shared_ptr<Node> node) {
    debug("gen()0");
    debug((int)node->kind);

    int LIfNow = LIf;
    int LWhileNow = LWhile;
    int LForNow = LFor;

    switch(node->kind) {
    case NodeKind::NOP:
        break;
    case NodeKind::NUM:
        std::cout << "  push " << node->val << std::endl;
        break;
    case NodeKind::BLOCK:
        for (auto arg : node->args) {
            gen(arg);
            if (arg->kind != NodeKind::BLOCK)
                std::cout << "  pop rax" << std::endl;
        }
        break;
    case NodeKind::LVAR:
        gen_lval(node);
        std::cout << "  pop rax" << std::endl;
        std::cout << "  mov rax, [rax]" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  mov [rax], rdi" << std::endl;
        std::cout << "  push rdi" << std::endl;
        break;
    case NodeKind::RETURN:
        gen(node->lhs);
        std::cout << "  pop rax" << std::endl;
        std::cout << "  mov rsp, rbp" << std::endl;
        std::cout << "  pop rbp" << std::endl;
        std::cout << "  ret" << std::endl;
        break;
    case NodeKind::IF:
        LIf++;
        gen(node->args.at(0));
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cmp rax, 0" << std::endl;
        if (node->args.size() == 2) {
            std::cout << "  je .LIfEnd" << LIfNow << std::endl;
            gen(node->args.at(1));
            std::cout << ".LIfEnd" << LIfNow << ":" << std::endl;
        } else {
            std::cout << "  je .LIfElse" << LIfNow << std::endl;
            gen(node->args.at(1));
            std::cout << "  jmp .LIfEnd" << LIfNow << std::endl;
            std::cout << ".LIfElse" << LIfNow << ":" << std::endl;
            gen(node->args.at(2));
            std::cout << ".LIfEnd" << LIfNow << ":" << std::endl;
        }
        break;
    case NodeKind::WHILE:
        LWhile++;
        std::cout << ".LWhileBegin" << LWhileNow << ":" << std::endl;
        gen(node->args.at(0));
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cmp rax, 0" << std::endl;
        std::cout << "  je .LWhileEnd" << LWhileNow << std::endl;
        gen(node->args.at(1));
        std::cout << "  jmp .LWhileBegin" << LWhileNow << std::endl;
        std::cout << ".LWhileEnd" << LWhileNow << ":" << std::endl;
        break;
    case NodeKind::FOR:
        LFor++;
        gen(node->args.at(0));
        std::cout << ".LForBegin" << LForNow << ":" << std::endl;
        gen(node->args.at(1));
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cmp rax, 0" << std::endl;
        std::cout << "  je .LForEnd" << LForNow << std::endl;
        gen(node->args.at(3));
        gen(node->args.at(2));
        std::cout << "  jmp .LForBegin" << LForNow << std::endl;
        std::cout << ".LForEnd" << LForNow << ":" << std::endl;
        break;
    case NodeKind::CALL:
        for (size_t i = 0; i < node->args.size(); i++) {
            auto arg = node->args.at(i);
            gen(arg);
            if (arg->kind != NodeKind::BLOCK)
                std::cout << "  pop rax" << std::endl;
            std::cout << "  mov " << func_args.at(i) <<", eax" << std::endl;
        }
        std::cout << "  call " << node->str << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::DEFINE:
        std::cout << node->str << ":" << std::endl;

        // 変数26個分の領域を確保する
        std::cout << "  push rbp" << std::endl;
        std::cout << "  mov rbp, rsp" << std::endl;
        std::cout << "  sub rsp, 208" << std::endl;

        for (size_t i = 0; i + 1 < node->args.size(); i++) {
            gen_lval(node->args.at(i));

            std::cout << "  mov edi, " << func_args.at(i) << std::endl;

            // std::cout << "  pop rdi" << std::endl;
            std::cout << "  pop rax" << std::endl;
            std::cout << "  mov [rax], rdi" << std::endl;
            std::cout << "  push rdi" << std::endl;
        }

        gen(node->args.back());

        std::cout << "  mov rsp, rbp" << std::endl;
        std::cout << "  pop rbp" << std::endl;

        std::cout << "  ret" << std::endl;
        break;
    case NodeKind::ADDR:
        gen_lval(node->lhs);
        break;
    case NodeKind::DEREF:
        gen(node->lhs);
        std::cout << "  pop rax" << std::endl;
        std::cout << "  mov rax, [rax]" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::ADD:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  add rax, rdi" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::SUB:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  sub rax, rdi" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::MUL:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  imul rax, rdi" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::DIV:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cqo" << std::endl;
        std::cout << "  idiv rdi" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::EQ:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cmp rax, rdi" << std::endl;
        std::cout << "  sete al" << std::endl;
        std::cout << "  movzb rax, al" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::NEQ:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cmp rax, rdi" << std::endl;
        std::cout << "  setne al" << std::endl;
        std::cout << "  movzb rax, al" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::LT:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cmp rax, rdi" << std::endl;
        std::cout << "  setl al" << std::endl;
        std::cout << "  movzb rax, al" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    case NodeKind::LEQ:
        gen(node->lhs);
        gen(node->rhs);
        std::cout << "  pop rdi" << std::endl;
        std::cout << "  pop rax" << std::endl;
        std::cout << "  cmp rax, rdi" << std::endl;
        std::cout << "  setle al" << std::endl;
        std::cout << "  movzb rax, al" << std::endl;
        std::cout << "  push rax" << std::endl;
        break;
    }
}

int main(int argc, char **argv) {
    if (2 > argc && argc > 3) {
        debug("引数の個数が正しくありません");
        return 1;
    }

    if (argc == 3) {
        if (std::string(argv[2]) == "-D") {
            debug_mode = true;
        }
    }

    code = std::string(argv[1]);
    code_itr = code.begin();

    locals = std::shared_ptr<LVar>(new LVar());

    token = tokenize();
    auto node = program();

    std::cout << ".intel_syntax noprefix" << std::endl;
    std::cout << ".globl main" << std::endl;
    gen(node);

    return 0;
}
