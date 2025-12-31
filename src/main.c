#include <stdio.h>
#include <sys/mman.h>

#include "base.h"

#include "arena.h"
#include "val.h"
#include "chunk.h"
#include "vm.h"
#include "lexer.h"

#include "arena.c"
#include "val.c"
#include "chunk.c"
#include "debug.c"
#include "vm.c"
#include "lexer.c"

typedef struct {
    memory_arena main_arena;
    memory_arena temporary_arena;
} context;

internal void
InitializeContext(context *ctx)
{
    Assert(ctx);

    usize permanent_storage_size = MB(64);
    usize temporary_storage_size = MB(512);
    usize total_size = permanent_storage_size + temporary_storage_size;

    u8 *permanent_storage =
        (u8 *)mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    Assert(permanent_storage);

    u8 *temporary_storage = (permanent_storage + permanent_storage_size);

    InitializeArena(&ctx->main_arena, permanent_storage_size, permanent_storage);
    InitializeArena(&ctx->temporary_arena, temporary_storage_size, temporary_storage);
}

internal const char *
ReadFile(memory_arena *arena, const char *path)
{
    FILE *file_ptr = fopen(path, "rb");

    if (!file_ptr) {
        perror("fopen failed");
        return NULL;
    }

    if (fseek(file_ptr, 0, SEEK_END) != 0) {
        perror("fseek failed");
        goto cleanup;
    }

    i64 size = ftell(file_ptr);

    if (size == -1) {
        perror("ftell failed");
        goto cleanup;
    }

    if (fseek(file_ptr, 0, SEEK_SET) != 0) {
        perror("fseek failed");
        goto cleanup;
    }

    char *source = PushSize(arena, size + 1);
    if (!source)
        goto cleanup;

    usize bytes_read = fread(source, 1, size, file_ptr);

    if (ferror(file_ptr) != 0) {
        perror("fread error");
        goto cleanup;
    }

    fclose(file_ptr);
    source[bytes_read] = '\0';

    return source;

cleanup:
    if (file_ptr)
        fclose(file_ptr);
    return NULL;
}

int
main(void)
{
    context ctx = { 0 };
    InitializeContext(&ctx);

    const char *source = ReadFile(&ctx.main_arena, "main.lox");

    lexer l = { 0 };
    InitializeLexer(&l, source);

    for (;;) {
        token t = ScanToken(&l);

        if (t.kind == TokenKind_Error)
            continue;

        printf("%s:%zu:%zu: %.*s\n", "main.lox", t.line, t.col, (int)t.length, t.lexeme_start);

        if (t.kind == TokenKind_Eof)
            break;
    }

    // chunk c = { 0 };
    // InitializeChunk(&ctx.main_arena, &c, 256);
    // InitializeVM();
    //
    // WriteConstant(&ctx.main_arena, &c, 5.0f, 4);
    // WriteChunk(&ctx.main_arena, &c, OP_NEGATE, 4);
    // WriteChunk(&ctx.main_arena, &c, OP_RETURN, 4);
    // Interpret(&c);
    //
    // DissassembleChunk(&c, "test chunk");

    FreeArena(&ctx.main_arena);

    return 0;
}
