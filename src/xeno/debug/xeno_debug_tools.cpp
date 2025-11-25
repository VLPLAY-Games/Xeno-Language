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


#include "xeno_debug_tools.h"

void Debugger::disassemble(const std::vector<XenoInstruction>& instructions, 
                        const std::vector<String>& string_table,
                        const String& title,
                        bool show_string_table) {
    Serial.println("=== " + title + " ===");
    
    if (show_string_table) {
        Serial.println("String table:");
        for (size_t i = 0; i < string_table.size(); ++i) {
            Serial.print("  ");
            Serial.print(i);
            Serial.print(": \"");
            Serial.print(string_table[i]);
            Serial.println("\"");
        }
    }
    
    Serial.println(show_string_table ? "Bytecode:" : "Instructions:");
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        printInstruction(i, instructions[i], string_table);
    }
}

void Debugger::printInstruction(size_t index, const XenoInstruction& instr, 
                            const std::vector<String>& string_table) {
    Serial.print(index);
    Serial.print(": ");
    
    switch (instr.opcode) {
        case OP_NOP: 
            Serial.println("NOP"); 
            break;
            
        case OP_PRINT:
            Serial.print("PRINT ");
            printStringArg(instr.arg1, string_table);
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
        
        case OP_PUSH_BOOL:
            Serial.print("PUSH_BOOL ");
            Serial.println(instr.arg1 ? "true" : "false");
            break;
            
        case OP_PUSH_STRING:
            Serial.print("PUSH_STRING ");
            printStringArg(instr.arg1, string_table, true);
            Serial.println();
            break;
            
        case OP_POP: 
            Serial.println("POP"); 
            break;
            
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
            printStringArg(instr.arg1, string_table, false);
            Serial.println();
            break;
            
        case OP_EQ: Serial.println("EQ"); break;
        case OP_NEQ: Serial.println("NEQ"); break;
        case OP_LT: Serial.println("LT"); break;
        case OP_GT: Serial.println("GT"); break;
        case OP_LTE: Serial.println("LTE"); break;
        case OP_GTE: Serial.println("GTE"); break;
            
        case OP_PRINT_NUM: 
            Serial.println("PRINT_NUM"); 
            break;
            
        case OP_STORE:
            Serial.print("STORE ");
            printStringArg(instr.arg1, string_table, false);
            Serial.println();
            break;
            
        case OP_LOAD:
            Serial.print("LOAD ");
            printStringArg(instr.arg1, string_table, false);
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
            
        case OP_SIN: Serial.println("SIN"); break;
        case OP_COS: Serial.println("COS"); break;
        case OP_TAN: Serial.println("TAN"); break;
        case OP_HALT: Serial.println("HALT"); break;
            
        default:
            Serial.print("UNKNOWN ");
            Serial.println(instr.opcode);
            break;
    }
}

void Debugger::printStringArg(uint32_t arg, const std::vector<String>& string_table, bool quoted) {
    if (arg < string_table.size()) {
        if (quoted) Serial.print("\"");
        Serial.print(string_table[arg]);
        if (quoted) Serial.print("\"");
    } else {
        Serial.print("<invalid>");
    }
}
