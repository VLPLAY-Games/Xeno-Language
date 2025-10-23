#ifndef XENO_COMMON_H
#define XENO_COMMON_H

#include <Arduino.h>

// Operation codes for Xeno bytecode
enum XenoOpcodes {
    OP_NOP = 0,
    OP_PRINT = 1,
    OP_LED_ON = 2,
    OP_LED_OFF = 3,  
    OP_DELAY = 4,
    OP_PUSH = 5,
    OP_POP = 6,
    OP_ADD = 7,
    OP_SUB = 8,
    OP_MUL = 9,
    OP_DIV = 10,
    OP_JUMP = 11,
    OP_JUMP_IF = 12,
    OP_PRINT_NUM = 13,
    OP_STORE = 14,
    OP_LOAD = 15,
    OP_MOD = 16,
    OP_ABS = 17,
    OP_POW = 18,
    OP_EQ = 19,
    OP_NEQ = 20,
    OP_LT = 21,
    OP_GT = 22,
    OP_LTE = 23,
    OP_GTE = 24,
    OP_HALT = 255,
    OP_PUSH_FLOAT = 25,
    OP_PUSH_STRING = 26
};

// Data types
enum XenoDataType {
    TYPE_INT = 0,
    TYPE_FLOAT = 1,
    TYPE_STRING = 2
};

// Value structure that can hold different data types
struct XenoValue {
    XenoDataType type;
    union {
        int32_t int_val;
        float float_val;
        uint16_t string_index;
    };
    
    XenoValue() : type(TYPE_INT), int_val(0) {}
    
    static XenoValue makeInt(int32_t val) {
        XenoValue v;
        v.type = TYPE_INT;
        v.int_val = val;
        return v;
    }
    
    static XenoValue makeFloat(float val) {
        XenoValue v;
        v.type = TYPE_FLOAT;
        v.float_val = val;
        return v;
    }
    
    static XenoValue makeString(uint16_t str_idx) {
        XenoValue v;
        v.type = TYPE_STRING;
        v.string_index = str_idx;
        return v;
    }
};

// Bytecode instruction structure
struct XenoInstruction {
    uint8_t opcode;
    uint32_t arg1;
    uint16_t arg2;
    
    XenoInstruction(uint8_t op = OP_NOP, uint32_t a1 = 0, uint16_t a2 = 0) 
        : opcode(op), arg1(a1), arg2(a2) {}
};

// Структура для хранения информации о цикле
struct LoopInfo {
    String var_name;
    int start_address;
    int condition_address;
    int end_jump_address;
};

#endif // XENO_COMMON_H