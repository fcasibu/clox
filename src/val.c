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

internal inline void
WriteValue(memory_arena *arena, values *v, value item)
{
    Assert(arena);
    Assert(v);

    if (v->size >= v->capacity)
        GrowArray(arena, v);

    Assert(v->size < v->capacity);
    v->items[v->size++] = item;
}

internal void
PrintValue(value item)
{
    switch (item.type) {
        case Value_Nil: {
            printf("nil");
        } break;

        case Value_Bool: {
            printf(AsBoolean(item) ? "true" : "false");
        } break;

        case Value_Number: {
            printf("%g", AsNumber(item));
        } break;

            INVALID_DEFAULT_CASE;
    }
}

internal inline b32
IsFalsyValue(value v)
{
    return IsNil(v) || (IsBoolean(v) && !AsBoolean(v));
}

internal b32
AreValuesEqual(value a, value b)
{
    if (a.type != b.type)
        return false;

    switch (a.type) {
        case Value_Number:
            return AsNumber(a) == AsNumber(b);
        case Value_Bool:
            return AsBoolean(a) == AsBoolean(b);
        case Value_Nil:
            return true;

            INVALID_DEFAULT_CASE;
    }
}
