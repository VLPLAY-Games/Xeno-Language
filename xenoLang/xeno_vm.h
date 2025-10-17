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
    OP_PUSH = 5,     // Push to stack
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
    OP_HALT = 255    // Stop execution
};

// Bytecode instruction structure
struct XenoInstruction {
    uint8_t opcode;
    uint16_t arg1;
    uint16_t arg2;
    
    XenoInstruction(uint8_t op = OP_NOP, uint16_t a1 = 0, uint16_t a2 = 0) 
        : opcode(op), arg1(a1), arg2(a2) {}
};

// Xeno Virtual Machine
class XenoVM {
private:
    std::vector<XenoInstruction> program;
    std::vector<String> string_table;
    uint32_t program_counter;
    uint32_t stack[64];
    uint32_t stack_pointer;
    uint32_t registers[8];
    std::map<String, uint32_t> variables;
    bool running;
    
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
                    stack[stack_pointer++] = instr.arg1;
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
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = a + b;
                } else {
                    Serial.println("ERROR: Not enough values for ADD");
                }
                break;
                
            case OP_SUB:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = a - b;
                } else {
                    Serial.println("ERROR: Not enough values for SUB");
                }
                break;
                
            case OP_MUL:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = a * b;
                } else {
                    Serial.println("ERROR: Not enough values for MUL");
                }
                break;
                
            case OP_DIV:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    if (b != 0) {
                        stack[stack_pointer++] = a / b;
                    } else {
                        Serial.println("ERROR: Division by zero");
                        stack[stack_pointer++] = 0;
                    }
                } else {
                    Serial.println("ERROR: Not enough values for DIV");
                }
                break;
                
            case OP_MOD:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    if (b != 0) {
                        stack[stack_pointer++] = a % b;
                    } else {
                        Serial.println("ERROR: Modulo by zero");
                        stack[stack_pointer++] = 0;
                    }
                } else {
                    Serial.println("ERROR: Not enough values for MOD");
                }
                break;
                
            case OP_ABS:
                if (stack_pointer >= 1) {
                    int32_t value = (int32_t)stack[stack_pointer - 1];
                    stack[stack_pointer - 1] = abs(value);
                } else {
                    Serial.println("ERROR: Not enough values for ABS");
                }
                break;
                
            case OP_POW:
                if (stack_pointer >= 2) {
                    uint32_t exponent = stack[--stack_pointer];
                    uint32_t base = stack[--stack_pointer];
                    uint32_t result = 1;
                    for (uint32_t i = 0; i < exponent; i++) {
                        result *= base;
                    }
                    stack[stack_pointer++] = result;
                } else {
                    Serial.println("ERROR: Not enough values for POW");
                }
                break;
                
            // Comparison operations
            case OP_EQ:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = (a == b) ? 0 : 1;
                } else {
                    Serial.println("ERROR: Not enough values for EQ");
                }
                break;
                
            case OP_NEQ:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = (a != b) ? 0 : 1;
                } else {
                    Serial.println("ERROR: Not enough values for NEQ");
                }
                break;
                
            case OP_LT:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = (a < b) ? 0 : 1;
                } else {
                    Serial.println("ERROR: Not enough values for LT");
                }
                break;
                
            case OP_GT:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = (a > b) ? 0 : 1;
                } else {
                    Serial.println("ERROR: Not enough values for GT");
                }
                break;
                
            case OP_LTE:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = (a <= b) ? 0 : 1;
                } else {
                    Serial.println("ERROR: Not enough values for LTE");
                }
                break;
                
            case OP_GTE:
                if (stack_pointer >= 2) {
                    uint32_t b = stack[--stack_pointer];
                    uint32_t a = stack[--stack_pointer];
                    stack[stack_pointer++] = (a >= b) ? 0 : 1;
                } else {
                    Serial.println("ERROR: Not enough values for GTE");
                }
                break;
                
            case OP_PRINT_NUM:
                if (stack_pointer > 0) {
                    Serial.println(String(stack[stack_pointer - 1]));
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
                        stack[stack_pointer++] = 0;
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
                    uint32_t condition = stack[--stack_pointer];
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
        program_counter = 0;
        stack_pointer = 0;
        running = false;
        memset(stack, 0, sizeof(stack));
        memset(registers, 0, sizeof(registers));
        variables.clear();
    }
    
    // Load program into VM
    void loadProgram(const std::vector<XenoInstruction>& bytecode, 
                    const std::vector<String>& strings) {
        program = bytecode;
        string_table = strings;
        program_counter = 0;
        stack_pointer = 0;
        variables.clear();
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
        
        // Safety: limit maximum instructions
        if (program_counter > 10000) {
            Serial.println("ERROR: Instruction limit exceeded");
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
            Serial.println("  " + String(i) + ": " + String(stack[i]));
        }
        if (stack_pointer > 10) Serial.println("  ...");
        Serial.println("]");
        
        Serial.println("Variables: {");
        for (auto& var : variables) {
            Serial.println("  " + var.first + ": " + String(var.second));
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