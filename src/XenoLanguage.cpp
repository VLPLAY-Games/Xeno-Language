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

#include "XenoLanguage.h"

XenoLanguage::XenoLanguage() : compiler(security_config), vm(security_config) {
}

bool XenoLanguage::compile(const String& source_code) {
    compiler.compile(source_code);
    return true;
}

bool XenoLanguage::run() {
    vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
    vm.run();
    return true;
}

void XenoLanguage::step() {
    vm.step();
}

void XenoLanguage::stop() {
    vm.stop();
}

bool XenoLanguage::isRunning() const {
    return vm.isRunning();
}

void XenoLanguage::dumpState() {
    vm.dumpState();
}

void XenoLanguage::disassemble() {
    vm.disassemble();
}

void XenoLanguage::printCompiledCode() {
    compiler.printCompiledCode();
}

bool XenoLanguage::setMaxInstructions(uint32_t max_instr) {
    return security_config.setMaxInstructions(max_instr);
}

XenoSecurityConfig& XenoLanguage::getSecurityConfig() {
    return security_config;
}

bool XenoLanguage::updateSecurityConfig(const XenoSecurityConfig& new_config) {
    security_config = new_config;
    return true;
}

bool XenoLanguage::setStringLimit(size_t length) {
    return security_config.setMaxStringLength(length);
}

bool XenoLanguage::setVariableNameLimit(size_t length) {
    return security_config.setMaxVariableNameLength(length);
}

bool XenoLanguage::setStackSize(size_t size) {
    return security_config.setMaxStackSize(size);
}

bool XenoLanguage::setAllowedPins(const std::vector<uint8_t>& pins) {
    security_config.setAllowedPins(pins);
    return true;
}

bool XenoLanguage::addAllowedPin(uint8_t pin) {
    for (auto existing_pin : security_config.ALLOWED_PINS) {
        if (existing_pin == pin) {
            return true;
        }
    }
    security_config.ALLOWED_PINS.push_back(pin);
    return true;
}

bool XenoLanguage::removeAllowedPin(uint8_t pin) {
    auto& pins = security_config.ALLOWED_PINS;
    for (auto it = pins.begin(); it != pins.end(); ++it) {
        if (*it == pin) {
            pins.erase(it);
            return true;
        }
    }
    return false;
}
