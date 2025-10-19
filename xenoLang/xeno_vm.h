#ifndef XENO_VM_H
#define XENO_VM_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <stack>
#include <cmath>
#include <limits>
#include <memory>

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

// Allowed pins for safety
const uint8_t ALLOWED_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, LED_BUILTIN};
const size_t NUM_ALLOWED_PINS = sizeof(ALLOWED_PINS) / sizeof(ALLOWED_PINS[0]);

bool isPinAllowed(uint8_t pin) {
    for (size_t i = 0; i < NUM_ALLOWED_PINS; i++) {
        if (pin == ALLOWED_PINS[i]) {
            return true;
        }
    }
    return false;
}

// Xeno Virtual Machine
class XenoVM {
private:
    std::vector<XenoInstruction> program;
    std::vector<String> string_table;
    std::map<String, uint16_t> string_lookup;
    uint32_t program_counter;
    XenoValue stack[64];
    uint32_t stack_pointer;
    std::map<String, XenoValue> variables;
    bool running;
    uint32_t instruction_count;
    uint32_t max_instructions;
    uint32_t iteration_count;
    static const uint32_t MAX_ITERATIONS = 100000;
    
    // Typedef for instruction handler functions
    typedef void (XenoVM::*InstructionHandler)(const XenoInstruction&);
    
    // Dispatch table for fast instruction execution
    InstructionHandler dispatch_table[256];
    
    // String sanitization for security
    String sanitizeString(const String& input) {
        String sanitized;
        sanitized.reserve(input.length());
        
        for (size_t i = 0; i < input.length(); i++) {
            char c = input[i];
            
            // Allow only safe printable ASCII characters
            if (c >= 32 && c <= 126) {
                // Escape potentially dangerous characters
                if (c == '\\' || c == '"' || c == '\'' || c == '`') {
                    sanitized += '\\';
                }
                sanitized += c;
            }
            // Allow basic whitespace
            else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                sanitized += c;
            }
            // Replace other characters with safe equivalent
            else {
                sanitized += '?';
            }
            
            // Limit maximum string length for safety
            if (sanitized.length() >= 256) {
                sanitized += "...";
                break;
            }
        }
        
        return sanitized;
    }
    
    // Bytecode verification for security
    bool verifyBytecode(const std::vector<XenoInstruction>& bytecode, 
                       const std::vector<String>& strings) {
        // Check program size limits
        if (bytecode.size() > 10000) {
            Serial.println("SECURITY: Program too large");
            return false;
        }
        
        // Check string table size limits
        if (strings.size() > 1000) {
            Serial.println("SECURITY: String table too large");
            return false;
        }
        
        // Verify each instruction
        for (size_t i = 0; i < bytecode.size(); i++) {
            const XenoInstruction& instr = bytecode[i];
            
            // Check for valid opcode range
            if (instr.opcode > 26 && instr.opcode != 255) {
                Serial.println("SECURITY: Invalid opcode at instruction " + String(i));
                return false;
            }
            
            // Verify jump targets are within program bounds
            if (instr.opcode == OP_JUMP || instr.opcode == OP_JUMP_IF) {
                if (instr.arg1 >= bytecode.size()) {
                    Serial.println("SECURITY: Invalid jump target at instruction " + String(i));
                    return false;
                }
            }
            
            // Verify string indices are within string table bounds
            if (instr.opcode == OP_PRINT || instr.opcode == OP_STORE || 
                instr.opcode == OP_LOAD || instr.opcode == OP_PUSH_STRING) {
                if (instr.arg1 >= strings.size()) {
                    Serial.println("SECURITY: Invalid string index at instruction " + String(i));
                    return false;
                }
            }
            
            // Verify pin numbers are allowed
            if (instr.opcode == OP_LED_ON || instr.opcode == OP_LED_OFF) {
                if (!isPinAllowed(instr.arg1)) {
                    Serial.println("SECURITY: Unauthorized pin access at instruction " + String(i));
                    return false;
                }
            }
            
            // Verify delay values are reasonable
            if (instr.opcode == OP_DELAY) {
                if (instr.arg1 > 60000) { // Max 60 seconds
                    Serial.println("SECURITY: Excessive delay at instruction " + String(i));
                    return false;
                }
            }
        }
        
        // Verify no infinite loops without conditions
        bool has_halt = false;
        for (const auto& instr : bytecode) {
            if (instr.opcode == OP_HALT) {
                has_halt = true;
                break;
            }
        }
        
        if (!has_halt && bytecode.size() > 10) {
            Serial.println("SECURITY: Program missing HALT instruction");
            return false;
        }
        
        return true;
    }
    
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
    
    // Safe stack operations with immediate termination on error
    bool safePush(const XenoValue& value) {
        if (stack_pointer >= 64) {
            Serial.println("CRITICAL ERROR: Stack overflow - terminating execution");
            running = false;
            return false;
        }
        stack[stack_pointer++] = value;
        return true;
    }
    
    bool safePop(XenoValue& value) {
        if (stack_pointer == 0) {
            Serial.println("CRITICAL ERROR: Stack underflow - terminating execution");
            running = false;
            return false;
        }
        value = stack[--stack_pointer];
        return true;
    }
    
    bool safePopTwo(XenoValue& a, XenoValue& b) {
        if (stack_pointer < 2) {
            Serial.println("CRITICAL ERROR: Stack underflow in binary operation - terminating execution");
            running = false;
            return false;
        }
        b = stack[--stack_pointer];
        a = stack[--stack_pointer];
        return true;
    }
    
    bool safePeek(XenoValue& value) {
        if (stack_pointer == 0) {
            Serial.println("CRITICAL ERROR: Stack underflow in peek - terminating execution");
            running = false;
            return false;
        }
        value = stack[stack_pointer - 1];
        return true;
    }
    
    // Safe integer operations with overflow checking
    bool safeAdd(int32_t a, int32_t b, int32_t& result) {
        if ((b > 0 && a > std::numeric_limits<int32_t>::max() - b) ||
            (b < 0 && a < std::numeric_limits<int32_t>::min() - b)) {
            Serial.println("ERROR: Integer overflow in addition");
            return false;
        }
        result = a + b;
        return true;
    }
    
    bool safeSub(int32_t a, int32_t b, int32_t& result) {
        if ((b > 0 && a < std::numeric_limits<int32_t>::min() + b) ||
            (b < 0 && a > std::numeric_limits<int32_t>::max() + b)) {
            Serial.println("ERROR: Integer overflow in subtraction");
            return false;
        }
        result = a - b;
        return true;
    }
    
    bool safeMul(int32_t a, int32_t b, int32_t& result) {
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
    
    bool safePow(int32_t base, int32_t exponent, int32_t& result) {
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
            if (!safeMul(result, base, result)) {
                Serial.println("ERROR: Integer overflow in power operation");
                return false;
            }
        }
        return true;
    }

    bool safeMod(int32_t a, int32_t b, int32_t& result) {
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
        // String concatenation
        if (a.type == TYPE_STRING && b.type == TYPE_STRING) {
            String combined = string_table[a.string_index] + string_table[b.string_index];
            uint16_t combined_index = addString(combined);
            return XenoValue::makeString(combined_index);
        }
        
        // Numeric addition
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(a_val + b_val);
            } else {
                int32_t result;
                if (safeAdd(a.int_val, b.int_val, result)) {
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
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(a_val - b_val);
            } else {
                int32_t result;
                if (safeSub(a.int_val, b.int_val, result)) {
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
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(a_val * b_val);
            } else {
                int32_t result;
                if (safeMul(a.int_val, b.int_val, result)) {
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
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                
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
            if (safeMod(a.int_val, b.int_val, result)) {
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
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(pow(a_val, b_val));
            } else {
                int32_t result;
                if (safePow(a.int_val, b.int_val, result)) {
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
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                
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
        String safe_str = sanitizeString(str);
        
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
        if (!isPinAllowed(instr.arg1)) {
            Serial.println("ERROR: Pin not allowed: " + String(instr.arg1));
            return;
        }
        pinMode(instr.arg1, OUTPUT);
        digitalWrite(instr.arg1, HIGH);
        Serial.println("LED ON pin " + String(instr.arg1));
    }
    
    void handleLED_OFF(const XenoInstruction& instr) {
        if (!isPinAllowed(instr.arg1)) {
            Serial.println("ERROR: Pin not allowed: " + String(instr.arg1));
            return;
        }
        pinMode(instr.arg1, OUTPUT);
        digitalWrite(instr.arg1, LOW);
        Serial.println("LED OFF pin " + String(instr.arg1));
    }
    
    void handleDELAY(const XenoInstruction& instr) {
        delay(instr.arg1);
    }
    
    void handlePUSH(const XenoInstruction& instr) {
        if (!safePush(XenoValue::makeInt(instr.arg1))) return;
    }
    
    void handlePUSH_FLOAT(const XenoInstruction& instr) {
        float fval;
        memcpy(&fval, &instr.arg1, sizeof(float));
        if (!safePush(XenoValue::makeFloat(fval))) return;
    }
    
    void handlePUSH_STRING(const XenoInstruction& instr) {
        if (!safePush(XenoValue::makeString(instr.arg1))) return;
    }
    
    void handlePOP(const XenoInstruction& instr) {
        XenoValue temp;
        if (!safePop(temp)) return;
    }
    
    void handleADD(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(performAddition(a, b))) return;
    }
    
    void handleSUB(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(performSubtraction(a, b))) return;
    }
    
    void handleMUL(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(performMultiplication(a, b))) return;
    }
    
    void handleDIV(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(performDivision(a, b))) return;
    }
    
    void handleMOD(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(performModulo(a, b))) return;
    }
    
    void handleABS(const XenoInstruction& instr) {
        XenoValue a;
        if (!safePeek(a)) return;
        stack[stack_pointer - 1] = performAbs(a);
    }
    
    void handlePOW(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(performPower(a, b))) return;
    }
    
    void handleEQ(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(XenoValue::makeInt(performComparison(a, b, OP_EQ) ? 0 : 1))) return;
    }
    
    void handleNEQ(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(XenoValue::makeInt(performComparison(a, b, OP_NEQ) ? 0 : 1))) return;
    }
    
    void handleLT(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(XenoValue::makeInt(performComparison(a, b, OP_LT) ? 0 : 1))) return;
    }
    
    void handleGT(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(XenoValue::makeInt(performComparison(a, b, OP_GT) ? 0 : 1))) return;
    }
    
    void handleLTE(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(XenoValue::makeInt(performComparison(a, b, OP_LTE) ? 0 : 1))) return;
    }
    
    void handleGTE(const XenoInstruction& instr) {
        XenoValue a, b;
        if (!safePopTwo(a, b)) return;
        if (!safePush(XenoValue::makeInt(performComparison(a, b, OP_GTE) ? 0 : 1))) return;
    }
    
    void handlePRINT_NUM(const XenoInstruction& instr) {
        XenoValue val;
        if (!safePeek(val)) return;
        switch (val.type) {
            case TYPE_INT: Serial.println(String(val.int_val)); break;
            case TYPE_FLOAT: Serial.println(String(val.float_val, 2)); break;
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
        if (!safePop(value)) return;
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
            if (!safePush(it->second)) return;
        } else {
            Serial.println("ERROR: Variable not found: " + var_name);
            if (!safePush(XenoValue::makeInt(0))) return;
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
        if (!safePop(condition_val)) return;
        
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
    
public:
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
            sanitized_strings.push_back(sanitizeString(str));
        }
        
        // Verify bytecode integrity before loading
        if (!verifyBytecode(bytecode, sanitized_strings)) {
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
        Serial.println("SECURITY: Program loaded and verified successfully");
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
            Serial.println("ERROR: Unknown instruction " + String(instr.opcode));
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
        Serial.println("Program Counter: " + String(program_counter));
        Serial.println("Stack Pointer: " + String(stack_pointer));
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
            Serial.println("  " + String(i) + ": " + type_str + " " + value_str);
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
            Serial.println("  " + var.first + ": " + type_str + " " + value_str);
        }
        Serial.println("}");
    }
    
    void disassemble() {
        Serial.println("=== Disassembly ===");
        for (size_t i = 0; i < program.size(); ++i) {
            const XenoInstruction& instr = program[i];
            Serial.print(String(i) + ": ");
            
            switch (instr.opcode) {
                case OP_NOP: Serial.println("NOP"); break;
                case OP_PRINT: 
                    Serial.println(instr.arg1 < string_table.size() ? 
                        "PRINT \"" + string_table[instr.arg1] + "\"" : "PRINT <invalid string>");
                    break;
                case OP_LED_ON: Serial.println("LED_ON pin=" + String(instr.arg1)); break;
                case OP_LED_OFF: Serial.println("LED_OFF pin=" + String(instr.arg1)); break;
                case OP_DELAY: Serial.println("DELAY " + String(instr.arg1) + "ms"); break;
                case OP_PUSH: Serial.println("PUSH " + String(instr.arg1)); break;
                case OP_PUSH_FLOAT: {
                    float fval;
                    memcpy(&fval, &instr.arg1, sizeof(float));
                    Serial.println("PUSH_FLOAT " + String(fval, 4));
                    break;
                }
                case OP_PUSH_STRING:
                    Serial.println(instr.arg1 < string_table.size() ? 
                        "PUSH_STRING \"" + string_table[instr.arg1] + "\"" : "PUSH_STRING <invalid>");
                    break;
                case OP_POP: Serial.println("POP"); break;
                case OP_ADD: Serial.println("ADD"); break;
                case OP_SUB: Serial.println("SUB"); break;
                case OP_MUL: Serial.println("MUL"); break;
                case OP_DIV: Serial.println("DIV"); break;
                case OP_MOD: Serial.println("MOD"); break;
                case OP_ABS: Serial.println("ABS"); break;
                case OP_POW: Serial.println("POW"); break;
                case OP_EQ: Serial.println("EQ"); break;
                case OP_NEQ: Serial.println("NEQ"); break;
                case OP_LT: Serial.println("LT"); break;
                case OP_GT: Serial.println("GT"); break;
                case OP_LTE: Serial.println("LTE"); break;
                case OP_GTE: Serial.println("GTE"); break;
                case OP_PRINT_NUM: Serial.println("PRINT_NUM"); break;
                case OP_STORE: 
                    Serial.println(instr.arg1 < string_table.size() ? 
                        "STORE " + string_table[instr.arg1] : "STORE <invalid var>");
                    break;
                case OP_LOAD: 
                    Serial.println(instr.arg1 < string_table.size() ? 
                        "LOAD " + string_table[instr.arg1] : "LOAD <invalid var>");
                    break;
                case OP_JUMP: Serial.println("JUMP " + String(instr.arg1)); break;
                case OP_JUMP_IF: Serial.println("JUMP_IF " + String(instr.arg1)); break;
                case OP_HALT: Serial.println("HALT"); break;
                default: Serial.println("UNKNOWN " + String(instr.opcode)); break;
            }
        }
    }
};

#endif // XENO_VM_H