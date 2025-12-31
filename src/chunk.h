#ifndef CHUNK_H
#define CHUNK_H

typedef Enum(u8, op_code){
    OP_CONSTANT, OP_CONSTANT_LONG, OP_NEGATE, OP_ADD,
    OP_SUBTRACT, OP_MULTIPLY,      OP_DIVIDE, OP_RETURN,
};

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
