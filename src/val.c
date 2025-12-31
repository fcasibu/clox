internal void
InitializeValues(memory_arena *arena, values *v, usize initial_cap)
{
    Assert(arena);
    Assert(v);

    v->capacity = initial_cap;
    v->size = 0;
    v->items = PushArray(arena, initial_cap, typeof(*v->items));

    Assert(v->items);
}

internal void
WriteValue(memory_arena *arena, values *v, value item)
{
    Assert(arena);
    Assert(v);

    if (v->size >= v->capacity)
        GrowArray(arena, v);

    Assert(v->size < v->capacity);
    v->items[v->size++] = item;
}

internal inline void
PrintValue(value item)
{
    printf("%g", item);
}
