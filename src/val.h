#ifndef VAL_H
#define VAL_H

typedef f64 value;

typedef struct {
    usize size;
    usize capacity;
    value *items;
} values;

#endif // VAL_H
