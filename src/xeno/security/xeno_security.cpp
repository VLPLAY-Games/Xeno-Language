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
#include "xeno_security.h"

bool XenoSecurity::isPinAllowed(uint8_t pin) {
    const std::vector<uint8_t>& allowed_pins = config.getAllowedPins();
    for (size_t i = 0; i < allowed_pins.size(); i++) {
        if (pin == allowed_pins[i]) {
            return true;
        }
    }
    return false;
}

String XenoSecurity::sanitizeString(const String& input) {
    String sanitized;
    sanitized.reserve(input.length());

    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];

        if (c >= 32 && c <= 126) {
            if (c == '\\' || c == '"' || c == '\'' || c == '`') {
                sanitized += '\\';
            }
            sanitized += c;
        } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            sanitized += c;
        } else {
            sanitized += '?';
        }

        if (sanitized.length() >= config.getMaxStringLength()) {
            sanitized += "...";
            break;
        }
    }

    return sanitized;
}

bool XenoSecurity::verifyBytecode(const std::vector<XenoInstruction>& bytecode,
                                 const std::vector<String>& strings) {
    if (bytecode.size() > 10000) {
        Serial.println("SECURITY: Program too large");
        return false;
    }

    if (strings.size() > 1000) {
        Serial.println("SECURITY: String table too large");
        return false;
    }

    for (size_t i = 0; i < bytecode.size(); i++) {
        const XenoInstruction& instr = bytecode[i];

        if (instr.opcode > 34 && instr.opcode != 255) {
            Serial.print("SECURITY: Invalid opcode at instruction ");
            Serial.println(i);
            return false;
        }

        if (instr.opcode == OP_JUMP || instr.opcode == OP_JUMP_IF) {
            if (instr.arg1 >= bytecode.size()) {
                Serial.print("SECURITY: Invalid jump target at instruction ");
                Serial.println(i);
                return false;
            }
        }

        if (instr.opcode == OP_PRINT || instr.opcode == OP_STORE ||
            instr.opcode == OP_LOAD || instr.opcode == OP_PUSH_STRING ||
            instr.opcode == OP_INPUT) {
            if (instr.arg1 >= strings.size()) {
                Serial.print("SECURITY: Invalid string index at instruction ");
                Serial.println(i);
                return false;
            }
        }

        if (instr.opcode == OP_LED_ON || instr.opcode == OP_LED_OFF) {
            if (!isPinAllowed(instr.arg1)) {
                Serial.print("SECURITY: Unauthorized pin access at instruction ");
                Serial.println(i);
                return false;
            }
        }

        if (instr.opcode == OP_DELAY) {
            if (instr.arg1 > 60000) {
                Serial.print("SECURITY: Excessive delay at instruction ");
                Serial.println(i);
                return false;
            }
        }
    }

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
