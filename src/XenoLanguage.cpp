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
#include "XenoLanguage.h"

XenoLanguage::XenoLanguage() {
    recreateObjects();
}

void XenoLanguage::recreateObjects() {
    if (compiler) delete compiler;
    if (vm) delete vm;
    compiler = new XenoCompiler(security_config);
    vm = new XenoVM(security_config);
}


bool XenoLanguage::compile(const String& source_code) {
    recreateObjects();
    compiler->compile(source_code);
    return true;
}

bool XenoLanguage::run(bool less_output) {
    vm->loadProgram(compiler->getBytecode(), compiler->getStringTable(), less_output);
    vm->run(less_output);
    return true;
}

bool XenoLanguage::compile_and_run(const String& source_code, bool less_output) {
    recreateObjects();
    compiler->compile(source_code);
    vm->loadProgram(compiler->getBytecode(), compiler->getStringTable(), less_output);
    vm->run(less_output);
    return true;
}

void XenoLanguage::step() {
    vm->step();
}

void XenoLanguage::stop() {
    vm->stop();
}

bool XenoLanguage::isRunning() const {
    return vm->isRunning();
}

void XenoLanguage::dumpState() {
    vm->dumpState();
}

void XenoLanguage::disassemble() {
    vm->disassemble();
}

void XenoLanguage::printCompiledCode() {
    compiler->printCompiledCode();
}

bool XenoLanguage::setMaxInstructions(uint32_t max_instr) {
    return security_config.setCurrentMaxInstructions(max_instr);
}

const XenoSecurityConfig& XenoLanguage::getSecurityConfig() const {
    return security_config;
}

bool XenoLanguage::setStringLimit(uint16_t length) {
    return security_config.setMaxStringLength(length);
}

bool XenoLanguage::setVariableNameLimit(uint16_t length) {
    return security_config.setMaxVariableNameLength(length);
}

bool XenoLanguage::setExpressionDepth(uint16_t depth) {
    return security_config.setMaxExpressionDepth(depth);
}

bool XenoLanguage::setLoopDepth(uint16_t depth) {
    return security_config.setMaxLoopDepth(depth);
}

bool XenoLanguage::setIfDepth(uint16_t depth) {
    return security_config.setMaxIfDepth(depth);
}

bool XenoLanguage::setStackSize(uint16_t size) {
    return security_config.setMaxStackSize(size);
}

bool XenoLanguage::setAllowedPins(const std::vector<uint8_t>& pins) {
    return security_config.setAllowedPins(pins);
}

bool XenoLanguage::addAllowedPin(uint8_t pin) {
    std::vector<uint8_t> current_pins = security_config.getAllowedPins();

    for (uint8_t existing_pin : current_pins) {
        if (existing_pin == pin) {
            return true;
        }
    }

    current_pins.push_back(pin);
    return security_config.setAllowedPins(current_pins);
}

bool XenoLanguage::removeAllowedPin(uint8_t pin) {
    std::vector<uint8_t> current_pins = security_config.getAllowedPins();

    for (auto it = current_pins.begin(); it != current_pins.end(); ++it) {
        if (*it == pin) {
            current_pins.erase(it);
            return security_config.setAllowedPins(current_pins);
        }
    }
    return false;
}

bool XenoLanguage::validateSecurityConfig() const {
    return security_config.validateConfig();
}

String XenoLanguage::getSecurityLimitsInfo() const {
    return security_config.getSecurityLimitsInfo();
}
