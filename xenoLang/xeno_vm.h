#ifndef XENO_VM_H
#define XENO_VM_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <stack>
#include <cmath>

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

// Xeno Virtual Machine
class XenoVM {
private:
    std::vector<XenoInstruction> program;
    std::vector<String> string_table;
    uint32_t program_counter;
    XenoValue stack[64];
    uint32_t stack_pointer;
    std::map<String, XenoValue> variables;
    bool running;
    uint32_t instruction_count;
    
    void resetState() {
        program_counter = 0;
        stack_pointer = 0;
        running = false;
        instruction_count = 0;
        // Initialize stack with zeros - optimized loop
        memset(stack, 0, sizeof(stack));
        variables.clear();
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
            
            // Add to string table and return - optimized search
            for (size_t i = 0; i < string_table.size(); ++i) {
                if (string_table[i] == combined) {
                    return XenoValue::makeString(i);
                }
            }
            string_table.push_back(combined);
            return XenoValue::makeString(string_table.size() - 1);
        }
        
        // Numeric addition
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(a_val + b_val);
            }
            return XenoValue::makeInt(a.int_val + b.int_val);
        }
        
        return XenoValue::makeInt(0);
    }
    
    XenoValue performSubtraction(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(a_val - b_val);
            }
            return XenoValue::makeInt(a.int_val - b.int_val);
        }
        return XenoValue::makeInt(0);
    }
    
    XenoValue performMultiplication(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(a_val * b_val);
            }
            return XenoValue::makeInt(a.int_val * b.int_val);
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
            }
            if (b.int_val != 0) {
                return XenoValue::makeInt(a.int_val / b.int_val);
            }
            Serial.println("ERROR: Division by zero");
            return XenoValue::makeInt(0);
        }
        return XenoValue::makeInt(0);
    }
    
    XenoValue performModulo(const XenoValue& a, const XenoValue& b) {
        if (a.type == TYPE_INT && b.type == TYPE_INT) {
            if (b.int_val != 0) {
                return XenoValue::makeInt(a.int_val % b.int_val);
            }
            Serial.println("ERROR: Modulo by zero");
        } else {
            Serial.println("ERROR: Modulo requires integer operands");
        }
        return XenoValue::makeInt(0);
    }
    
    XenoValue performPower(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(pow(a_val, b_val));
            }
            int32_t result = 1;
            for (int32_t i = 0; i < b.int_val; ++i) {
                result *= a.int_val;
            }
            return XenoValue::makeInt(result);
        }
        return XenoValue::makeInt(0);
    }
    
    XenoValue performAbs(const XenoValue& a) {
        if (a.type == TYPE_INT) {
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
    
    void executeInstruction(const XenoInstruction& instr) {
        switch (instr.opcode) {
            case OP_NOP: break;
                
            case OP_PRINT:
                if (instr.arg1 < string_table.size()) {
                    Serial.println(string_table[instr.arg1]);
                } else {
                    Serial.println("ERROR: Invalid string index");
                }
                break;
                
            case OP_LED_ON:
                pinMode(instr.arg1, OUTPUT);
                digitalWrite(instr.arg1, HIGH);
                Serial.println("LED ON pin " + String(instr.arg1));
                break;
                
            case OP_LED_OFF:
                pinMode(instr.arg1, OUTPUT);
                digitalWrite(instr.arg1, LOW);
                Serial.println("LED OFF pin " + String(instr.arg1));
                break;
                
            case OP_DELAY:
                delay(instr.arg1);
                break;
                
            case OP_PUSH:
                if (stack_pointer < 64) {
                    stack[stack_pointer++] = XenoValue::makeInt(instr.arg1);
                } else {
                    Serial.println("ERROR: Stack overflow");
                }
                break;
                
            case OP_PUSH_FLOAT:
                if (stack_pointer < 64) {
                    float fval;
                    memcpy(&fval, &instr.arg1, sizeof(float));
                    stack[stack_pointer++] = XenoValue::makeFloat(fval);
                } else {
                    Serial.println("ERROR: Stack overflow");
                }
                break;
                
            case OP_PUSH_STRING:
                if (stack_pointer < 64) {
                    stack[stack_pointer++] = XenoValue::makeString(instr.arg1);
                } else {
                    Serial.println("ERROR: Stack overflow");
                }
                break;
                
            case OP_POP:
                if (stack_pointer > 0) {
                    --stack_pointer;
                } else {
                    Serial.println("ERROR: Stack underflow");
                }
                break;
                
            case OP_ADD:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = performAddition(a, b);
                } else {
                    Serial.println("ERROR: Not enough values for ADD");
                }
                break;
                
            case OP_SUB:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = performSubtraction(a, b);
                } else {
                    Serial.println("ERROR: Not enough values for SUB");
                }
                break;
                
            case OP_MUL:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = performMultiplication(a, b);
                } else {
                    Serial.println("ERROR: Not enough values for MUL");
                }
                break;
                
            case OP_DIV:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = performDivision(a, b);
                } else {
                    Serial.println("ERROR: Not enough values for DIV");
                }
                break;
                
            case OP_MOD:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = performModulo(a, b);
                } else {
                    Serial.println("ERROR: Not enough values for MOD");
                }
                break;
                
            case OP_ABS:
                if (stack_pointer >= 1) {
                    stack[stack_pointer - 1] = performAbs(stack[stack_pointer - 1]);
                } else {
                    Serial.println("ERROR: Not enough values for ABS");
                }
                break;
                
            case OP_POW:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = performPower(a, b);
                } else {
                    Serial.println("ERROR: Not enough values for POW");
                }
                break;
                
            case OP_EQ:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = XenoValue::makeInt(performComparison(a, b, OP_EQ) ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for EQ");
                }
                break;
                
            case OP_NEQ:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = XenoValue::makeInt(performComparison(a, b, OP_NEQ) ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for NEQ");
                }
                break;
                
            case OP_LT:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = XenoValue::makeInt(performComparison(a, b, OP_LT) ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for LT");
                }
                break;
                
            case OP_GT:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = XenoValue::makeInt(performComparison(a, b, OP_GT) ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for GT");
                }
                break;
                
            case OP_LTE:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = XenoValue::makeInt(performComparison(a, b, OP_LTE) ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for LTE");
                }
                break;
                
            case OP_GTE:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    stack[stack_pointer++] = XenoValue::makeInt(performComparison(a, b, OP_GTE) ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for GTE");
                }
                break;
                
            case OP_PRINT_NUM:
                if (stack_pointer > 0) {
                    const XenoValue& val = stack[stack_pointer - 1];
                    switch (val.type) {
                        case TYPE_INT: Serial.println(String(val.int_val)); break;
                        case TYPE_FLOAT: Serial.println(String(val.float_val, 2)); break;
                        case TYPE_STRING: Serial.println(string_table[val.string_index]); break;
                    }
                } else {
                    Serial.println("ERROR: Stack empty for PRINT_NUM");
                }
                break;
                
            case OP_STORE: {
                if (stack_pointer > 0) {
                    String var_name = string_table[instr.arg1];
                    variables[var_name] = stack[--stack_pointer];
                } else {
                    Serial.println("ERROR: Stack empty for STORE");
                }
                break;
            }
                
            case OP_LOAD: {
                String var_name = string_table[instr.arg1];
                auto it = variables.find(var_name);
                if (it != variables.end()) {
                    if (stack_pointer < 64) {
                        stack[stack_pointer++] = it->second;
                    } else {
                        Serial.println("ERROR: Stack overflow in LOAD");
                    }
                } else {
                    Serial.println("ERROR: Variable not found: " + var_name);
                    if (stack_pointer < 64) {
                        stack[stack_pointer++] = XenoValue::makeInt(0);
                    }
                }
                break;
            }
                
            case OP_JUMP:
                if (instr.arg1 < program.size()) {
                    program_counter = instr.arg1 - 1;
                } else {
                    Serial.println("ERROR: Jump to invalid address");
                }
                break;
                
            case OP_JUMP_IF:
                if (stack_pointer > 0) {
                    XenoValue condition_val = stack[--stack_pointer];
                    int condition = 0;
                    
                    switch (condition_val.type) {
                        case TYPE_INT: condition = (condition_val.int_val != 0); break;
                        case TYPE_FLOAT: condition = (condition_val.float_val != 0.0f); break;
                        case TYPE_STRING: condition = !string_table[condition_val.string_index].isEmpty(); break;
                    }
                    
                    if (condition && instr.arg1 < program.size()) {
                        program_counter = instr.arg1 - 1;
                    }
                } else {
                    Serial.println("ERROR: No condition for JUMP_IF");
                }
                break;
                
            default:
                Serial.println("ERROR: Unknown instruction " + String(instr.opcode));
                break;
        }
    }
    
public:
    XenoVM() {
        resetState();
        program.reserve(128);
        string_table.reserve(32);
    }
    
    void loadProgram(const std::vector<XenoInstruction>& bytecode, 
                    const std::vector<String>& strings) {
        resetState();
        program = bytecode;
        string_table = strings;
        running = true;
    }
    
    bool step() {
        if (!running || program_counter >= program.size()) {
            return false;
        }
        
        XenoInstruction instr = program[program_counter];
        
        if (instr.opcode == OP_HALT) {
            running = false;
            return false;
        }
        
        executeInstruction(instr);
        ++program_counter;
        
        instruction_count++;
        if (instruction_count > 5000) {
            Serial.println("ERROR: Instruction limit exceeded - possible infinite loop");
            running = false;
            return false;
        }
        
        return true;
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