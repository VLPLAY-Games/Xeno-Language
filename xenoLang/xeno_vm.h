#ifndef XENO_VM_H
#define XENO_VM_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <stack>
#include <cmath>

// Operation codes for Xeno bytecode
enum XenoOpcodes {
    OP_NOP = 0,      // No operation
    OP_PRINT = 1,    // Print string
    OP_LED_ON = 2,   // Turn LED on
    OP_LED_OFF = 3,  // Turn LED off  
    OP_DELAY = 4,    // Delay in milliseconds
    OP_PUSH = 5,     // Push integer to stack
    OP_POP = 6,      // Pop from stack
    OP_ADD = 7,      // Addition
    OP_SUB = 8,      // Subtraction
    OP_MUL = 9,      // Multiplication
    OP_DIV = 10,     // Division
    OP_JUMP = 11,    // Jump to address
    OP_JUMP_IF = 12, // Conditional jump
    OP_PRINT_NUM = 13, // Print number from stack
    OP_STORE = 14,   // Store to variable
    OP_LOAD = 15,    // Load from variable
    OP_MOD = 16,     // Modulo (remainder)
    OP_ABS = 17,     // Absolute value
    OP_POW = 18,     // Power (exponentiation)
    OP_EQ = 19,      // Equal to
    OP_NEQ = 20,     // Not equal to
    OP_LT = 21,      // Less than
    OP_GT = 22,      // Greater than
    OP_LTE = 23,     // Less than or equal
    OP_GTE = 24,     // Greater than or equal
    OP_HALT = 255,   // Stop execution
    OP_PUSH_FLOAT = 25, // Push float to stack
    OP_PUSH_STRING = 26 // Push string to stack
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
        uint16_t string_index; // index in string_table
    };
    
    XenoValue() : type(TYPE_INT), int_val(0) {}
    
    // Helper method to create from int without ambiguity
    static XenoValue makeInt(int32_t val) {
        XenoValue v;
        v.type = TYPE_INT;
        v.int_val = val;
        return v;
    }
    
    // Helper method to create from float without ambiguity  
    static XenoValue makeFloat(float val) {
        XenoValue v;
        v.type = TYPE_FLOAT;
        v.float_val = val;
        return v;
    }
    
    // Helper method to create from string without ambiguity
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
    uint32_t arg1;  // Changed to 32-bit to store float bits
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
        for (int i = 0; i < 64; i++) {
            stack[i] = XenoValue::makeInt(0);
        }
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
            String str_a = string_table[a.string_index];
            String str_b = string_table[b.string_index];
            String combined = str_a + str_b;
            
            // Add to string table and return
            for (size_t i = 0; i < string_table.size(); i++) {
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
            } else {
                return XenoValue::makeInt(a.int_val + b.int_val);
            }
        }
        
        // Default to integer 0 on type mismatch
        return XenoValue::makeInt(0);
    }
    
    XenoValue performSubtraction(const XenoValue& a, const XenoValue& b) {
        if (bothNumeric(a, b)) {
            if (a.type == TYPE_FLOAT || b.type == TYPE_FLOAT) {
                float a_val = (a.type == TYPE_INT) ? static_cast<float>(a.int_val) : a.float_val;
                float b_val = (b.type == TYPE_INT) ? static_cast<float>(b.int_val) : b.float_val;
                return XenoValue::makeFloat(a_val - b_val);
            } else {
                return XenoValue::makeInt(a.int_val - b.int_val);
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
                return XenoValue::makeInt(a.int_val * b.int_val);
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
                } else {
                    Serial.println("ERROR: Division by zero");
                    return XenoValue::makeFloat(0.0f);
                }
            } else {
                if (b.int_val != 0) {
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
            if (b.int_val != 0) {
                return XenoValue::makeInt(a.int_val % b.int_val);
            } else {
                Serial.println("ERROR: Modulo by zero");
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
                int32_t result = 1;
                for (int32_t i = 0; i < b.int_val; i++) {
                    result *= a.int_val;
                }
                return XenoValue::makeInt(result);
            }
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
            // Try to convert to common type
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
                    String str_a = string_table[a.string_index];
                    String str_b = string_table[b.string_index];
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
            case OP_NOP:
                break;
                
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
                    stack_pointer--;
                } else {
                    Serial.println("ERROR: Stack underflow");
                }
                break;
                
            case OP_ADD:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    XenoValue result = performAddition(a, b);
                    stack[stack_pointer++] = result;
                } else {
                    Serial.println("ERROR: Not enough values for ADD");
                }
                break;
                
            case OP_SUB:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    XenoValue result = performSubtraction(a, b);
                    stack[stack_pointer++] = result;
                } else {
                    Serial.println("ERROR: Not enough values for SUB");
                }
                break;
                
            case OP_MUL:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    XenoValue result = performMultiplication(a, b);
                    stack[stack_pointer++] = result;
                } else {
                    Serial.println("ERROR: Not enough values for MUL");
                }
                break;
                
            case OP_DIV:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    XenoValue result = performDivision(a, b);
                    stack[stack_pointer++] = result;
                } else {
                    Serial.println("ERROR: Not enough values for DIV");
                }
                break;
                
            case OP_MOD:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    XenoValue result = performModulo(a, b);
                    stack[stack_pointer++] = result;
                } else {
                    Serial.println("ERROR: Not enough values for MOD");
                }
                break;
                
            case OP_ABS:
                if (stack_pointer >= 1) {
                    XenoValue result = performAbs(stack[stack_pointer - 1]);
                    stack[stack_pointer - 1] = result;
                } else {
                    Serial.println("ERROR: Not enough values for ABS");
                }
                break;
                
            case OP_POW:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    XenoValue result = performPower(a, b);
                    stack[stack_pointer++] = result;
                } else {
                    Serial.println("ERROR: Not enough values for POW");
                }
                break;
                
            // Comparison operations
            case OP_EQ:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    bool result = performComparison(a, b, OP_EQ);
                    stack[stack_pointer++] = XenoValue::makeInt(result ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for EQ");
                }
                break;
                
            case OP_NEQ:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    bool result = performComparison(a, b, OP_NEQ);
                    stack[stack_pointer++] = XenoValue::makeInt(result ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for NEQ");
                }
                break;
                
            case OP_LT:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    bool result = performComparison(a, b, OP_LT);
                    stack[stack_pointer++] = XenoValue::makeInt(result ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for LT");
                }
                break;
                
            case OP_GT:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    bool result = performComparison(a, b, OP_GT);
                    stack[stack_pointer++] = XenoValue::makeInt(result ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for GT");
                }
                break;
                
            case OP_LTE:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    bool result = performComparison(a, b, OP_LTE);
                    stack[stack_pointer++] = XenoValue::makeInt(result ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for LTE");
                }
                break;
                
            case OP_GTE:
                if (stack_pointer >= 2) {
                    XenoValue b = stack[--stack_pointer];
                    XenoValue a = stack[--stack_pointer];
                    bool result = performComparison(a, b, OP_GTE);
                    stack[stack_pointer++] = XenoValue::makeInt(result ? 0 : 1);
                } else {
                    Serial.println("ERROR: Not enough values for GTE");
                }
                break;
                
            case OP_PRINT_NUM:
                if (stack_pointer > 0) {
                    XenoValue val = stack[stack_pointer - 1];
                    switch (val.type) {
                        case TYPE_INT:
                            Serial.println(String(val.int_val));
                            break;
                        case TYPE_FLOAT:
                            Serial.println(String(val.float_val, 2)); // Show 2 decimal places
                            break;
                        case TYPE_STRING:
                            Serial.println(string_table[val.string_index]);
                            break;
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
                if (variables.find(var_name) != variables.end()) {
                    if (stack_pointer < 64) {
                        stack[stack_pointer++] = variables[var_name];
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
                    
                    // Convert to boolean
                    switch (condition_val.type) {
                        case TYPE_INT:
                            condition = (condition_val.int_val != 0) ? 1 : 0;
                            break;
                        case TYPE_FLOAT:
                            condition = (condition_val.float_val != 0.0f) ? 1 : 0;
                            break;
                        case TYPE_STRING:
                            condition = (string_table[condition_val.string_index].length() > 0) ? 1 : 0;
                            break;
                    }
                    
                    if (condition != 0 && instr.arg1 < program.size()) {
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
    }
    
    // Load program into VM
    void loadProgram(const std::vector<XenoInstruction>& bytecode, 
                    const std::vector<String>& strings) {
        resetState();
        program = bytecode;
        string_table = strings;
        running = true;
    }
    
    // Execute one instruction
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
        program_counter++;
        
        // Safety: limit maximum instructions to prevent infinite loops
        instruction_count++;
        if (instruction_count > 5000) {
            Serial.println("ERROR: Instruction limit exceeded - possible infinite loop");
            running = false;
            return false;
        }
        
        return true;
    }
    
    // Run entire program
    void run() {
        Serial.println("Starting Xeno VM...");
        
        while (step()) {
            // Continue execution
        }
        
        Serial.println("Xeno VM finished");
    }
    
    // Stop execution
    void stop() {
        running = false;
        program_counter = 0;
        stack_pointer = 0;
    }
    
    // Get VM state
    bool isRunning() const { return running; }
    uint32_t getPC() const { return program_counter; }
    uint32_t getSP() const { return stack_pointer; }
    
    // Debug information
    void dumpState() {
        Serial.println("=== VM State ===");
        Serial.println("Program Counter: " + String(program_counter));
        Serial.println("Stack Pointer: " + String(stack_pointer));
        Serial.println("Stack: [");
        for (int i = 0; i < stack_pointer && i < 10; i++) {
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
        for (auto& var : variables) {
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
        for (size_t i = 0; i < program.size(); i++) {
            XenoInstruction instr = program[i];
            Serial.print(String(i) + ": ");
            
            switch (instr.opcode) {
                case OP_NOP: Serial.println("NOP"); break;
                case OP_PRINT: 
                    if (instr.arg1 < string_table.size()) {
                        Serial.println("PRINT \"" + string_table[instr.arg1] + "\"");
                    } else {
                        Serial.println("PRINT <invalid string>");
                    }
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
                    if (instr.arg1 < string_table.size()) {
                        Serial.println("PUSH_STRING \"" + string_table[instr.arg1] + "\"");
                    } else {
                        Serial.println("PUSH_STRING <invalid>");
                    }
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
                    if (instr.arg1 < string_table.size()) {
                        Serial.println("STORE " + string_table[instr.arg1]);
                    } else {
                        Serial.println("STORE <invalid var>");
                    }
                    break;
                case OP_LOAD: 
                    if (instr.arg1 < string_table.size()) {
                        Serial.println("LOAD " + string_table[instr.arg1]);
                    } else {
                        Serial.println("LOAD <invalid var>");
                    }
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