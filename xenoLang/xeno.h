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


#ifndef XENO_H
#define XENO_H

#include "core/xeno_compiler.h"
#include "core/xeno_vm.h"

class Xeno {
private:
    static constexpr const char* xeno_language_version = "v0.1.0";
    static constexpr const char* xeno_language_date = "26.10.2025";
    static constexpr const char* xeno_language_name = "Xeno Language";

    XenoCompiler compiler;
    XenoVM vm;

public:
    bool compile(const String& source_code) {
        compiler.compile(source_code);
        return true;
    }
    
    bool run() {
        vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
        vm.run();
        return true;
    }
    
    void step() {
        vm.step();
    }
    
    void stop() {
        vm.stop();
    }
    
    bool isRunning() const {
        return vm.isRunning();
    }
    
    void dumpState() {
        vm.dumpState();
    }
    
    void disassemble() {
        vm.disassemble();
    }
    
    void printCompiledCode() {
        compiler.printCompiledCode();
    }


    // Information about language
    static constexpr const char* getLanguageVersion() noexcept {
        return xeno_language_version;
    }

    static constexpr const char* getLanguageDate() noexcept {
        return xeno_language_date;
    }

    static constexpr const char* getLanguageName() noexcept {
        return xeno_language_name;
    }

    // Information about language Compiler
    const char* getCompilerVersion() noexcept {
        return compiler.xeno_compiler_version;
    }

    const char* getCompilerDate() noexcept {
        return compiler.xeno_compiler_date;
    }

    const char* getCompilerName() noexcept {
        return compiler.xeno_compiler_name;
    }

    // Information about language VM
    const char* getVMVersion() noexcept {
        return vm.xeno_vm_version;
    }

    const char* getVMDate() noexcept {
        return vm.xeno_vm_date;
    }

    const char* getVMName() noexcept {
        return vm.xeno_vm_name;
    }
};

#endif // XENO_H