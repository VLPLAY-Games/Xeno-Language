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


#ifndef XENOLANG_CORE_XENO_VM_H_
#define XENOLANG_CORE_XENO_VM_H_

#include <vector>
#include <map>
#include <stack>
#include <cmath>
#include <limits>
#include <memory>
#include <algorithm>
#include "./xeno_common.h"
#include "./xeno_security.h"


// Xeno Virtual Machine
class XenoVM {
 private:
    std::vector<XenoInstruction> program;
    std::vector<String> string_table;
    std::map<String, uint16_t> string_lookup;
    uint32_t program_counter;
    XenoValue stack[MAX_STACK_SIZE];
    uint32_t stack_pointer;
    std::map<String, XenoValue> variables;
    bool running;
    uint32_t instruction_count;
    uint32_t max_instructions;
    uint32_t iteration_count;
    static const uint32_t MAX_ITERATIONS = 100000;
    XenoSecurity security;

    friend class Xeno;

    // Typedef for instruction handler functions
    typedef void (XenoVM::*InstructionHandler)(const XenoInstruction&);

    // Dispatch table for fast instruction execution
    InstructionHandler dispatch_table[256];

    void initializeDispatchTable() {
        // Initialize all to nullptr for safety
        for (int i = 0; i < 256; i++) {
            dispatch_table[i] = nullptr;
        }

        // Map opcodes to handler functions
        dispatch_table[OP_NOP] = &XenoVM::handleNOP;
        dispatch_table[OP_PRINT] = &XenoVM::handlePRINT;
        dispatch_table[OP_LED_ON] = &XenoVM::handleLED_ON;
        dispatch_table[OP_LED_OFF] = &XenoVM::handleLED_OFF;
        dispatch_table[OP_DELAY] = &XenoVM::handleDELAY;
        dispatch_table[OP_PUSH] = &XenoVM::handlePUSH;
        dispatch_table[OP_POP] = &XenoVM::handlePOP;
        dispatch_table[OP_ADD] = &XenoVM::handleADD;
        dispatch_table[OP_SUB] = &XenoVM::handleSUB;
        dispatch_table[OP_MUL] = &XenoVM::handleMUL;
        dispatch_table[OP_DIV] = &XenoVM::handleDIV;
        dispatch_table[OP_JUMP] = &XenoVM::handleJUMP;
        dispatch_table[OP_JUMP_IF] = &XenoVM::handleJUMP_IF;
        dispatch_table[OP_PRINT_NUM] = &XenoVM::handlePRINT_NUM;
        dispatch_table[OP_STORE] = &XenoVM::handleSTORE;
        dispatch_table[OP_LOAD] = &XenoVM::handleLOAD;
        dispatch_table[OP_MOD] = &XenoVM::handleMOD;
        dispatch_table[OP_ABS] = &XenoVM::handleABS;
        dispatch_table[OP_POW] = &XenoVM::handlePOW;
        dispatch_table[OP_MAX] = &XenoVM::handleMAX;
        dispatch_table[OP_MIN] = &XenoVM::handleMIN;
        dispatch_table[OP_SQRT] = &XenoVM::handleSQRT;
        dispatch_table[OP_INPUT] = &XenoVM::handleINPUT;
        dispatch_table[OP_EQ] = &XenoVM::handleEQ;
        dispatch_table[OP_NEQ] = &XenoVM::handleNEQ;
        dispatch_table[OP_LT] = &XenoVM::handleLT;
        dispatch_table[OP_GT] = &XenoVM::handleGT;
        dispatch_table[OP_LTE] = &XenoVM::handleLTE;
        dispatch_table[OP_GTE] = &XenoVM::handleGTE;
        dispatch_table[OP_PUSH_FLOAT] = &XenoVM::handlePUSH_FLOAT;
        dispatch_table[OP_PUSH_STRING] = &XenoVM::handlePUSH_STRING;
        dispatch_table[OP_HALT] = &XenoVM::handleHALT;
    }

    void resetState() {
        program_counter = 0;
        stack_pointer = 0;
        running = false;
        instruction_count = 0;
        iteration_count = 0;
        max_instructions = 10000;
        memset(stack, 0, sizeof(stack));
        variables.clear();
        string_lookup.clear();
    }

    String convertToString(const XenoValue& val) {
        switch (val.type) {
            case TYPE_INT:
                return String(val.int_val);
            case TYPE_FLOAT:
                return String(val.float_val, 3);  // 3 decimal places
            case TYPE_STRING:
                return string_table[val.string_index];
            default:
                return String();
        }
    }

    float toFloat(const XenoValue& v) {
        return (v.type == TYPE_INT) ? static_cast<float>(v.int_val) : v.float_val;
    }

    // Safe stack operations with immediate termination on error
    bool Push(const XenoValue& value) {
        if (stack_pointer >= MAX_STACK_SIZE) {
            Serial.println("CRITICAL ERROR: Stack overflow - terminating execution");
            running = false;
            return false;
        }
        stack[stack_pointer++] = value;
        return true;
    }

    bool Pop(XenoValue& value) {
        if (stack_pointer == 0) {
            Serial.println("CRITICAL ERROR: Stack underflow - terminating execution");
            running = false;
            return false;
        }
        value = stack[--stack_pointer];
        return true;
    }

    bool PopTwo(XenoValue& a, XenoValue& b) {
        if (stack_pointer < 2) {
            Serial.println("CRITICAL ERROR: Stack underflow in binary operation - terminating execution");
            running = false;
            return false;
        }
        b = stack[--stack_pointer];
        a = stack[--stack_pointer];
        return true;
    }

    bool Peek(XenoValue& value) {
        if (stack_pointer == 0) {
            Serial.println("CRITICAL ERROR: Stack underflow in peek - terminating execution");
            running = false;
            return false;
        }
        value = stack[stack_pointer - 1];
        return true;
    }

    // Safe integer operations with overflow checking
    bool Add(int32_t a, int32_t b, int32_t& result) {
        if ((b > 0 && a > std::numeric_limits<int32_t>::max() - b) ||
            (b < 0 && a < std::numeric_limits<int32_t>::min() - b)) {
            Serial.println("ERROR: Integer overflow in addition");
            return false;
        }
        result = a + b;
        return true;
    }

    bool Sub(int32_t a, int32_t b, int32_t& result) {
        if ((b > 0 && a < std::numeric_limits<int32_t>::min() + b) ||
            (b < 0 && a > std::numeric_limits<int32_t>::max() + b)) {
            Serial.println("ERROR: Integer overflow in subtraction");
            return false;
        }
        result = a - b;
        return true;
    }

    bool Mul(int32_t a, int32_t b, int32_t& result) {
        if (a == 0 || b == 0) {
            result = 0;
            return true;
        }

        if (a > 0) {
            if (b > 0) {
                if (a > std::numeric_limits<int32_t>::max() / b) return false;
            } else {
                if (b < std::numeric_limits<int32_t>::min() / a) return false;
            }
        } else {
            if (b > 0) {
                if (a < std::numeric_limits<int32_t>::min() / b) return false;
            } else {
                if (a < std::numeric_limits<int32_t>::max() / b) return false;
            }
        }

        result = a * b;
        return true;
    }

    bool Pow(int32_t base, int32_t exponent, int32_t& result) {
        if (exponent < 0) return false;
        if (exponent == 0) {
            result = 1;
            return true;
        }
        if (base == 0) {
            result = 0;
            return true;
        }

        result = 1;
        for (int32_t i = 0; i < exponent; ++i) {
            if (!Mul(result, base, result)) {
                Serial.println("ERROR: Integer overflow in power operation");
                return false;
            }
        }
        return true;
    }

    bool Mod(int32_t a, int32_t b, int32_t& result) {
        if (b == 0) {
            Serial.println("ERROR: Modulo by zero");
            return false;
        }

        // Handle special case that can cause overflow
        if (a == std::numeric_limits<int32_t>::min() && b == -1) {
            result = 0;
            return true;
        }

        result = a % b;
        return true;
    }

    // Safe square root with validation
    XenoValue Sqrt(const XenoValue& a) {
        if (a.type == TYPE_INT) {
            if (a.int_val < 0) {
                Serial.println("ERROR: Square root of negative number");
                return XenoValue::makeInt(0);
            }
            return XenoValue::makeFloat(sqrt(static_cast<float>(a.int_val)));
        } else if (a.type == TYPE_FLOAT) {
            if (a.float_val < 0) {
                Serial.println("ERROR: Square root of negative number");
                return XenoValue::makeFloat(0.0f);
            }
            return XenoValue::makeFloat(sqrt(a.float_val));
        }
        return XenoValue::makeInt(0);
    }

    // Safe max operation
    XenoValue Max(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);
                return XenoValue::makeFloat(max(a_val, b_val));
            } else {
                return XenoValue::makeInt(max(a.int_val, b.int_val));
            }
        }
        return XenoValue::makeInt(0);
    }

    // Safe min operation
    XenoValue Min(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);

                return XenoValue::makeFloat(min(a_val, b_val));
            } else {
                return XenoValue::makeInt(min(a.int_val, b.int_val));
            }
        }
        return XenoValue::makeInt(0);
    }

    // Helper functions for type conversion and operations
    XenoValue convertToFloat(const XenoValue& val) {
        if (val.type == TYPE_FLOAT) return val;
        if (val.type == TYPE_INT) {
            return XenoValue::makeFloat(static_cast<float>(val.int_val));
        }
        return XenoValue::makeFloat(0.0f);
    }

    bool bothNumeric(const XenoValue& a, const XenoValue& b) {
        return (a.type == TYPE_INT || a.type == TYPE_FLOAT) &&
               (b.type == TYPE_INT || b.type == TYPE_FLOAT);
    }

    XenoValue performAddition(const XenoValue& a, const XenoValue& b) {
        if (a.type == TYPE_STRING || b.type == TYPE_STRING) {
            String str_a = convertToString(a);
            String str_b = convertToString(b);
            String combined = str_a + str_b;
            uint16_t combined_index = addString(combined);
            return XenoValue::makeString(combined_index);
        }

        // Иначе числовое сложение
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);
                return XenoValue::makeFloat(a_val + b_val);
            } else {
                int32_t result;
                if (Add(a.int_val, b.int_val, result)) {
                    return XenoValue::makeInt(result);
                } else {
                    return XenoValue::makeInt(0);
                }
            }
        }

        return XenoValue::makeInt(0);
    }

    XenoValue performSubtraction(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);
                return XenoValue::makeFloat(a_val - b_val);
            } else {
                int32_t result;
                if (Sub(a.int_val, b.int_val, result)) {
                    return XenoValue::makeInt(result);
                } else {
                    return XenoValue::makeInt(0);
                }
            }
        }
        return XenoValue::makeInt(0);
    }

    XenoValue performMultiplication(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);
                return XenoValue::makeFloat(a_val * b_val);
            } else {
                int32_t result;
                if (Mul(a.int_val, b.int_val, result)) {
                    return XenoValue::makeInt(result);
                } else {
                    return XenoValue::makeInt(0);
                }
            }
        }
        return XenoValue::makeInt(0);
    }

    XenoValue performDivision(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);

                if (b_val != 0.0f) {
                    return XenoValue::makeFloat(a_val / b_val);
                }
                Serial.println("ERROR: Division by zero");
                return XenoValue::makeFloat(0.0f);
            } else {
                if (b.int_val != 0) {
                    if (a.int_val == std::numeric_limits<int32_t>::min() && b.int_val == -1) {
                        Serial.println("ERROR: Integer overflow in division");
                        return XenoValue::makeInt(0);
                    }
                    return XenoValue::makeInt(a.int_val / b.int_val);
                } else {
                    Serial.println("ERROR: Division by zero");
                    return XenoValue::makeInt(0);
                }
            }
        }
        return XenoValue::makeInt(0);
    }

    XenoValue performModulo(const XenoValue& a, const XenoValue& b) {
        if (a.type == TYPE_INT && b.type == TYPE_INT) {
            int32_t result;
            if (Mod(a.int_val, b.int_val, result)) {
                return XenoValue::makeInt(result);
            } else {
                return XenoValue::makeInt(0);
            }
        } else {
            Serial.println("ERROR: Modulo requires integer operands");
            return XenoValue::makeInt(0);
        }
    }

    XenoValue performPower(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);
                return XenoValue::makeFloat(pow(a_val, b_val));
            } else {
                int32_t result;
                if (Pow(a.int_val, b.int_val, result)) {
                    return XenoValue::makeInt(result);
                } else {
                    return XenoValue::makeInt(0);
                }
            }
        }
        return XenoValue::makeInt(0);
    }

    XenoValue performAbs(const XenoValue& a) {
        if (a.type == TYPE_INT) {
            if (a.int_val == std::numeric_limits<int32_t>::min()) {
                Serial.println("ERROR: Integer overflow in absolute value");
                return XenoValue::makeInt(std::numeric_limits<int32_t>::max());
            }
            return XenoValue::makeInt(abs(a.int_val));
        } else if (a.type == TYPE_FLOAT) {
            return XenoValue::makeFloat(fabs(a.float_val));
        }
        return XenoValue::makeInt(0);
    }

    bool performComparison(const XenoValue& a, const XenoValue& b, uint8_t op) {
        if (a.type != b.type) {
            if (bothNumeric(a, b)) {
                float a_val = toFloat(a);
                float b_val = toFloat(b);

                switch (op) {
                    case OP_EQ: return a_val == b_val;
                    case OP_NEQ: return a_val != b_val;
                    case OP_LT: return a_val < b_val;
                    case OP_GT: return a_val > b_val;
                    case OP_LTE: return a_val <= b_val;
                    case OP_GTE: return a_val >= b_val;
                }
            }
            return false;
        }

        switch (a.type) {
            case TYPE_INT:
                switch (op) {
                    case OP_EQ: return a.int_val == b.int_val;
                    case OP_NEQ: return a.int_val != b.int_val;
                    case OP_LT: return a.int_val < b.int_val;
                    case OP_GT: return a.int_val > b.int_val;
                    case OP_LTE: return a.int_val <= b.int_val;
                    case OP_GTE: return a.int_val >= b.int_val;
                }
                break;

            case TYPE_FLOAT:
                switch (op) {
                    case OP_EQ: return a.float_val == b.float_val;
                    case OP_NEQ: return a.float_val != b.float_val;
                    case OP_LT: return a.float_val < b.float_val;
                    case OP_GT: return a.float_val > b.float_val;
                    case OP_LTE: return a.float_val <= b.float_val;
                    case OP_GTE: return a.float_val >= b.float_val;
                }
                break;

            case TYPE_STRING:
                {
                    const String& str_a = string_table[a.string_index];
                    const String& str_b = string_table[b.string_index];
                    switch (op) {
                        case OP_EQ: return str_a == str_b;
                        case OP_NEQ: return str_a != str_b;
                        case OP_LT: return str_a < str_b;
                        case OP_GT: return str_a > str_b;
                        case OP_LTE: return str_a <= str_b;
                        case OP_GTE: return str_a >= str_b;
                    }
                }
                break;
        }
        return false;
    }

    // Optimized string addition with lookup table
    uint16_t addString(const String& str) {
        // Sanitize input string first
        String safe_str = security.sanitizeString(str);

        // Check lookup first
        auto it = string_lookup.find(safe_str);
        if (it != string_lookup.end()) {
            return it->second;
        }

        // Not in lookup, add to string table
        for (size_t i = 0; i < string_table.size(); ++i) {
            if (string_table[i] == safe_str) {
                string_lookup[safe_str] = i;
                return i;
            }
        }

        if (string_table.size() >= 65535) {
            Serial.println("ERROR: String table overflow");
            return 0;
        }

        string_table.push_back(safe_str);
        uint16_t new_index = string_table.size() - 1;
        string_lookup[safe_str] = new_index;
        return new_index;
    }

    // Fast instruction handlers
    void handleNOP(const XenoInstruction& instr) { /* Do nothing */ }

    void handlePRINT(const XenoInstruction& instr) {
        if (instr.arg1 < string_table.size()) {
            Serial.println(string_table[instr.arg1]);
        } else {
            Serial.println("ERROR: Invalid string index");
        }
    }

    void handleLED_ON(const XenoInstruction& instr) {
        if (!security.isPinAllowed(instr.arg1)) {
            Serial.print("ERROR: Pin not allowed: ");
            Serial.println(instr.arg1);
            return;
        }
        pinMode(instr.arg1, OUTPUT);
        digitalWrite(instr.arg1, HIGH);
        Serial.print("LED ON pin ");
        Serial.println(instr.arg1);
    }

    void handleLED_OFF(const XenoInstruction& instr) {
        if (!security.isPinAllowed(instr.arg1)) {
            Serial.print("ERROR: Pin not allowed: ");
            Serial.println(instr.arg1);
            return;
        }
        pinMode(instr.arg1, OUTPUT);
        digitalWrite(instr.arg1, LOW);
        Serial.print("LED OFF pin ");
        Serial.println(instr.arg1);
    }

    void handleDELAY(const XenoInstruction& instr) {
        delay(instr.arg1);
    }

    void handlePUSH(const XenoInstruction& instr) {
        if (!Push(XenoValue::makeInt(instr.arg1))) return;
    }

    void handlePUSH_FLOAT(const XenoInstruction& instr) {
        float fval;
        memcpy(&fval, &instr.arg1, sizeof(float));
        if (!Push(XenoValue::makeFloat(fval))) return;
    }

    void handlePUSH_STRING(const XenoInstruction& instr) {
        if (!Push(XenoValue::makeString(instr.arg1))) return;
    }

    void handlePOP(const XenoInstruction& instr) {
        XenoValue temp;
        if (!Pop(temp)) return;
    }

    void handleADD(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(performAddition(a, b))) return;
    }

    void handleSUB(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(performSubtraction(a, b))) return;
    }

    void handleMUL(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(performMultiplication(a, b))) return;
    }

    void handleDIV(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(performDivision(a, b))) return;
    }

    void handleMOD(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(performModulo(a, b))) return;
    }

    void handleABS(const XenoInstruction& instr) {
        XenoValue a;
        if (!Peek(a)) return;
        stack[stack_pointer - 1] = performAbs(a);
    }

    void handlePOW(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(performPower(a, b))) return;
    }

    void handleMAX(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(Max(a, b))) return;
    }

    void handleMIN(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(Min(a, b))) return;
    }

    void handleSQRT(const XenoInstruction& instr) {
        XenoValue a;
        if (!Peek(a)) return;
        stack[stack_pointer - 1] = Sqrt(a);
    }

    void handleINPUT(const XenoInstruction& instr) {
        if (instr.arg1 >= string_table.size()) {
            Serial.println("ERROR: Invalid variable name index in INPUT");
            running = false;
            return;
        }

        String var_name = string_table[instr.arg1];
        Serial.print("INPUT ");
        Serial.print(var_name);
        Serial.print(": ");

        // Wait for input with timeout
        unsigned long startTime = millis();
        String input_str = "";

        while (millis() - startTime < 30000) {  // 30 second timeout
            if (Serial.available() > 0) {
                input_str = Serial.readString();
                input_str.trim();
                break;
            }
            delay(100);
        }

        if (input_str.isEmpty()) {
            Serial.println("TIMEOUT - using default value 0");
            variables[var_name] = XenoValue::makeInt(0);
            return;
        }

        // Determine input type and convert
        XenoValue input_value;
        if (isInteger(input_str)) {
            input_value = XenoValue::makeInt(input_str.toInt());
        } else if (isFloat(input_str)) {
            input_value = XenoValue::makeFloat(input_str.toFloat());
        } else {
            // Treat as string
            input_value = XenoValue::makeString(addString(input_str));
        }

        variables[var_name] = input_value;
        Serial.print("-> ");
        Serial.println(input_str);
    }

    // Helper functions for input parsing
    bool isInteger(const String& str) {
        if (str.isEmpty()) return false;
        const char* cstr = str.c_str();
        size_t start = 0;
        if (cstr[0] == '-') start = 1;
        for (size_t i = start; i < str.length(); ++i) {
            if (!isdigit(cstr[i])) return false;
        }
        return true;
    }

    bool isFloat(const String& str) {
        if (str.isEmpty()) return false;
        const char* cstr = str.c_str();
        bool has_decimal = false;
        size_t start = 0;
        if (cstr[0] == '-') start = 1;
        for (size_t i = start; i < str.length(); ++i) {
            if (cstr[i] == '.') {
                if (has_decimal) return false;
                has_decimal = true;
            } else if (!isdigit(cstr[i])) {
                return false;
            }
        }
        return has_decimal;
    }

    void handleEQ(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(XenoValue::makeInt(performComparison(a, b, OP_EQ) ? 0 : 1))) return;
    }

    void handleNEQ(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(XenoValue::makeInt(performComparison(a, b, OP_NEQ) ? 0 : 1))) return;
    }

    void handleLT(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(XenoValue::makeInt(performComparison(a, b, OP_LT) ? 0 : 1))) return;
    }

    void handleGT(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(XenoValue::makeInt(performComparison(a, b, OP_GT) ? 0 : 1))) return;
    }

    void handleLTE(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(XenoValue::makeInt(performComparison(a, b, OP_LTE) ? 0 : 1))) return;
    }

    void handleGTE(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!PopTwo(a, b)) return;
        if (!Push(XenoValue::makeInt(performComparison(a, b, OP_GTE) ? 0 : 1))) return;
    }

    void handlePRINT_NUM(const XenoInstruction& instr) {
        XenoValue val;
        if (!Peek(val)) return;
        switch (val.type) {
            case TYPE_INT: Serial.println(val.int_val); break;
            case TYPE_FLOAT: Serial.println(val.float_val, 2); break;
            case TYPE_STRING: Serial.println(string_table[val.string_index]); break;
        }
    }

    void handleSTORE(const XenoInstruction& instr) {
        if (instr.arg1 >= string_table.size()) {
            Serial.println("ERROR: Invalid variable name index in STORE");
            running = false;
            return;
        }
        XenoValue value;
        if (!Pop(value)) return;
        String var_name = string_table[instr.arg1];
        variables[var_name] = value;
    }

    void handleLOAD(const XenoInstruction& instr) {
        if (instr.arg1 >= string_table.size()) {
            Serial.println("ERROR: Invalid variable name index in LOAD");
            running = false;
            return;
        }
        String var_name = string_table[instr.arg1];
        auto it = variables.find(var_name);
        if (it != variables.end()) {
            if (!Push(it->second)) return;
        } else {
            Serial.print("ERROR: Variable not found: ");
            Serial.println(var_name);
            if (!Push(XenoValue::makeInt(0))) return;
        }
    }

    void handleJUMP(const XenoInstruction& instr) {
        if (instr.arg1 < program.size()) {
            program_counter = instr.arg1;
        } else {
            Serial.println("ERROR: Jump to invalid address");
            running = false;
            return;
        }
    }

    void handleJUMP_IF(const XenoInstruction& instr) {
        XenoValue condition_val;
        if (!Pop(condition_val)) return;

        int condition = 0;
        switch (condition_val.type) {
            case TYPE_INT: condition = (condition_val.int_val != 0); break;
            case TYPE_FLOAT: condition = (condition_val.float_val != 0.0f); break;
            case TYPE_STRING: condition = !string_table[condition_val.string_index].isEmpty(); break;
        }

        if (condition && instr.arg1 < program.size()) {
            program_counter = instr.arg1;
        }
    }

    void handleHALT(const XenoInstruction& instr) {
        running = false;
    }

 protected:
    static constexpr const char* xeno_vm_name = "Xeno Virtual Machine";
    static constexpr const char* xeno_vm_version = "v0.1.0";
    static constexpr const char* xeno_vm_date = "26.10.2025";

    XenoVM() {
        initializeDispatchTable();
        resetState();
        program.reserve(128);
        string_table.reserve(32);
    }

    void setMaxInstructions(uint32_t max_instr) {
        max_instructions = max_instr;
    }

    void loadProgram(const std::vector<XenoInstruction>& bytecode,
                    const std::vector<String>& strings) {
        resetState();

        // Sanitize all input strings first
        std::vector<String> sanitized_strings;
        sanitized_strings.reserve(strings.size());
        for (const String& str : strings) {
            sanitized_strings.push_back(security.sanitizeString(str));
        }

        // Verify bytecode integrity before loading
        if (!security.verifyBytecode(bytecode, sanitized_strings)) {
            Serial.println("SECURITY: Bytecode verification failed - refusing to load");
            running = false;
            return;
        }

        program = bytecode;
        string_table = sanitized_strings;

        // Pre-populate lookup with initial strings
        for (size_t i = 0; i < string_table.size(); ++i) {
            string_lookup[string_table[i]] = i;
        }

        running = true;
        Serial.println("Program loaded and verified successfully");
    }

    bool step() {
        if (!running || program_counter >= program.size()) {
            return false;
        }

        // Check iteration limit to prevent infinite loops
        if (++iteration_count > MAX_ITERATIONS) {
            Serial.println("ERROR: Iteration limit exceeded - possible infinite loop");
            running = false;
            return false;
        }

        const XenoInstruction& instr = program[program_counter++];

        // Fast dispatch using function pointer table
        InstructionHandler handler = dispatch_table[instr.opcode];
        if (handler != nullptr) {
            (this->*handler)(instr);
        } else {
            Serial.print("ERROR: Unknown instruction ");
            Serial.println(instr.opcode);
            running = false;
            return false;
        }

        instruction_count++;
        if (instruction_count > max_instructions) {
            Serial.println("ERROR: Instruction limit exceeded - possible infinite loop");
            running = false;
            return false;
        }

        return running;
    }

    void run() {
        Serial.println("Starting Xeno VM...");

        while (step()) {
            // Continue execution
        }

        Serial.println("Xeno VM finished");
    }

    void stop() {
        running = false;
        program_counter = 0;
        stack_pointer = 0;
    }

    bool isRunning() const { return running; }
    uint32_t getPC() const { return program_counter; }
    uint32_t getSP() const { return stack_pointer; }
    uint32_t getInstructionCount() const { return instruction_count; }
    uint32_t getIterationCount() const { return iteration_count; }

    void dumpState() {
        Serial.println("=== VM State ===");

        Serial.print("Program Counter: ");
        Serial.println(program_counter);

        Serial.print("Stack Pointer: ");
        Serial.println(stack_pointer);

        Serial.println("Stack: [");

        for (int i = 0; i < stack_pointer && i < 10; ++i) {
            String type_str;
            String value_str;
            switch (stack[i].type) {
                case TYPE_INT:
                    type_str = "INT";
                    value_str = String(stack[i].int_val);
                    break;
                case TYPE_FLOAT:
                    type_str = "FLOAT";
                    value_str = String(stack[i].float_val, 4);
                    break;
                case TYPE_STRING:
                    type_str = "STRING";
                    value_str = "\"" + string_table[stack[i].string_index] + "\"";
                    break;
            }
            Serial.print("  ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(type_str);
            Serial.print(" ");
            Serial.println(value_str);
        }
        if (stack_pointer > 10) Serial.println("  ...");
        Serial.println("]");

        Serial.println("Variables: {");
        for (const auto& var : variables) {
            String type_str;
            String value_str;
            switch (var.second.type) {
                case TYPE_INT:
                    type_str = "INT";
                    value_str = String(var.second.int_val);
                    break;
                case TYPE_FLOAT:
                    type_str = "FLOAT";
                    value_str = String(var.second.float_val, 4);
                    break;
                case TYPE_STRING:
                    type_str = "STRING";
                    value_str = "\"" + string_table[var.second.string_index] + "\"";
                    break;
            }
            Serial.print("  ");
            Serial.print(var.first);
            Serial.print(": ");
            Serial.print(type_str);
            Serial.print(" ");
            Serial.println(value_str);
        }
        Serial.println("}");
    }

    void disassemble() {
        Serial.println("=== Disassembly ===");
        for (size_t i = 0; i < program.size(); ++i) {
            const XenoInstruction& instr = program[i];
            Serial.print(i);
            Serial.print(": ");

            switch (instr.opcode) {
                case OP_NOP: Serial.println("NOP"); break;
                case OP_PRINT:
                    Serial.print("PRINT ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.print("\"");
                        Serial.print(string_table[instr.arg1]);
                        Serial.print("\"");
                    } else {
                        Serial.print("<invalid string>");
                    }
                    Serial.println();
                    break;
                case OP_LED_ON:
                    Serial.print("LED_ON pin=");
                    Serial.println(instr.arg1);
                    break;
                case OP_LED_OFF:
                    Serial.print("LED_OFF pin=");
                    Serial.println(instr.arg1);
                    break;
                case OP_DELAY:
                    Serial.print("DELAY ");
                    Serial.print(instr.arg1);
                    Serial.println("ms");
                    break;
                case OP_PUSH:
                    Serial.print("PUSH ");
                    Serial.println(instr.arg1);
                    break;
                case OP_PUSH_FLOAT: {
                    float fval;
                    memcpy(&fval, &instr.arg1, sizeof(float));
                    Serial.print("PUSH_FLOAT ");
                    Serial.println(fval, 4);
                    break;
                }
                case OP_PUSH_STRING:
                    Serial.print("PUSH_STRING ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.print("\"");
                        Serial.print(string_table[instr.arg1]);
                        Serial.print("\"");
                    } else {
                        Serial.print("<invalid>");
                    }
                    Serial.println();
                    break;
                case OP_POP: Serial.println("POP"); break;
                case OP_ADD: Serial.println("ADD"); break;
                case OP_SUB: Serial.println("SUB"); break;
                case OP_MUL: Serial.println("MUL"); break;
                case OP_DIV: Serial.println("DIV"); break;
                case OP_MOD: Serial.println("MOD"); break;
                case OP_ABS: Serial.println("ABS"); break;
                case OP_POW: Serial.println("POW"); break;
                case OP_MAX: Serial.println("MAX"); break;
                case OP_MIN: Serial.println("MIN"); break;
                case OP_SQRT: Serial.println("SQRT"); break;
                case OP_INPUT:
                    Serial.print("INPUT ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.print(string_table[instr.arg1]);
                    } else {
                        Serial.print("<invalid var>");
                    }
                    Serial.println();
                    break;
                case OP_EQ: Serial.println("EQ"); break;
                case OP_NEQ: Serial.println("NEQ"); break;
                case OP_LT: Serial.println("LT"); break;
                case OP_GT: Serial.println("GT"); break;
                case OP_LTE: Serial.println("LTE"); break;
                case OP_GTE: Serial.println("GTE"); break;
                case OP_PRINT_NUM: Serial.println("PRINT_NUM"); break;
                case OP_STORE:
                    Serial.print("STORE ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.print(string_table[instr.arg1]);
                    } else {
                        Serial.print("<invalid var>");
                    }
                    Serial.println();
                    break;
                case OP_LOAD:
                    Serial.print("LOAD ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.print(string_table[instr.arg1]);
                    } else {
                        Serial.print("<invalid var>");
                    }
                    Serial.println();
                    break;
                case OP_JUMP:
                    Serial.print("JUMP ");
                    Serial.println(instr.arg1);
                    break;
                case OP_JUMP_IF:
                    Serial.print("JUMP_IF ");
                    Serial.println(instr.arg1);
                    break;
                case OP_HALT: Serial.println("HALT"); break;
                default:
                    Serial.print("UNKNOWN ");
                    Serial.println(instr.opcode);
                    break;
            }
        }
    }
};

#endif  // XENOLANG_CORE_XENO_VM_H_
