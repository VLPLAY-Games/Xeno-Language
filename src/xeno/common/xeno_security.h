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

#ifndef SRC_XENO_XENO_SECURITY_H_
#define SRC_XENO_XENO_SECURITY_H_

#include <Arduino.h>
#include <array>
#include <vector>
#include "xeno_common.h"

// Security limits
#define MAX_STRING_LENGTH 256
#define MAX_VARIABLE_NAME_LENGTH 32
#define MAX_EXPRESSION_DEPTH 32
#define MAX_LOOP_DEPTH 16
#define MAX_IF_DEPTH 16
#define MAX_STACK_SIZE 256

class XenoSecurity {
 private:
    // Allowed pins for safety
    static constexpr std::array<uint8_t, 13> ALLOWED_PINS = {
        2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, LED_BUILTIN
    };
    static constexpr size_t NUM_ALLOWED_PINS = ALLOWED_PINS.size();

 public:
    bool isPinAllowed(uint8_t pin);
    String sanitizeString(const String& input);
    bool verifyBytecode(const std::vector<XenoInstruction>& bytecode,
                       const std::vector<String>& strings);
};

#endif  // SRC_XENO_XENO_SECURITY_H_
