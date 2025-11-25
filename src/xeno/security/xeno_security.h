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
#include <vector>
#include "../xeno_common.h"
#include "xeno_security_config.h"

class XenoSecurity {
private:
    XenoSecurityConfig& config;

public:
    XenoSecurity(XenoSecurityConfig& cfg) : config(cfg) {}
    
    bool isPinAllowed(uint8_t pin);
    String sanitizeString(const String& input);
    bool verifyBytecode(const std::vector<XenoInstruction>& bytecode,
                       const std::vector<String>& strings);
};

#endif  // SRC_XENO_XENO_SECURITY_H_
