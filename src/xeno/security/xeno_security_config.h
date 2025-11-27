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

#ifndef SRC_XENO_XENO_SECURITY_CONFIG_H_
#define SRC_XENO_XENO_SECURITY_CONFIG_H_

#include <Arduino.h>
#include <vector>
#include <cstdint>

class XenoSecurityConfig {
 private:
    uint16_t max_string_length = 256;
    uint16_t max_variable_name_length = 32;
    uint16_t max_expression_depth = 32;
    uint16_t max_loop_depth = 16;
    uint16_t max_if_depth = 16;
    uint16_t max_stack_size = 256;

    uint32_t current_max_instructions = 10000;

    std::vector<uint8_t> allowed_pins = { LED_BUILTIN };

    static constexpr uint16_t MIN_STRING_LENGTH = 1;
    static constexpr uint16_t MAX_STRING_LENGTH_LIMIT = 4096;

    static constexpr uint16_t MIN_VARIABLE_NAME_LENGTH = 1;
    static constexpr uint16_t MAX_VARIABLE_NAME_LENGTH_LIMIT = 256;
    
    static constexpr uint16_t MIN_EXPRESSION_DEPTH = 1;
    static constexpr uint16_t MAX_EXPRESSION_DEPTH_LIMIT = 256;

    static constexpr uint16_t MIN_LOOP_DEPTH = 1;
    static constexpr uint16_t MAX_LOOP_DEPTH_LIMIT = 64;

    static constexpr uint16_t MIN_IF_DEPTH = 1;
    static constexpr uint16_t MAX_IF_DEPTH_LIMIT = 64;

    static constexpr uint16_t MIN_STACK_SIZE = 16;
    static constexpr uint16_t MAX_STACK_SIZE_LIMIT = 2048;

    static constexpr uint32_t MIN_INSTRUCTIONS_LIMIT = 1000;
    static constexpr uint32_t MAX_INSTRUCTIONS_LIMIT = 1000000;

    static constexpr uint8_t MIN_PIN_NUMBER = 0;
    static constexpr uint8_t MAX_PIN_NUMBER = 255;

    bool validateSizeLimit(uint16_t value, uint16_t min_val, uint16_t max_val, const char* param_name);

 protected:
    friend class XenoLanguage;
    friend class XenoCompiler;
    friend class XenoVM;
    friend class XenoSecurity;
    XenoSecurityConfig() = default;

    uint16_t getMaxStringLength() const { return max_string_length; }
    uint16_t getMaxVariableNameLength() const { return max_variable_name_length; }
    uint16_t getMaxExpressionDepth() const { return max_expression_depth; }
    uint16_t getMaxLoopDepth() const { return max_loop_depth; }
    uint16_t getMaxIfDepth() const { return max_if_depth; }
    uint16_t getMaxStackSize() const { return max_stack_size; }
    uint32_t getCurrentMaxInstructions() const { return current_max_instructions; }
    const std::vector<uint8_t>& getAllowedPins() const { return allowed_pins; }

    static constexpr uint16_t getMinStringLength() { return MIN_STRING_LENGTH; }
    static constexpr uint16_t getMaxStringLengthLimit() { return MAX_STRING_LENGTH_LIMIT; }
    static constexpr uint16_t getMinVariableNameLength() { return MIN_VARIABLE_NAME_LENGTH; }
    static constexpr uint16_t getMaxVariableNameLengthLimit() { return MAX_VARIABLE_NAME_LENGTH_LIMIT; }
    static constexpr uint16_t getMinExpressionDepth() { return MIN_EXPRESSION_DEPTH; }
    static constexpr uint16_t getMaxExpressionDepthLimit() { return MAX_EXPRESSION_DEPTH_LIMIT; }
    static constexpr uint16_t getMinLoopDepth() { return MIN_LOOP_DEPTH; }
    static constexpr uint16_t getMaxLoopDepthLimit() { return MAX_LOOP_DEPTH_LIMIT; }
    static constexpr uint16_t getMinIfDepth() { return MIN_IF_DEPTH; }
    static constexpr uint16_t getMaxIfDepthLimit() { return MAX_IF_DEPTH_LIMIT; }
    static constexpr uint16_t getMinStackSize() { return MIN_STACK_SIZE; }
    static constexpr uint16_t getMaxStackSizeLimit() { return MAX_STACK_SIZE_LIMIT; }
    static constexpr uint32_t getMinInstructionsLimit() { return MIN_INSTRUCTIONS_LIMIT; }
    static constexpr uint32_t getMaxInstructionsLimitValue() { return MAX_INSTRUCTIONS_LIMIT; }
    static constexpr uint8_t getMinPinNumber() { return MIN_PIN_NUMBER; }
    static constexpr uint8_t getMaxPinNumber() { return MAX_PIN_NUMBER; }

    bool setMaxStringLength(uint16_t length);
    bool setMaxVariableNameLength(uint16_t length);
    bool setMaxExpressionDepth(uint16_t depth);
    bool setMaxLoopDepth(uint16_t depth);
    bool setMaxIfDepth(uint16_t depth);
    bool setMaxStackSize(uint16_t size);
    bool setCurrentMaxInstructions(uint32_t max_instr);
    bool setAllowedPins(const std::vector<uint8_t>& pins);

    bool isPinAllowed(uint8_t pin) const;

    bool validateConfig() const;

    String getSecurityLimitsInfo() const;
};

#endif  // SRC_XENO_XENO_SECURITY_CONFIG_H_
