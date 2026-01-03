global vm VM;

internal inline void
PushStack(value v)
{
    *VM.stack_top = v;
    VM.stack_top++;
}

internal inline value
PopStack()
{
    VM.stack_top--;
    return *VM.stack_top;
}

internal inline void
ResetStack()
{
    VM.stack_top = VM.stack;
}

internal inline void
InitializeVM()
{
    ResetStack();
}

internal interpret_result
Run()
{
#define READ_BYTE() (*VM.ip++)
#define READ_CONSTANT() (VM.chunks->constants.items[READ_BYTE()])
#define READ_CONSTANT_LONG() \
    (VM.chunks->constants.items[(READ_BYTE() << 16) | (READ_BYTE() << 8) | READ_BYTE()])
#define BINARY_OP(op) Stmt(double b = PopStack(); double a = PopStack(); PushStack(a op b);)

    for (;;) {
#if DEBUG
        printf("%10s", "");
        for (value *slot = VM.stack; slot < VM.stack_top; ++slot) {
            printf("[ ");
            PrintValue(*slot);
            printf(" ]");
        }
        printf("\n");

        Assert(VM.chunks && VM.chunks->items);
        Assert(VM.ip >= VM.chunks->items && VM.ip < VM.chunks->items + VM.chunks->size);
        isize offset = VM.ip - VM.chunks->items;
        DissassembleInstruction(VM.chunks, offset);
#endif

        u8 instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                value constant = READ_CONSTANT();
                PushStack(constant);
            } break;

            case OP_CONSTANT_LONG: {
                value constant = READ_CONSTANT_LONG();
                PushStack(constant);
            } break;

                // clang-format off
            case OP_ADD:      BINARY_OP(+);             break;
            case OP_SUBTRACT: BINARY_OP(-);             break;
            case OP_MULTIPLY: BINARY_OP(*);             break;
            case OP_DIVIDE:   BINARY_OP(/);             break;
                // clang-format on

            case OP_NEGATE: {
                Assert(VM.stack_top > VM.stack);
                *(VM.stack_top - 1) *= -1.0f;
            } break;

            case OP_RETURN: {
                PrintValue(PopStack());
                printf("\n");
                return Interpret_Ok;
            }

            default:
                return Interpret_RuntimeError;
        }
    }

#undef READ_BYTE
#undef READ_LONG
#undef READ_CONSTANT
#undef BINARY_OP
}

internal interpret_result
Interpret(context *ctx, const char *source)
{
    Assert(ctx);
    Assert(source);

    chunk c = { 0 };
    temporary_memory temp_mem = BeginTemporaryMemory(&ctx->temporary_arena);
    InitializeChunk(temp_mem.arena, &c, 256);

    VM.chunks = &c;
    VM.ip = VM.chunks->items;

    if (!Compile(&ctx->main_arena, source, &c)) {
        EndTemporaryMemory(temp_mem);
        return Interpret_CompileError;
    }

    interpret_result result = Run();

    EndTemporaryMemory(temp_mem);
    return result;
}
