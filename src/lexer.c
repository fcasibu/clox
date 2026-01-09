#include <ctype.h>

// TODO(fcasibu): refactor context
global const char *FILE_NAME = "main.lox";

global lexer Lexer;

// clang-format off
global_const struct {
    const char *identifier;
    token_kind kind;
} RESERVED_WORDS_TABLE[] = {
    {"true",  TokenKind_True},
    {"false", TokenKind_False},
    {"nil",   TokenKind_Nil},
};
// clang-format on

internal void
ReportLexer(const char *message)
{
    fflush(stdout);
    const char *line_start = Lexer.lexeme_start;
    while (line_start > Lexer.source_start && line_start[-1] != '\n')
        line_start -= 1;

    const char *line_end = Lexer.lexeme_start;
    while (*line_end != '\0' && *line_end != '\n')
        line_end += 1;

    int line_len = (int)(line_end - line_start);
    int indent = (int)(Lexer.lexeme_start - line_start);

    const char *visual_end = Min(Lexer.current_char, line_end);
    int visual_len = (int)(visual_end - Lexer.lexeme_start);
    if (visual_len <= 0)
        visual_len = 1;

    fprintf(stderr, "%s:%zu:%zu: %s\n", FILE_NAME, Lexer.token_line, Lexer.token_col, message);
    fprintf(stderr, " %4d | %.*s\n", (int)Lexer.token_line, line_len, line_start);
    fprintf(stderr, "      | %*s", indent, "");

    for (int i = 0; i < visual_len; ++i)
        fputc('^', stderr);
    fputc('\n', stderr);
    fflush(stderr);
}

internal void
InitializeLexer(const char *source)
{
    Lexer.current_char = source;
    Lexer.lexeme_start = source;
    Lexer.source_start = source;
    Lexer.line = 1;
    Lexer.col = 1;
    Lexer.token_line = 1;
    Lexer.token_col = 1;
}

internal token_kind
LookupKeyword(const char *str)
{
    local_const usize count = ArrayCount(RESERVED_WORDS_TABLE);

    for (usize i = 0; i < count; ++i) {
        if (strcmp(str, RESERVED_WORDS_TABLE[i].identifier) == 0)
            return RESERVED_WORDS_TABLE[i].kind;
    }

    ReportLexer("Unexpected token");
    return TokenKind_Error;
}

internal inline token
MakeToken(token_kind kind)
{
    token result = {};
    result.lexeme_start = Lexer.lexeme_start;
    result.length = Lexer.current_char - Lexer.lexeme_start;
    result.col = Lexer.col - result.length;
    result.line = Lexer.line;
    result.kind = kind;

    return result;
}

internal inline char
PeekToken(usize dist)
{
    return Lexer.current_char[dist];
}

internal inline char
AdvanceLexer(void)
{
    if (*Lexer.current_char == '\0')
        return '\0';

    char c = *Lexer.current_char++;

    if (c == '\n') {
        Lexer.line += 1;
        Lexer.col = 1;
    } else {
        Lexer.col += 1;
    }

    return c;
}

internal token
LexerIdentifier(void)
{
    while ((isalnum(PeekToken(0)) || PeekToken(0) == '_') && PeekToken(0) != '\0')
        AdvanceLexer();

    usize length = Lexer.current_char - Lexer.lexeme_start;
    char tmp[length + 1];
    strncpy(tmp, Lexer.lexeme_start, length);
    tmp[length] = '\0';

    return MakeToken(LookupKeyword(tmp));
}

internal token
LexerString(void)
{
    while (PeekToken(0) != '"' && PeekToken(0) != '\0')
        AdvanceLexer();

    if (PeekToken(0) == '\0') {
        ReportLexer("Unterminated string.");
        return MakeToken(TokenKind_Error);
    }

    AdvanceLexer();

    return MakeToken(TokenKind_String);
}

internal token
LexerNumber(void)
{
    while (isdigit(PeekToken(0)) && PeekToken(0) != '\0')
        AdvanceLexer();

    if (PeekToken(0) == '.') {
        AdvanceLexer();

        if (!isdigit(PeekToken(0))) {
            ReportLexer("Invalid numeric literal: expected digits after '.'.");
            return MakeToken(TokenKind_Error);
        }

        while (isdigit(PeekToken(0)) && PeekToken(0) != '\0')
            AdvanceLexer();
    }

    return MakeToken(TokenKind_Number);
}

internal token
ScanToken(void)
{
#define ReturnNextMatchOrCurrent(char, next_token, current_token) \
    do {                                                          \
        if (PeekToken(0) == char) {                               \
            AdvanceLexer();                                       \
            return MakeToken(next_token);                         \
        }                                                         \
        return MakeToken(current_token);                          \
    } while (0)

    for (;;) {
        while (isspace(PeekToken(0)))
            AdvanceLexer();

        Lexer.lexeme_start = Lexer.current_char;
        Lexer.token_line = Lexer.line;
        Lexer.token_col = Lexer.col;

        if (PeekToken(0) == '/' && PeekToken(1) == '/') {
            while (PeekToken(0) != '\n' && PeekToken(0) != '\0')
                AdvanceLexer();
        } else if (PeekToken(0) == '/' && PeekToken(1) == '*') {
            while (PeekToken(0) != '\0') {
                if (PeekToken(0) == '*' && PeekToken(1) == '/')
                    break;

                AdvanceLexer();
            }

            if (PeekToken(0) != '*' && PeekToken(1) != '/') {
                ReportLexer("Unterminated comment.");
                return MakeToken(TokenKind_Error);
            }

            AdvanceLexer();
            AdvanceLexer();
            continue;
        }

        char ch = AdvanceLexer();

        switch (ch) {
            case '\0':
                return MakeToken(TokenKind_Eof);

            case '\n':
                continue;

            case '(':
                return MakeToken(TokenKind_LeftParen);

            case ')':
                return MakeToken(TokenKind_RightParen);

            case '{':
                return MakeToken(TokenKind_LeftBrace);

            case '}':
                return MakeToken(TokenKind_RightBrace);

            case ',':
                return MakeToken(TokenKind_Comma);

            case '.':
                return MakeToken(TokenKind_Dot);

            case '-':
                return MakeToken(TokenKind_Minus);

            case '+':
                return MakeToken(TokenKind_Plus);

            case ';':
                return MakeToken(TokenKind_Semicolon);

            case '*':
                return MakeToken(TokenKind_Star);

            case '?':
                return MakeToken(TokenKind_QuestionMark);

            case ':':
                return MakeToken(TokenKind_Colon);

            case '!':
                ReturnNextMatchOrCurrent('=', TokenKind_BangEqual, TokenKind_Bang);

            case '=':
                ReturnNextMatchOrCurrent('=', TokenKind_EqualEqual, TokenKind_Equal);

            case '>':
                ReturnNextMatchOrCurrent('=', TokenKind_GreaterEqual, TokenKind_Greater);

            case '<':
                ReturnNextMatchOrCurrent('=', TokenKind_LessEqual, TokenKind_Less);

            case '/':
                return MakeToken(TokenKind_Slash);

            case '"':
                return LexerString();

            default: {
                if (isdigit(ch))
                    return LexerNumber();

                if (isalnum(ch) || ch == '_')
                    return LexerIdentifier();

                ReportLexer("Unexpected token");
                return MakeToken(TokenKind_Error);
            };
        }
    }

#undef ReturnNextMatchOrCurrent
}
