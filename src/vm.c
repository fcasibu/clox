#include <stdarg.h>
#include <stdio.h>

global vm VM;

internal inline void
ResetStack(void)
{
    VM.stack_top = VM.stack;
}

internal inline void
InitializeVM(void)
{
    ResetStack();
}

internal void
ReportRuntime(const char *fmt, ...)
{
    fflush(stdout);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputs("\n", stderr);

    line_info prev_line = VM.chunks->lines.items[VM.ip - VM.chunks->items - 1];
    fprintf(stderr, "[line %zu] in script\n", prev_line.line);
    ResetStack();

    fflush(stderr);
}

internal inline void
PushStack(value v)
{
    *VM.stack_top = v;
    VM.stack_top++;
}

internal inline value
PeekStack(usize dist)
{
    return VM.stack_top[-1 - dist];
}

internal inline value
PopStack(void)
{
    VM.stack_top--;
    return *VM.stack_top;
}

internal interpret_result
Run(void)
{
#define READ_BYTE() (*VM.ip++)
#define READ_CONSTANT() (VM.chunks->constants.items[READ_BYTE()])
#define READ_CONSTANT_LONG() \
    (VM.chunks->constants.items[(READ_BYTE() << 16) | (READ_BYTE() << 8) | READ_BYTE()])
#define BINARY_OP(value_type, op)                                 \
    do {                                                          \
        if (!IsNumber(PeekStack(0)) || !IsNumber(PeekStack(1))) { \
            ReportRuntime("Operands must be a number.");          \
            return Interpret_RuntimeError;                        \
        }                                                         \
        f32 b = AsNumber(PopStack());                             \
        f32 a = AsNumber(PopStack());                             \
        PushStack(value_type(a op b));                            \
    } while (0)

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
            case OP_NIL  : PushStack(NilVal())      ; break;
            case OP_TRUE : PushStack(BoolVal(true)) ; break;
            case OP_FALSE: PushStack(BoolVal(false)); break;
                // clang-format on

                // clang-format off
            case OP_ADD:      BINARY_OP(NumberVal, +); break;
            case OP_SUBTRACT: BINARY_OP(NumberVal, -); break;
            case OP_MULTIPLY: BINARY_OP(NumberVal, *); break;
            case OP_DIVIDE:   BINARY_OP(NumberVal, /); break;
            case OP_GREATER:  BINARY_OP(BoolVal, >); break;
            case OP_LESS:     BINARY_OP(BoolVal, <); break;
                // clang-format on

            case OP_NOT: {
                *(VM.stack_top - 1) = BoolVal(IsFalsyValue(*(VM.stack_top - 1)));
            } break;

            case OP_NEGATE: {
                value val = *(VM.stack_top - 1);
                if (!IsNumber(val)) {
                    ReportRuntime("Operand must be a number.");
                    return Interpret_RuntimeError;
                }

                *(VM.stack_top - 1) = NumberVal(AsNumber(val) * -1.0f);
            } break;

            case OP_EQUAL: {
                value b = PopStack();
                value a = PopStack();
                PushStack(BoolVal(AreValuesEqual(a, b)));
            } break;

            case OP_RETURN: {
                PrintValue(PopStack());
                printf("\n");
                return Interpret_Ok;
            }

                INVALID_DEFAULT_CASE;
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

    DissassembleChunk(&c, "tesst");

    interpret_result result = Run();

    printf("%d\n", result);

    EndTemporaryMemory(temp_mem);
    return result;
}
