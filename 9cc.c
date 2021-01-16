#include "9cc.h"

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char* op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

int get_offset() {
    LVar *lvar = find_lvar(token);
    int offset;
    if (lvar) {
        offset = lvar->offset;
    } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = token->str;
        lvar->len = token->len;
        lvar->offset = locals->offset + 8;
        offset = lvar->offset;
        locals = lvar;
    }
    token = token->next;
    return offset;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startwith(char *p, char* q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

char *reserved_words[] = {
    "==",
    "!=",
    "<=",
    ">=",
};

char *reserved_words_alnum[] = {
    "return",
    "if",
    "else",
    "while",
};

int reserved_len(char *p)
{
    for (int i = 0; i < 4; i++) {
        if (!memcmp(p, reserved_words[i], strlen(reserved_words[i]))) {
            return strlen(reserved_words[i]);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (!memcmp(p, reserved_words_alnum[i], strlen(reserved_words_alnum[i]))
            && !is_alnum(p[strlen(reserved_words_alnum[i])])) {
            return strlen(reserved_words_alnum[i]);
        }
    }
    return 0;
}

// user_inputをトークナイズしてそれを返す
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        int reslen = reserved_len(p);
        if (reslen != 0) {
            cur = new_token(TK_RESERVED, cur, p, reslen);
            p += reslen;
            continue;
        }

        if (strchr("+-*/()<>=;", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z') {
            int i = 1;
            while(is_alnum(*(p + i)))
            {
                i++;
            }
            cur = new_token(TK_IDENT, cur, p, i);
            p += i;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(int offset) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = offset;
    return node;
}

void program() {
    locals = calloc(1, sizeof(LVar));
    locals->offset = 0;

    int i = 0;
    while(!at_eof()) {
        // fprintf(stderr, "program() token->kind = %d\n", token->kind);
        code[i++] = stmt();
    }
    code[i] = NULL;
}

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *stmt() {
    Node *node;

    if (consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
    } else if (consume("if")) {
        consume("(");

        Node *condition = expr();

        consume(")");

        node = stmt();

        if (consume("else")) {
            node = new_node(ND_ELSE, node, stmt());
        }

        node = new_node(ND_IF, condition, node);
    } else if (consume("while")) {
        consume("(");

        Node *condition = expr();

        consume(")");

        node = new_node(ND_WHILE, condition, stmt());
    } else {
        node = expr();
        expect(";");
    }

    return node;
}

// expr = assign
Node *expr() {
    return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
    // fprintf(stderr, "add()1 token->kind = %d\n", token->kind);
    Node *node = mul();

    for (;;) {
        // fprintf(stderr, "add()2 token->kind = %d\n", token->kind);
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
    // fprintf(stderr, "mul()1 token->kind = %d\n", token->kind);
    Node *node = unary();

    for (;;) {
        // fprintf(stderr, "mul()2 token->kind = %d\n", token->kind);
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? primary
Node* unary() {
    // fprintf(stderr, "unary()1 token->kind = %d\n", token->kind);
    if (consume("+"))
        return primary();
    // fprintf(stderr, "unary()2 token->kind = %d\n", token->kind);
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    // fprintf(stderr, "unary()3 token->kind = %d\n", token->kind);
    return primary();
}

// primary = num | ident | "(" expr ")"
Node *primary() {
    // fprintf(stderr, "primary()1 token->kind = %d\n", token->kind);

    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // fprintf(stderr, "primary()2 token->kind = %d\n", token->kind);

    if (token->kind == TK_IDENT) {
        return new_node_ident(get_offset());
    }

    // fprintf(stderr, "primary()3 token->kind = %d\n", token->kind);

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        error_at(token->str, "代入の左辺値が変数ではありません");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen(Node *node) {
    int LIfEndNow = LIfEnd;
    int LElseNow = LElse;
    int LWhileBeginNow = LWhileBegin;
    int LWhileEndNow = LWhileEnd;

    switch (node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_IF:
        LIfEnd++;
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->rhs->kind == ND_ELSE) {
            gen(node->rhs);
        } else {
            printf("  je .LIfEnd%d\n", LIfEndNow);
            gen(node->rhs);
        }
        printf(".LIfEnd%d:\n", LIfEndNow);
        return;
    case ND_ELSE:
        LElse++;
        printf("  je .LElse%d\n", LElseNow);
        gen(node->lhs);
        printf("  jmp .LIfEnd%d\n", LIfEndNow - 1);
        printf(".LElse%d:\n", LElseNow);
        gen(node->rhs);
        return;
    case ND_WHILE:
        LWhileBegin++;
        LWhileEnd++;
        printf(".LWhileBegin%d:\n", LWhileBeginNow);
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .LWhileEnd%d\n", LWhileEndNow);
        gen(node->rhs);
        printf("  jmp .LWhileBegin%d\n", LWhileBeginNow);
        printf(".LWhileEnd%d:\n", LWhileEndNow);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    }

    printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    user_input = argv[1];
    token = tokenize();
    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 先頭の式から順にコード生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // 式の評価結果としてスタックに一つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");

    return 0;
}
