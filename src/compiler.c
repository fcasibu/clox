#include <errno.h>

#define PARSE_FN(name) void name(void)

internal PARSE_FN(Grouping);
internal PARSE_FN(Number);
internal PARSE_FN(Expression);
internal PARSE_FN(Unary);
internal PARSE_FN(Binary);

// clang-format off
global parse_rule Rules[] = {
    [TokenKind_LeftParen]    = {Grouping, NULL,   Prec_None  },
    [TokenKind_RightParen]   = {NULL,     NULL,   Prec_None  },
    [TokenKind_LeftBrace]    = {NULL,     NULL,   Prec_None  },
    [TokenKind_RightBrace]   = {NULL,     NULL,   Prec_None  },
    [TokenKind_Comma]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Dot]          = {NULL,     NULL,   Prec_None  },
    [TokenKind_Minus]        = {Unary,    Binary, Prec_Term  },
    [TokenKind_Plus]         = {NULL,     Binary, Prec_Term  },
    [TokenKind_Semicolon]    = {NULL,     NULL,   Prec_None  },
    [TokenKind_Slash]        = {NULL,     Binary, Prec_Factor},
    [TokenKind_Star]         = {NULL,     Binary, Prec_Factor},
    [TokenKind_QuestionMark] = {NULL,     NULL,   Prec_None  },
    [TokenKind_Colon]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Bang]         = {NULL,     NULL,   Prec_None  },
    [TokenKind_BangEqual]    = {NULL,     NULL,   Prec_None  },
    [TokenKind_Equal]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_EqualEqual]   = {NULL,     NULL,   Prec_None  },
    [TokenKind_Greater]      = {NULL,     NULL,   Prec_None  },
    [TokenKind_GreaterEqual] = {NULL,     NULL,   Prec_None  },
    [TokenKind_Less]         = {NULL,     NULL,   Prec_None  },
    [TokenKind_LessEqual]    = {NULL,     NULL,   Prec_None  },
    [TokenKind_Identifier]   = {NULL,     NULL,   Prec_None  },
    [TokenKind_String]       = {NULL,     NULL,   Prec_None  },
    [TokenKind_Number]       = {Number,   NULL,   Prec_None  },
    [TokenKind_And]          = {NULL,     NULL,   Prec_None  },
    [TokenKind_Class]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Else]         = {NULL,     NULL,   Prec_None  },
    [TokenKind_False]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Fun]          = {NULL,     NULL,   Prec_None  },
    [TokenKind_For]          = {NULL,     NULL,   Prec_None  },
    [TokenKind_If]           = {NULL,     NULL,   Prec_None  },
    [TokenKind_Nil]          = {NULL,     NULL,   Prec_None  },
    [TokenKind_Or]           = {NULL,     NULL,   Prec_None  },
    [TokenKind_Print]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Return]       = {NULL,     NULL,   Prec_None  },
    [TokenKind_Super]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_This]         = {NULL,     NULL,   Prec_None  },
    [TokenKind_True]         = {NULL,     NULL,   Prec_None  },
    [TokenKind_Var]          = {NULL,     NULL,   Prec_None  },
    [TokenKind_While]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Break]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Error]        = {NULL,     NULL,   Prec_None  },
    [TokenKind_Eof]          = {NULL,     NULL,   Prec_None  },
};
// clang-format on

global parser Parser;
chunk *CompilingChunk;

internal void
ReportParser(const char *message)
{
    fflush(stdout);
    const char *line_start = Parser.current.lexeme_start;
    while (line_start > Parser.source_start && line_start[-1] != '\n')
        line_start -= 1;

    const char *line_end = Parser.current.lexeme_start;
    while (*line_end != '\0' && *line_end != '\n')
        line_end += 1;

    int line_len = (int)(line_end - line_start);
    int indent = (int)(Parser.current.lexeme_start - line_start);

    fprintf(
        stderr, "%s:%zu:%zu: %s\n", FILE_NAME, Parser.current.line, Parser.current.col, message);
    fprintf(stderr, " %4d | %.*s\n", (int)Parser.current.line, line_len, line_start);
    fprintf(stderr, "      | %*s", indent, "");

    for (usize i = 0; i < Parser.current.length; ++i)
        fputc('^', stderr);
    fputc('\n', stderr);
    fflush(stderr);

    Parser.had_error = true;
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

internal inline parse_rule *
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

internal
PARSE_FN(Unary)
{
    token_kind kind = Parser.previous.kind;

    ParsePrecedence(Prec_Unary);

    switch (kind) {
            // clang-format off
        case TokenKind_Minus: EmitByte(OP_NEGATE);  break;

        INVALID_DEFAULT_CASE;
            // clang-format on
    }
}

internal
PARSE_FN(Binary)
{
    token_kind kind = Parser.previous.kind;
    parse_rule *rule = GetRule(kind);
    ParsePrecedence(rule->precedence + 1);

    switch (kind) {
            // clang-format off
        case TokenKind_Plus : EmitByte(OP_ADD)     ; break;
        case TokenKind_Minus: EmitByte(OP_SUBTRACT); break;
        case TokenKind_Star : EmitByte(OP_MULTIPLY); break;
        case TokenKind_Slash: EmitByte(OP_DIVIDE)  ; break;

        INVALID_DEFAULT_CASE;
            // clang-format on
    }
}

internal
PARSE_FN(Expression)
{
    ParsePrecedence(Prec_Assignment);
}

internal
PARSE_FN(Grouping)
{
    Expression();
    ConsumeParser(TokenKind_RightParen, "Expected closing parenthesis.");
}

internal
PARSE_FN(Number)
{
    token tok = Parser.previous;

    char tmp[tok.length + 1];
    memcpy(tmp, tok.lexeme_start, tok.length);
    tmp[tok.length] = '\0';

    errno = 0;
    char *end = NULL;
    f64 number_val = strtod(tmp, &end);

    if (errno == ERANGE || *end != '\0') {
        ReportParser("Failed to parse numeric literal.");
        return;
    }

    EmitConstant(number_val);
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
