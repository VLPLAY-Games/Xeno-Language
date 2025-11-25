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

#include "xeno_security_config.h"

bool XenoSecurityConfig::validateSizeLimit(size_t value, size_t min_val, size_t max_val, const char* param_name) {
    if (value < min_val || value > max_val) {
        Serial.print("SECURITY: ");
        Serial.print(param_name);
        Serial.print(" must be between ");
        Serial.print(min_val);
        Serial.print(" and ");
        Serial.println(max_val);
        return false;
    }
    return true;
}

bool XenoSecurityConfig::setMaxStringLength(size_t length) {
    if (!validateSizeLimit(length, MIN_STRING_LENGTH, MAX_STRING_LENGTH_LIMIT, "MAX_STRING_LENGTH")) {
        return false;
    }
    max_string_length = length;
    return true;
}

bool XenoSecurityConfig::setMaxVariableNameLength(size_t length) {
    if (!validateSizeLimit(length, MIN_VARIABLE_NAME_LENGTH, MAX_VARIABLE_NAME_LENGTH_LIMIT, "MAX_VARIABLE_NAME_LENGTH")) {
        return false;
    }
    max_variable_name_length = length;
    return true;
}

bool XenoSecurityConfig::setMaxExpressionDepth(size_t depth) {
    if (!validateSizeLimit(depth, MIN_EXPRESSION_DEPTH, MAX_EXPRESSION_DEPTH_LIMIT, "MAX_EXPRESSION_DEPTH")) {
        return false;
    }
    max_expression_depth = depth;
    return true;
}

bool XenoSecurityConfig::setMaxLoopDepth(size_t depth) {
    if (!validateSizeLimit(depth, MIN_LOOP_DEPTH, MAX_LOOP_DEPTH_LIMIT, "MAX_LOOP_DEPTH")) {
        return false;
    }
    max_loop_depth = depth;
    return true;
}

bool XenoSecurityConfig::setMaxIfDepth(size_t depth) {
    if (!validateSizeLimit(depth, MIN_IF_DEPTH, MAX_IF_DEPTH_LIMIT, "MAX_IF_DEPTH")) {
        return false;
    }
    max_if_depth = depth;
    return true;
}

bool XenoSecurityConfig::setMaxStackSize(size_t size) {
    if (!validateSizeLimit(size, MIN_STACK_SIZE, MAX_STACK_SIZE_LIMIT, "MAX_STACK_SIZE")) {
        return false;
    }
    max_stack_size = size;
    return true;
}

bool XenoSecurityConfig::setCurrentMaxInstructions(uint32_t max_instr) {
    if (max_instr < MIN_INSTRUCTIONS_LIMIT || max_instr > MAX_INSTRUCTIONS_LIMIT) {
        Serial.print("SECURITY: max_instructions must be between ");
        Serial.print(MIN_INSTRUCTIONS_LIMIT);
        Serial.print(" and ");
        Serial.println(MAX_INSTRUCTIONS_LIMIT);
        return false;
    }
    current_max_instructions = max_instr;
    return true;
}

bool XenoSecurityConfig::setAllowedPins(const std::vector<uint8_t>& pins) {
    for (uint8_t pin : pins) {
        if (pin < MIN_PIN_NUMBER || pin > MAX_PIN_NUMBER) {
            Serial.print("SECURITY: Invalid pin number (");
            Serial.print(pin);
            Serial.print("). Must be between ");
            Serial.print(MIN_PIN_NUMBER);
            Serial.print(" and ");
            Serial.println(MAX_PIN_NUMBER);
            return false;
        }
    }
    allowed_pins = pins;
    return true;
}

bool XenoSecurityConfig::isPinAllowed(uint8_t pin) const {
    for (uint8_t allowed_pin : allowed_pins) {
        if (pin == allowed_pin) {
            return true;
        }
    }
    return false;
}

bool XenoSecurityConfig::validateConfig() const {
    XenoSecurityConfig temp = *this;
    return temp.setMaxStringLength(max_string_length) &&
           temp.setMaxVariableNameLength(max_variable_name_length) &&
           temp.setMaxExpressionDepth(max_expression_depth) &&
           temp.setMaxLoopDepth(max_loop_depth) &&
           temp.setMaxIfDepth(max_if_depth) &&
           temp.setMaxStackSize(max_stack_size) &&
           temp.setCurrentMaxInstructions(current_max_instructions) &&
           temp.setAllowedPins(allowed_pins);
}

String XenoSecurityConfig::getSecurityLimitsInfo() const {
    String info = "Security Limits:\n";
    info += "String Length: " + String(MIN_STRING_LENGTH) + " - " + String(MAX_STRING_LENGTH_LIMIT) + "\n";
    info += "Variable Name: " + String(MIN_VARIABLE_NAME_LENGTH) + " - " + String(MAX_VARIABLE_NAME_LENGTH_LIMIT) + "\n";
    info += "Expression Depth: " + String(MIN_EXPRESSION_DEPTH) + " - " + String(MAX_EXPRESSION_DEPTH_LIMIT) + "\n";
    info += "Loop Depth: " + String(MIN_LOOP_DEPTH) + " - " + String(MAX_LOOP_DEPTH_LIMIT) + "\n";
    info += "If Depth: " + String(MIN_IF_DEPTH) + " - " + String(MAX_IF_DEPTH_LIMIT) + "\n";
    info += "Stack Size: " + String(MIN_STACK_SIZE) + " - " + String(MAX_STACK_SIZE_LIMIT) + "\n";
    info += "Instructions: " + String(MIN_INSTRUCTIONS_LIMIT) + " - " + String(MAX_INSTRUCTIONS_LIMIT) + "\n";
    info += "Pin Numbers: " + String(MIN_PIN_NUMBER) + " - " + String(MAX_PIN_NUMBER);
    return info;
}
