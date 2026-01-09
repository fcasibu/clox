#ifndef CHUNK_H
#define CHUNK_H

// clang-format off
typedef Enum(u8, op_code){
    OP_CONSTANT, OP_CONSTANT_LONG, OP_NEGATE, OP_ADD,
    OP_SUBTRACT, OP_MULTIPLY,      OP_DIVIDE, OP_NOT,
    OP_EQUAL,    OP_GREATER,       OP_LESS,   OP_NIL,
    OP_TRUE,     OP_FALSE,         OP_RETURN,
};
// clang-format on

typedef struct {
    usize line;
    usize count;
} line_info;

typedef struct {
    usize size;
    usize capacity;
    line_info *items;
} lines;

typedef struct {
    usize size;
    usize capacity;
    u8 *items;

    lines lines;
    values constants;
} chunk;

#endif // CHUNK_H
