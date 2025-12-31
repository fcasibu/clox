internal void
InitializeLines(memory_arena *arena, lines *l, usize initial_cap)
{
    Assert(arena);
    Assert(l);

    l->capacity = initial_cap;
    l->size = 0;
    l->items = PushArray(arena, initial_cap, typeof(*l->items));

    Assert(l->items);
}

internal void
InitializeChunk(memory_arena *arena, chunk *c, usize initial_cap)
{
    Assert(arena);
    Assert(c);

    c->capacity = initial_cap;
    c->size = 0;
    c->items = PushArray(arena, initial_cap, typeof(*c->items));
    Assert(c->items);

    InitializeLines(arena, &c->lines, 256);
    InitializeValues(arena, &c->constants, 256);
}

internal usize
GetLine(chunk *c, usize instruction_idx)
{
    usize offset = 0;
    for (usize i = 0; i < c->lines.size; ++i) {
        line_info info = c->lines.items[i];

        offset += info.count;
        if (instruction_idx < offset)
            return info.line;
    }

    Unreachable("Instruction index out of bounds");
}

internal inline void
WriteChunk(memory_arena *arena, chunk *c, u8 byte, usize line)
{
    Assert(arena);
    Assert(c);

    if (c->size >= c->capacity)
        GrowArray(arena, c);

    Assert(c->size < c->capacity);
    c->items[c->size++] = byte;

    if (c->lines.size > 0 && c->lines.items[c->lines.size - 1].line == line) {
        Assert(c->lines.items[c->lines.size - 1].count > 0);
        c->lines.items[c->lines.size - 1].count += 1;
    } else {
        if (c->lines.size >= c->lines.capacity)
            GrowArray(arena, &c->lines);

        Assert(c->lines.size < c->lines.capacity);
        c->lines.items[c->lines.size++] = (line_info){ line, 1 };
    }
}

internal inline usize
AddConstant(memory_arena *arena, chunk *c, value item)
{
    Assert(arena);
    Assert(c);

    WriteValue(arena, &c->constants, item);
    Assert(c->constants.size > 0);
    return c->constants.size - 1;
}

internal inline void
WriteConstant(memory_arena *arena, chunk *c, value item, usize line)
{
    Assert(arena);
    Assert(c);

    usize constant_idx = AddConstant(arena, c, item);

    if (constant_idx > 255) {
        WriteChunk(arena, c, OP_CONSTANT_LONG, line);
        WriteChunk(arena, c, (u8)(constant_idx >> 16), line);
        WriteChunk(arena, c, (u8)(constant_idx >> 8), line);
        WriteChunk(arena, c, (u8)constant_idx, line);
    } else {
        WriteChunk(arena, c, OP_CONSTANT, line);
        WriteChunk(arena, c, (u8)constant_idx, line);
    }
}
