#ifndef VAL_H
#define VAL_H

typedef Enum(u8, value_type){
    Value_Bool,
    Value_Number,
    Value_Nil,
};

typedef struct {
    value_type type;
    union {
        bool boolean;
        f32 number;
    } as;
} value;

typedef struct {
    usize size;
    usize capacity;
    value *items;
} values;

#define NilVal() ((value){ Value_Nil, .as.number = 0 })
#define NumberVal(v) ((value){ Value_Number, .as.number = v })
#define BoolVal(v) ((value){ Value_Bool, .as.boolean = v })

#define IsNil(v) ((v).type == Value_Nil)
#define IsNumber(v) ((v).type == Value_Number)
#define IsBoolean(v) ((v).type == Value_Bool)

#define AsNumber(v) ((v).as.number)
#define AsBoolean(v) ((v).as.boolean)

#endif // VAL_H
