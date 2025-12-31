#ifndef VM_H
#define VM_H

#define STACK_MAX MB(8)

typedef Enum(u8, interpret_result){
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
};

typedef struct {
    chunk *chunks;
    u8 *ip;

    value stack[STACK_MAX];
    value *stack_top;
} vm;

#endif // VM_H
