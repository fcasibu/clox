#ifndef LEXER_H
#define LEXER_H

// clang-format off
typedef Enum(u8, token_kind) {
  TokenKind_Error,
  // Single-character tokens.
  TokenKind_LeftParen, TokenKind_RightParen, TokenKind_LeftBrace, TokenKind_RightBrace,
  TokenKind_Comma, TokenKind_Dot, TokenKind_Minus, TokenKind_Plus, TokenKind_Semicolon, 
  TokenKind_Slash, TokenKind_Star, TokenKind_QuestionMark, TokenKind_Colon,

  // One or two character tokens.
  TokenKind_Bang, TokenKind_BangEqual,
  TokenKind_Equal, TokenKind_EqualEqual,
  TokenKind_Greater, TokenKind_GreaterEqual,
  TokenKind_Less, TokenKind_LessEqual,

  // Literals.
  TokenKind_Identifier, TokenKind_String, TokenKind_Number,

  // Keywords.
  TokenKind_And, TokenKind_Class, TokenKind_Else, TokenKind_False, TokenKind_Fun, 
  TokenKind_For, TokenKind_If, TokenKind_Nil, TokenKind_Or,
  TokenKind_Print, TokenKind_Return, TokenKind_Super, TokenKind_This, 
  TokenKind_True, TokenKind_Var, TokenKind_While, TokenKind_Break,

  TokenKind_Eof,
};
// clang-format on

typedef struct {
    token_kind kind;

    const char *lexeme_start;
    usize length;

    usize line;
    usize col;

    usize token_line;
    usize token_col;
} token;

typedef struct {
    const char *lexeme_start;
    const char *current_char;
    const char *source_start;

    usize line;
    usize col;
    usize token_line;
    usize token_col;
} lexer;

#endif // LEXER_H
