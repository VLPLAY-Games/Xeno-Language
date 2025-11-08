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

void XenoLanguage::setMaxInstructions(uint32_t max_instr) {
    vm.setMaxInstructions(max_instr);
}

const char* XenoLanguage::getCompilerVersion() noexcept {
    return compiler.xeno_compiler_version;
}

const char* XenoLanguage::getCompilerDate() noexcept {
    return compiler.xeno_compiler_date;
}

const char* XenoLanguage::getCompilerName() noexcept {
    return compiler.xeno_compiler_name;
}

const char* XenoLanguage::getVMVersion() noexcept {
    return vm.xeno_vm_version;
}

const char* XenoLanguage::getVMDate() noexcept {
    return vm.xeno_vm_date;
}

const char* XenoLanguage::getVMName() noexcept {
    return vm.xeno_vm_name;
}
