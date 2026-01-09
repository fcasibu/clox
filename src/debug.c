internal inline usize
SimpleInstruction(const char *name, usize offset)
{
    printf("%s\n", name);
    return offset + 1;
}

internal inline usize
ConstantLongInstruction(chunk *c, const char *name, usize offset)
{
    Assert(offset + 3 < c->size);
    usize constant_idx = (c->items[offset + 1] << 16) | (c->items[offset + 2] << 8) |
                         (c->items[offset + 3]);

    printf("%-16s %4zu '", name, constant_idx);

    Assert(constant_idx < c->constants.size);
    PrintValue(c->constants.items[constant_idx]);
    printf("'\n");

    return offset + 4;
}

internal inline usize
ConstantInstruction(chunk *c, const char *name, usize offset)
{
    Assert(offset + 1 < c->size);
    usize constant_idx = c->items[offset + 1];
    printf("%-16s %4zu '", name, constant_idx);

    Assert(constant_idx < c->constants.size);
    PrintValue(c->constants.items[constant_idx]);
    printf("'\n");

    return offset + 2;
}

internal usize
DissassembleInstruction(chunk *c, usize offset)
{
    Assert(c);
    Assert(offset < c->size);
    printf("%04zu ", offset);

    if (offset > 0 && GetLine(c, offset) == GetLine(c, offset - 1)) {
        printf("%3s| ", "");
    } else {
        printf("%4zu ", GetLine(c, offset));
    }

    op_code instruction = c->items[offset];

    switch (instruction) {
        case OP_RETURN: {
            return SimpleInstruction("OP_RETURN", offset);
        }

        case OP_NIL: {
            return SimpleInstruction("OP_NIL", offset);
        }

        case OP_TRUE: {
            return SimpleInstruction("OP_TRUE", offset);
        }

        case OP_FALSE: {
            return SimpleInstruction("OP_FALSE", offset);
        }

        case OP_CONSTANT: {
            return ConstantInstruction(c, "OP_CONSTANT", offset);
        }

        case OP_CONSTANT_LONG: {
            return ConstantLongInstruction(c, "OP_CONSTANT_LONG", offset);
        }

        case OP_NEGATE: {
            return SimpleInstruction("OP_NEGATE", offset);
        }

        case OP_ADD: {
            return SimpleInstruction("OP_ADD", offset);
        }

        case OP_SUBTRACT: {
            return SimpleInstruction("OP_SUBTRACT", offset);
        }

        case OP_MULTIPLY: {
            return SimpleInstruction("OP_MULTIPLY", offset);
        }

        case OP_DIVIDE: {
            return SimpleInstruction("OP_DIVIDE", offset);
        }

        case OP_NOT: {
            return SimpleInstruction("OP_NOT", offset);
        }

        case OP_EQUAL: {
            return SimpleInstruction("OP_EQUAL", offset);
        }

        case OP_LESS: {
            return SimpleInstruction("OP_LESS", offset);
        }

        case OP_GREATER: {
            return SimpleInstruction("OP_GREATER", offset);
        }

        default: {
            printf("Unknown opcode: %u\n", instruction);
            return offset + 1;
        }
    }
}

internal void
DissassembleChunk(chunk *c, const char *name)
{
    Assert(c);
    printf("== %s ==\n", name);

    for (usize offset = 0; offset < c->size;) {
        offset = DissassembleInstruction(c, offset);
    }
}
