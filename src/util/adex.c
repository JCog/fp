#include "adex.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector/vector.h>

const char *adexErrorName[] = {
    "success", "out of memory", "syntax error", "arithmetic error", "address unavailable",
};

#define ID_MAX 0x20

enum TokType {
    TOK_CONST,
    TOK_ID,
    TOK_OP,
    TOK_BRACK_L,
    TOK_BRACK_R,
};

enum Op {
    OP_B,
    OP_BZ,
    OP_H,
    OP_HZ,
    OP_W,
    OP_MUL,
    OP_DIV,
    OP_REM,
    OP_ADD,
    OP_SUB,
    OP_MAX,
};

enum Brack {
    BRACK_PAREN,
    BRACK_SQUARE,
};

struct Tok {
    enum TokType type;
    union {
        u32 value;
        char id[ID_MAX];
        enum Op op;
        enum Brack brack;
    };
};

static const char *opS[] = {"b.", "bz.", "h.", "hz.", "w.", "*", "/", "%", "+", "-"};

static s32 opPrec[] = {
    /* modes */
    0,
    0,
    0,
    0,
    0,
    /* mul, div, rem */
    1,
    1,
    1,
    /* add, sub */
    2,
    2,
};

static s32 opAr[] = {
    /* modes */
    1,
    1,
    1,
    1,
    1,
    /* mul, div, rem */
    2,
    2,
    2,
    /* add, sub */
    2,
    2,
};

struct Sym {
    const char *id;
    void *addr;
};

static bool makeTok(struct Tok *tok, enum TokType type, ...) {
    tok->type = type;
    va_list arg;
    va_start(arg, type);
    if (type == TOK_CONST) {
        tok->value = va_arg(arg, u32);
    } else if (type == TOK_ID) {
        strcpy(tok->id, va_arg(arg, char *));
    } else if (type == TOK_OP) {
        tok->op = va_arg(arg, enum Op);
    } else if (type == TOK_BRACK_L || type == TOK_BRACK_R) {
        tok->brack = va_arg(arg, enum Brack);
    }
    va_end(arg);
    return tok;
}

static bool isFuncTok(struct Tok *tok) {
    return tok->type == TOK_OP && opAr[tok->op] == 1;
}

static bool parseOp(const char **p, s32 *op) {
    for (s32 i = 0; i < OP_MAX; ++i) {
        s32 l = strlen(opS[i]);
        if (strncmp(*p, opS[i], l) == 0) {
            *p += l;
            *op = i;
            return 1;
        }
    }
    return 0;
}

static bool isWordChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static char *parseWord(const char **p) {
    const char *e = *p;
    while (isWordChar(*e)) {
        ++e;
    }
    s32 l = e - *p;
    char *s = malloc(l + 1);
    if (!s) {
        return NULL;
    }
    memcpy(s, *p, l);
    s[l] = 0;
    *p = e;
    return s;
}

static void stackInit(struct vector *stack) {
    vector_init(stack, sizeof(struct Tok));
}

static struct Tok *stackPeek(struct vector *stack) {
    if (stack->size == 0) {
        return NULL;
    } else {
        return vector_at(stack, stack->size - 1);
    }
}

static bool stackPush(struct vector *stack, struct Tok *tok) {
    return vector_push_back(stack, 1, tok);
}

static void stackPop(struct vector *stack, struct Tok *tok) {
    *tok = *stackPeek(stack);
    vector_erase(stack, stack->size - 1, 1);
}

enum AdexError adexParse(struct Adex *adex, const char *str) {
    enum AdexError e;
    /* initialize adex */
    stackInit(&adex->expr);
    /* initialize stack */
    struct vector stack;
    stackInit(&stack);
    /* parse string */
    bool modeOp = 0;
    const char *p = str;
    while (*p) {
        char c = *p;
        struct Tok tok;
        s32 op;
        /* parse whitespace */
        if (c == ' ' || c == '\t') {
            ++p;
            continue;
        }
        /* parse comment and end-of-line */
        else if (c == '#' || c == '\r' || c == '\n') {
            break;
            /* parse operator */
        } else if (parseOp(&p, &op)) {
            makeTok(&tok, TOK_OP, op);
            /* parse constant */
        } else if (c >= '0' && c <= '9') {
            char *word = parseWord(&p);
            if (!word) {
                goto mem_err;
            }
            char *wordS = word;
            char *wordE = wordS + strlen(wordS);
            s32 base;
            if (strncmp(word, "0x", 2) == 0 || strncmp(word, "0X", 2) == 0) {
                base = 16;
                wordS += 2;
            } else if (strncmp(word, "0b", 2) == 0 || strncmp(word, "0B", 2) == 0) {
                base = 2;
                wordS += 2;
            } else if (strncmp(word, "0", 1) == 0 && wordE - wordS > 1) {
                base = 8;
                wordS += 1;
            } else {
                base = 10;
            }
            if (wordS == wordE) {
                free(word);
                goto syntax_err;
            }
            u32 v = 0;
            u32 m = 1;
            for (char *wordP = wordE - 1; wordP >= wordS; --wordP) {
                char c = *wordP;
                s32 d = -1;
                if (c >= '0' && c <= '9') {
                    d = c - '0';
                } else if (c >= 'A' && c <= 'F') {
                    d = 10 + c - 'A';
                } else if (c >= 'a' && c <= 'f') {
                    d = 10 + c - 'a';
                }
                if (d < 0 || d >= base) {
                    free(word);
                    goto syntax_err;
                }
                v += d * m;
                m *= base;
            }
            free(word);
            makeTok(&tok, TOK_CONST, v);
        }
        /* parse identifier */
        else if (isWordChar(c)) {
            char *word = parseWord(&p);
            if (!word) {
                goto mem_err;
            }
            if (strlen(word) + 1 > ID_MAX) {
                free(word);
                goto syntax_err;
            }
            makeTok(&tok, TOK_ID, word);
            free(word);
        }
        /* parse bracket */
        else if (c == '[') {
            if (modeOp) {
                modeOp = FALSE;
                makeTok(&tok, TOK_BRACK_L, BRACK_SQUARE);
                ++p;
            } else {
                makeTok(&tok, TOK_OP, OP_W);
            }
        } else {
            if (c == '(') {
                makeTok(&tok, TOK_BRACK_L, BRACK_PAREN);
            } else if (c == ']') {
                makeTok(&tok, TOK_BRACK_R, BRACK_SQUARE);
            } else if (c == ')') {
                makeTok(&tok, TOK_BRACK_R, BRACK_PAREN);
            } else {
                goto syntax_err;
            }
            ++p;
        }
        /* process token */
        if (modeOp) {
            goto syntax_err;
        }
        if (tok.type == TOK_ID) {
            // TODO: figure out what this is even doing
            //      bool found = FALSE;
            //      for (s32 i = 0; i < sizeof(syms) / sizeof(*syms); ++i)
            //        if (strcmp(tok.id, syms[i].id) == 0) {
            //          make_tok(&tok, TOK_CONST, (u32)syms[i].addr);
            //          found = TRUE;
            //          break;
            //        }
            //      if (!found)
            //        goto syntax_err;
        }
        if (tok.type == TOK_CONST) {
            if (!stackPush(&adex->expr, &tok)) {
                goto mem_err;
            }
        } else if (tok.type == TOK_BRACK_L || isFuncTok(&tok)) {
            if (!stackPush(&stack, &tok)) {
                goto mem_err;
            }
            if (tok.type == TOK_OP && tok.op >= OP_B && tok.op <= OP_W) {
                modeOp = TRUE;
            }
        } else if (tok.type == TOK_BRACK_R) {
            while (1) {
                if (!stackPeek(&stack)) {
                    goto syntax_err;
                }
                struct Tok pop;
                stackPop(&stack, &pop);
                if (pop.type == TOK_BRACK_L && pop.brack == tok.brack) {
                    break;
                }
                if (!stackPush(&adex->expr, &pop)) {
                    goto mem_err;
                }
            }
            struct Tok *top = stackPeek(&stack);
            if (top && isFuncTok(top)) {
                struct Tok pop;
                stackPop(&stack, &pop);
                if (!stackPush(&adex->expr, &pop)) {
                    goto mem_err;
                }
            }
        } else if (tok.type == TOK_OP) {
            while (1) {
                struct Tok *top = stackPeek(&stack);
                if (!top || top->type != TOK_OP || opPrec[top->op] < opPrec[tok.op]) {
                    break;
                }
                struct Tok pop;
                stackPop(&stack, &pop);
                if (!stackPush(&adex->expr, &pop)) {
                    goto mem_err;
                }
            }
            if (!stackPush(&stack, &tok)) {
                goto mem_err;
            }
        }
    }
    /* push remaining ops */
    while (stackPeek(&stack)) {
        struct Tok pop;
        stackPop(&stack, &pop);
        if (pop.type != TOK_OP) {
            goto syntax_err;
        }
        if (!stackPush(&adex->expr, &pop)) {
            goto mem_err;
        }
    }
    /* check syntax */
    s32 nOps = 0;
    for (s32 i = 0; i < adex->expr.size; ++i) {
        struct Tok *tok = vector_at(&adex->expr, i);
        if (tok->type == TOK_CONST || tok->type == TOK_ID) {
            ++nOps;
        } else if (tok->type == TOK_OP) {
            s32 ar = opAr[tok->op];
            if (nOps < ar) {
                goto syntax_err;
            }
            nOps -= ar;
            nOps += 1;
        }
    }
    if (nOps != 1) {
        goto syntax_err;
    }
    e = ADEX_ERROR_SUCCESS;
    goto exit;
mem_err:
    e = ADEX_ERROR_MEMORY;
    goto err;
syntax_err:
    e = ADEX_ERROR_SYNTAX;
    goto err;
err:
    adexDestroy(adex);
    goto exit;
exit:
    vector_destroy(&stack);
    return e;
}

static bool validateAddr(u32 addr, s32 size) {
    return addr >= 0x80000000 && addr < 0x80800000 && addr % size == 0;
}

enum AdexError adexEval(struct Adex *adex, u32 *result) {
    enum AdexError e;
    /* initialize stack */
    struct vector stack;
    vector_init(&stack, sizeof(u32));
    /* evaluate sub-expressions */
    for (s32 i = 0; i < adex->expr.size; ++i) {
        struct Tok *tok = vector_at(&adex->expr, i);
        u32 value;
        if (tok->type == TOK_CONST) {
            /* push operand */
            value = tok->value;
        } else if (tok->type == TOK_OP) {
            /* compute operation */
            s32 op = tok->op;
            s32 ar = opAr[op];
            u32 *ops = vector_at(&stack, stack.size - ar);
            bool sign = TRUE;
            switch (op) {
                /* dereference operators */
                case OP_BZ: sign = FALSE;
                case OP_B:
                    if (!validateAddr(ops[0], 1)) {
                        goto addr_err;
                    }
                    value = (sign ? *(s8 *)ops[0] : *(u8 *)ops[0]);
                    break;
                case OP_HZ: sign = FALSE;
                case OP_H:
                    if (!validateAddr(ops[0], 2)) {
                        goto addr_err;
                    }
                    value = (sign ? *(s16 *)ops[0] : *(u16 *)ops[0]);
                    break;
                case OP_W:
                    if (!validateAddr(ops[0], 4)) {
                        goto addr_err;
                    }
                    value = *(u32 *)ops[0];
                    break;
                /* arithmetic operators */
                case OP_ADD: value = ops[0] + ops[1]; break;
                case OP_SUB: value = ops[0] - ops[1]; break;
                case OP_MUL: value = ops[0] * ops[1]; break;
                case OP_DIV:
                    if (ops[1] == 0) {
                        goto arith_err;
                    }
                    value = ops[0] / ops[1];
                    break;
                case OP_REM:
                    if (ops[1] == 0) {
                        goto arith_err;
                    }
                    value = ops[0] % ops[1];
                    break;
            }
            /* pop operands */
            vector_erase(&stack, stack.size - ar, ar);
        }
        /* push result */
        if (!vector_push_back(&stack, 1, &value)) {
            goto mem_err;
        }
    }
    u32 *top = vector_at(&stack, stack.size - 1);
    *result = *top;
    e = ADEX_ERROR_SUCCESS;
    goto exit;
mem_err:
    e = ADEX_ERROR_MEMORY;
    goto exit;
arith_err:
    e = ADEX_ERROR_ARITHMETIC;
    goto exit;
addr_err:
    e = ADEX_ERROR_ADDRESS;
    goto exit;
exit:
    vector_destroy(&stack);
    return e;
}

void adexDestroy(struct Adex *adex) {
    vector_destroy(&adex->expr);
}
