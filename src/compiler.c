#include <errno.h>

#define PARSE_FN(name) void name(void)

internal PARSE_FN(Grouping);
internal PARSE_FN(Number);
internal PARSE_FN(Literal);
internal PARSE_FN(Expression);
internal PARSE_FN(Unary);
internal PARSE_FN(Binary);
internal PARSE_FN(Ternary);

// clang-format off
global_const parse_rule Rules[] = {
    [TokenKind_LeftParen]    = {Grouping, NULL,    Prec_None               },
    [TokenKind_RightParen]   = {NULL,     NULL,    Prec_None               },
    [TokenKind_LeftBrace]    = {NULL,     NULL,    Prec_None               },
    [TokenKind_RightBrace]   = {NULL,     NULL,    Prec_None               },
    [TokenKind_Comma]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Dot]          = {NULL,     NULL,    Prec_None               },
    [TokenKind_Minus]        = {Unary,    Binary,  Prec_Term               },
    [TokenKind_Plus]         = {NULL,     Binary,  Prec_Term               },
    [TokenKind_Semicolon]    = {NULL,     NULL,    Prec_None               },
    [TokenKind_Slash]        = {NULL,     Binary,  Prec_Factor             },
    [TokenKind_Star]         = {NULL,     Binary,  Prec_Factor             },
    [TokenKind_QuestionMark] = {NULL,     Ternary, Prec_Ternary            },
    [TokenKind_Colon]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Bang]         = {Unary,    NULL,    Prec_None               },
    [TokenKind_BangEqual]    = {NULL,     Binary,  Prec_Equality           },
    [TokenKind_EqualEqual]   = {NULL,     Binary,  Prec_Equality           },
    [TokenKind_Equal]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Greater]      = {NULL,     Binary,  Prec_Comparison         },
    [TokenKind_GreaterEqual] = {NULL,     Binary,  Prec_Comparison         },
    [TokenKind_Less]         = {NULL,     Binary,  Prec_Comparison         },
    [TokenKind_LessEqual]    = {NULL,     Binary,  Prec_Comparison         },
    [TokenKind_Identifier]   = {NULL,     NULL,    Prec_None               },
    [TokenKind_String]       = {NULL,     NULL,    Prec_None               },
    [TokenKind_Number]       = {Number,   NULL,    Prec_None               },
    [TokenKind_And]          = {NULL,     NULL,    Prec_None               },
    [TokenKind_Class]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Else]         = {NULL,     NULL,    Prec_None               },
    [TokenKind_Fun]          = {NULL,     NULL,    Prec_None               },
    [TokenKind_For]          = {NULL,     NULL,    Prec_None               },
    [TokenKind_If]           = {NULL,     NULL,    Prec_None               },
    [TokenKind_Nil]          = {Literal,  NULL,    Prec_None               },
    [TokenKind_Or]           = {NULL,     NULL,    Prec_None               },
    [TokenKind_Print]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Return]       = {NULL,     NULL,    Prec_None               },
    [TokenKind_Super]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_This]         = {NULL,     NULL,    Prec_None               },
    [TokenKind_True]         = {Literal,  NULL,    Prec_None               },
    [TokenKind_False]        = {Literal,  NULL,    Prec_None               },
    [TokenKind_Var]          = {NULL,     NULL,    Prec_None               },
    [TokenKind_While]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Break]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Error]        = {NULL,     NULL,    Prec_None               },
    [TokenKind_Eof]          = {NULL,     NULL,    Prec_None               },
};
// clang-format on

global parser Parser;
global chunk *CompilingChunk;

internal inline void
GetLexeme(char **out)
{
    token tok = Parser.previous;
    char tmp[tok.length + 1];
    memcpy(tmp, tok.lexeme_start, tok.length);
    tmp[tok.length] = '\0';

    *out = tmp;
}

internal void
ReportParser(const char *message)
{
    fflush(stdout);
    const char *line_start = Parser.previous.lexeme_start;
    while (line_start > Parser.source_start && line_start[-1] != '\n')
        line_start -= 1;

    const char *line_end = Parser.previous.lexeme_start;
    while (*line_end != '\0' && *line_end != '\n')
        line_end += 1;

    int line_len = (int)(line_end - line_start);
    int indent = (int)(Parser.previous.lexeme_start - line_start);

    fprintf(
        stderr, "%s:%zu:%zu: %s\n", FILE_NAME, Parser.previous.line, Parser.previous.col, message);
    fprintf(stderr, " %4d | %.*s\n", (int)Parser.previous.line, line_len, line_start);
    fprintf(stderr, "      | %*s", indent, "");

    for (usize i = 0; i < Parser.previous.length; ++i)
        fputc('^', stderr);
    fputc('\n', stderr);
    fflush(stderr);

    Parser.had_error = true;
}

internal inline const parse_rule *
GetRule(token_kind kind)
{
    return &Rules[kind];
}

internal void
AdvanceParser(void)
{
    Parser.previous = Parser.current;

    for (;;) {
        Parser.current = ScanToken();

        if (Parser.current.kind != TokenKind_Error)
            break;
    }
}

internal inline void
EmitBytes(u8 b1, u8 b2)
{
    WriteChunk(Parser.arena, CompilingChunk, b1, Parser.previous.line);
    AdvanceParser();
    WriteChunk(Parser.arena, CompilingChunk, b2, Parser.previous.line);
}

internal inline void
EmitByte(u8 byte)
{
    WriteChunk(Parser.arena, CompilingChunk, byte, Parser.previous.line);
}

internal inline void
EmitConstant(value constant)
{
    WriteConstant(Parser.arena, CompilingChunk, constant, Parser.previous.line);
}

internal void
ParsePrecedence(token_precedence precedence)
{
    AdvanceParser();
    parse_fn PrefixRuleFn = GetRule(Parser.previous.kind)->prefix;

    if (!PrefixRuleFn) {
        ReportParser("Expected expression.");
        return;
    }

    PrefixRuleFn();

    while (precedence <= GetRule(Parser.current.kind)->precedence) {
        AdvanceParser();
        parse_fn InfixRuleFn = GetRule(Parser.previous.kind)->infix;
        Assert(InfixRuleFn);

        InfixRuleFn();
    }
}

internal inline void
ConsumeParser(token_kind kind, const char *message)
{
    if (Parser.current.kind == kind) {
        AdvanceParser();
        return;
    }

    ReportParser(message);
}

internal inline PARSE_FN(Unary)
{
    token_kind kind = Parser.previous.kind;

    ParsePrecedence(Prec_Unary);

    // clang-format off
    switch (kind) {
        case TokenKind_Minus: EmitByte(OP_NEGATE);  break;
        case TokenKind_Bang: EmitByte(OP_NOT); break;

        INVALID_DEFAULT_CASE;
    }
    // clang-format on
}

internal inline PARSE_FN(Binary)
{
    token_kind kind = Parser.previous.kind;
    const parse_rule *rule = GetRule(kind);
    ParsePrecedence(rule->precedence + 1);

    // clang-format off
    switch (kind) {
        case TokenKind_Plus        : EmitByte(OP_ADD)               ; break;
        case TokenKind_Minus       : EmitByte(OP_SUBTRACT)          ; break;
        case TokenKind_Star        : EmitByte(OP_MULTIPLY)          ; break;
        case TokenKind_Slash       : EmitByte(OP_DIVIDE)            ; break;

        case TokenKind_BangEqual   : EmitBytes(OP_EQUAL, OP_NOT)    ; break;
        case TokenKind_EqualEqual  : EmitByte(OP_EQUAL)             ; break;

        case TokenKind_Less        : EmitBytes(OP_LESS, OP_NOT)     ; break;
        case TokenKind_LessEqual   : EmitByte(OP_LESS)              ; break;
        case TokenKind_Greater     : EmitByte(OP_GREATER)           ; break;
        case TokenKind_GreaterEqual: EmitBytes(OP_GREATER, OP_NOT)  ; break;

        INVALID_DEFAULT_CASE;
    }
    // clang-format on
}

internal inline PARSE_FN(Ternary)
{
    Expression();
    ConsumeParser(TokenKind_Colon, "Expected ':'");
    Expression();
}

internal inline PARSE_FN(Expression)
{
    ParsePrecedence(Prec_Assignment);
}

internal inline PARSE_FN(Grouping)
{
    Expression();
    ConsumeParser(TokenKind_RightParen, "Expected closing parenthesis.");
}

internal inline PARSE_FN(Number)
{
    char *tmp = NULL;
    GetLexeme(&tmp);

    if (!tmp) {
        ReportParser("Invalid lexeme.");
        return;
    }

    errno = 0;
    char *end = NULL;
    f64 number_val = strtod(tmp, &end);

    if (errno == ERANGE || *end != '\0') {
        ReportParser("Failed to parse numeric literal.");
        return;
    }

    EmitConstant(NumberVal(number_val));
}

internal inline PARSE_FN(Literal)
{
    // clang-format off
    switch(Parser.previous.kind) {
        case TokenKind_Nil: EmitByte(OP_NIL); break;
        case TokenKind_True: EmitByte(OP_TRUE); break;
        case TokenKind_False: EmitByte(OP_FALSE); break;

        INVALID_DEFAULT_CASE;
    }
}

internal inline PARSE_FN(Nil)
{
    char *tmp = NULL;
    GetLexeme(&tmp);

    if (!tmp) {
        ReportParser("Invalid lexeme.");
        return;
    }

    EmitConstant(NilVal());
}

internal inline void
EndCompile(void)
{
    WriteChunk(Parser.arena, CompilingChunk, OP_RETURN, Parser.previous.line);
}

internal b32
Compile(memory_arena *arena, const char *source, chunk *c)
{
    InitializeLexer(source);

    Parser.source_start = source;
    Parser.arena = arena;
    CompilingChunk = c;

    AdvanceParser();
    Expression();

    ConsumeParser(TokenKind_Eof, "Expected end of expression.");
    EndCompile();

    return !Parser.had_error;
}
