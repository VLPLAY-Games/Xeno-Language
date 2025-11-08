/*
 * Copyright 2025 VL_PLAY Games
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef XENOLANG_CORE_XENO_COMMON_H_
#define XENOLANG_CORE_XENO_COMMON_H_

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
    OP_PUSH_FLOAT = 25,
    OP_PUSH_STRING = 26,
    OP_MAX = 27,
    OP_MIN = 28,
    OP_SQRT = 29,
    OP_INPUT = 30,
    OP_PUSH_BOOL = 31,
    OP_SIN = 32,
    OP_COS = 33,
    OP_TAN = 34,
    OP_HALT = 255
};

// Data types
enum XenoDataType {
    TYPE_INT = 0,
    TYPE_FLOAT = 1,
    TYPE_STRING = 2,
    TYPE_BOOL = 3
};

// Value structure that can hold different data types
struct XenoValue {
    XenoDataType type;
    union {
        int32_t int_val;
        float float_val;
        uint16_t string_index;
        bool bool_val;
    };

    XenoValue();
    
    static XenoValue makeInt(int32_t val);
    static XenoValue makeFloat(float val);
    static XenoValue makeString(uint16_t str_idx);
    static XenoValue makeBool(bool val);
};

// Bytecode instruction structure
struct XenoInstruction {
    uint8_t opcode;
    uint32_t arg1;
    uint16_t arg2;

    explicit XenoInstruction(uint8_t op = OP_NOP,
                         uint32_t a1 = 0,
                         uint16_t a2 = 0);
};

// Structure for storing information about loop
struct LoopInfo {
    String var_name;
    int start_address;
    int condition_address;
    int end_jump_address;
};

#endif  // XENOLANG_CORE_XENO_COMMON_H_