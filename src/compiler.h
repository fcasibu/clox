#ifndef COMPILER_H
#define COMPILER_H

// clang-format off
typedef Enum(u8, token_precedence){
    Prec_None,
    Prec_Assignment,
    Prec_Or,
    Prec_And,
    Prec_Equality,
    Prec_Comparison,
    Prec_Term,
    Prec_Factor,
    Prec_Unary,
    Prec_Call,
    Prec_Primary,
};
// clang-format on

typedef struct {
    const char *source_start;

    token previous;
    token current;

    b32 had_error;
    memory_arena *arena;
} parser;

typedef void (*parse_fn)(void);

typedef struct {
    parse_fn prefix;
    parse_fn infix;
    token_precedence precedence;
} parse_rule;

#endif // COMPILER_H
