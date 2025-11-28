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

#include <vector>
#include "xeno/main/xeno_compiler.h"
#include "xeno/main/xeno_vm.h"
#include "xeno/security/xeno_security_config.h"

class XenoLanguage {
 private:
    static constexpr const char* xeno_language_version = "v0.1.4";
    static constexpr const char* xeno_language_date = "27.11.2025";
    static constexpr const char* xeno_language_name = "Xeno Language";

    XenoSecurityConfig security_config;

    XenoCompiler* compiler = new XenoCompiler(security_config);
    XenoVM* vm = new XenoVM(security_config);

    void recreateObjects();

 public:
    XenoLanguage();

    bool compile(const String& source_code);
    bool run(bool less_output = true);
    void step();
    void stop();
    bool isRunning() const;
    void dumpState();
    void disassemble();
    void printCompiledCode();

    bool compile_and_run(const String& source_code, bool less_output = true);

    bool setMaxInstructions(uint32_t max_instr);

    const XenoSecurityConfig& getSecurityConfig() const;

    bool setStringLimit(uint16_t length);
    bool setVariableNameLimit(uint16_t length);
    bool setExpressionDepth(uint16_t depth);
    bool setLoopDepth(uint16_t depth);
    bool setIfDepth(uint16_t depth);
    bool setStackSize(uint16_t size);
    bool setAllowedPins(const std::vector<uint8_t>& pins);
    bool addAllowedPin(uint8_t pin);
    bool removeAllowedPin(uint8_t pin);

    bool validateSecurityConfig() const;

    String getSecurityLimitsInfo() const;

    uint16_t getMaxStringLength() const { return security_config.getMaxStringLength(); }
    uint16_t getMaxVariableNameLength() const { return security_config.getMaxVariableNameLength(); }
    uint16_t getMaxExpressionDepth() const { return security_config.getMaxExpressionDepth(); }
    uint16_t getMaxLoopDepth() const { return security_config.getMaxLoopDepth(); }
    uint16_t getMaxIfDepth() const { return security_config.getMaxIfDepth(); }
    uint16_t getMaxStackSize() const { return security_config.getMaxStackSize(); }
    uint32_t getCurrentMaxInstructions() const { return security_config.getCurrentMaxInstructions(); }
    const std::vector<uint8_t>& getAllowedPins() const { return security_config.getAllowedPins(); }

    static constexpr uint16_t getMinStringLength() { return XenoSecurityConfig::getMinStringLength(); }
    static constexpr uint16_t getMaxStringLengthLimit() { return XenoSecurityConfig::getMaxStringLengthLimit(); }
    static constexpr uint16_t getMinVariableNameLength() { return XenoSecurityConfig::getMinVariableNameLength(); }
    static constexpr uint16_t getMaxVariableNameLengthLimit() { return XenoSecurityConfig::getMaxVariableNameLengthLimit(); }
    static constexpr uint16_t getMinExpressionDepth() { return XenoSecurityConfig::getMinExpressionDepth(); }
    static constexpr uint16_t getMaxExpressionDepthLimit() { return XenoSecurityConfig::getMaxExpressionDepthLimit(); }
    static constexpr uint16_t getMinLoopDepth() { return XenoSecurityConfig::getMinLoopDepth(); }
    static constexpr uint16_t getMaxLoopDepthLimit() { return XenoSecurityConfig::getMaxLoopDepthLimit(); }
    static constexpr uint16_t getMinIfDepth() { return XenoSecurityConfig::getMinIfDepth(); }
    static constexpr uint16_t getMaxIfDepthLimit() { return XenoSecurityConfig::getMaxIfDepthLimit(); }
    static constexpr uint16_t getMinStackSize() { return XenoSecurityConfig::getMinStackSize(); }
    static constexpr uint16_t getMaxStackSizeLimit() { return XenoSecurityConfig::getMaxStackSizeLimit(); }
    static constexpr uint32_t getMinInstructionsLimit() { return XenoSecurityConfig::getMinInstructionsLimit(); }
    static constexpr uint32_t getMaxInstructionsLimitValue() { return XenoSecurityConfig::getMaxInstructionsLimitValue(); }
    static constexpr uint8_t getMinPinNumber() { return XenoSecurityConfig::getMinPinNumber(); }
    static constexpr uint8_t getMaxPinNumber() { return XenoSecurityConfig::getMaxPinNumber(); }

    static constexpr const char* getLanguageVersion() noexcept { return xeno_language_version; }
    static constexpr const char* getLanguageDate() noexcept { return xeno_language_date; }
    static constexpr const char* getLanguageName() noexcept { return xeno_language_name; }
};

#endif  // SRC_XENOLANGUAGE_H_
