#include <ctype.h>

// TODO(fcasibu): refactor context
global const char *FILE_NAME = "main.lox";

global lexer Lexer;

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
PeekNextToken(void)
{
    if (*Lexer.current_char == '\0')
        return '\0';

    return *(Lexer.current_char + 1);
}

internal inline char
PeekToken(void)
{
    return *Lexer.current_char;
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
LexerString(void)
{
    while (PeekToken() != '"' && PeekToken() != '\0')
        AdvanceLexer();

    if (PeekToken() == '\0') {
        ReportLexer("Unterminated string.");
        return MakeToken(TokenKind_Error);
    }

    AdvanceLexer();

    return MakeToken(TokenKind_String);
}

internal token
LexerNumber(void)
{
    while (isdigit(PeekToken()) && PeekToken() != '\0')
        AdvanceLexer();

    if (PeekToken() == '.') {
        AdvanceLexer();

        if (!isdigit(PeekToken())) {
            ReportLexer("Invalid numeric literal: expected digits after '.'.");
            return MakeToken(TokenKind_Error);
        }

        while (isdigit(PeekToken()) && PeekToken() != '\0')
            AdvanceLexer();
    }

    return MakeToken(TokenKind_Number);
}

internal token
ScanToken(void)
{
#define ReturnNextMatchOrCurrent(char, next_token, current_token) \
    do {                                                          \
        if (PeekNextToken() == char) {                            \
            AdvanceLexer();                                       \
            return MakeToken(next_token);                         \
        }                                                         \
        return MakeToken(current_token);                          \
    } while (0)

    for (;;) {
        while (isspace(PeekToken()))
            AdvanceLexer();

        Lexer.lexeme_start = Lexer.current_char;
        Lexer.token_line = Lexer.line;
        Lexer.token_col = Lexer.col;

        if (PeekToken() == '/' && PeekNextToken() == '/') {
            while (PeekToken() != '\n' && PeekToken() != '\0')
                AdvanceLexer();
        } else if (PeekToken() == '/' && PeekNextToken() == '*') {
            while (PeekToken() != '\0') {
                if (PeekToken() == '*' && PeekNextToken() == '/')
                    break;

                AdvanceLexer();
            }

            if (PeekToken() != '*' && PeekNextToken() != '/') {
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

                if (isalnum(ch) || ch == '_') {
                    while ((isalnum(PeekToken()) || PeekToken() == '_') && PeekToken() != '\0')
                        AdvanceLexer();

                    return MakeToken(TokenKind_Identifier);
                }

                ReportLexer("Unexpected token");
                return MakeToken(TokenKind_Error);
            };
        }
    }

#undef ReturnNextMatchOrCurrent
}
