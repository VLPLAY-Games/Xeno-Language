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


#include <vector>
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

    bool hasArg = false;
    const char* mnemonic = nullptr;

    switch (instr.opcode) {
        case OP_NOP: mnemonic = "NOP"; break;
        case OP_POP: mnemonic = "POP"; break;
        case OP_ADD: mnemonic = "ADD"; break;
        case OP_SUB: mnemonic = "SUB"; break;
        case OP_MUL: mnemonic = "MUL"; break;
        case OP_DIV: mnemonic = "DIV"; break;
        case OP_MOD: mnemonic = "MOD"; break;
        case OP_ABS: mnemonic = "ABS"; break;
        case OP_POW: mnemonic = "POW"; break;
        case OP_MAX: mnemonic = "MAX"; break;
        case OP_MIN: mnemonic = "MIN"; break;
        case OP_SQRT: mnemonic = "SQRT"; break;
        case OP_EQ: mnemonic = "EQ"; break;
        case OP_NEQ: mnemonic = "NEQ"; break;
        case OP_LT: mnemonic = "LT"; break;
        case OP_GT: mnemonic = "GT"; break;
        case OP_LTE: mnemonic = "LTE"; break;
        case OP_GTE: mnemonic = "GTE"; break;
        case OP_PRINT_NUM: mnemonic = "PRINT_NUM"; break;
        case OP_SIN: mnemonic = "SIN"; break;
        case OP_COS: mnemonic = "COS"; break;
        case OP_TAN: mnemonic = "TAN"; break;
        case OP_HALT: mnemonic = "HALT"; break;

        case OP_PRINT:
            Serial.print("PRINT ");
            printStringArg(instr.arg1, string_table, false);
            hasArg = true;
            break;

        case OP_LED_ON:
            Serial.print("LED_ON pin=");
            Serial.print(instr.arg1);
            hasArg = true;
            break;

        case OP_LED_OFF:
            Serial.print("LED_OFF pin=");
            Serial.print(instr.arg1);
            hasArg = true;
            break;

        case OP_DELAY:
            Serial.print("DELAY ");
            Serial.print(instr.arg1);
            Serial.print("ms");
            hasArg = true;
            break;

        case OP_PUSH:
            Serial.print("PUSH ");
            Serial.print(instr.arg1);
            hasArg = true;
            break;

        case OP_PUSH_FLOAT: {
            float fval;
            memcpy(&fval, &instr.arg1, sizeof(float));
            Serial.print("PUSH_FLOAT ");
            Serial.print(fval, 4);
            hasArg = true;
            break;
        }

        case OP_PUSH_BOOL:
            Serial.print("PUSH_BOOL ");
            Serial.print(instr.arg1 ? "true" : "false");
            hasArg = true;
            break;

        case OP_PUSH_STRING:
            Serial.print("PUSH_STRING ");
            printStringArg(instr.arg1, string_table, true);
            hasArg = true;
            break;

        case OP_INPUT:
            Serial.print("INPUT ");
            printStringArg(instr.arg1, string_table, false);
            hasArg = true;
            break;

        case OP_STORE:
            Serial.print("STORE ");
            printStringArg(instr.arg1, string_table, false);
            hasArg = true;
            break;

        case OP_LOAD:
            Serial.print("LOAD ");
            printStringArg(instr.arg1, string_table, false);
            hasArg = true;
            break;

        case OP_JUMP:
            Serial.print("JUMP ");
            Serial.print(instr.arg1);
            hasArg = true;
            break;

        case OP_JUMP_IF:
            Serial.print("JUMP_IF ");
            Serial.print(instr.arg1);
            hasArg = true;
            break;

        default:
            Serial.print("UNKNOWN ");
            Serial.print(instr.opcode);
            hasArg = true;
            break;
    }

    if (!hasArg && mnemonic != nullptr) {
        Serial.println(mnemonic);
    } else if (hasArg) {
        Serial.println();
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
