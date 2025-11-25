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

#ifndef SRC_XENOLANGUAGE_H_
#define SRC_XENOLANGUAGE_H_

#include "xeno/main/xeno_compiler.h"
#include "xeno/main/xeno_vm.h"
#include "xeno/security/xeno_security_config.h"

class XenoLanguage {
private:
    static constexpr const char* xeno_language_version = "v0.1.4";
    static constexpr const char* xeno_language_date = "25.11.2025";
    static constexpr const char* xeno_language_name = "Xeno Language";

    XenoSecurityConfig security_config;
    XenoCompiler compiler;
    XenoVM vm;

public:
    XenoLanguage();
    
    bool compile(const String& source_code);
    bool run();
    void step();
    void stop();
    bool isRunning() const;
    void dumpState();
    void disassemble();
    void printCompiledCode();
    
    bool setMaxInstructions(uint32_t max_instr);
    
    XenoSecurityConfig& getSecurityConfig();
    bool updateSecurityConfig(const XenoSecurityConfig& new_config);
    
    bool setStringLimit(size_t length);
    bool setVariableNameLimit(size_t length);
    bool setStackSize(size_t size);
    bool setAllowedPins(const std::vector<uint8_t>& pins);
    bool addAllowedPin(uint8_t pin);
    bool removeAllowedPin(uint8_t pin);
    
    static constexpr const char* getLanguageVersion() noexcept {
        return xeno_language_version;
    }

    static constexpr const char* getLanguageDate() noexcept {
        return xeno_language_date;
    }

    static constexpr const char* getLanguageName() noexcept {
        return xeno_language_name;
    }
};

#endif  // SRC_XENOLANGUAGE_H_
