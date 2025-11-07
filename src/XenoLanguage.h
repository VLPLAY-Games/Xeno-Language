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

#ifndef XENO_LANGUAGE_H
#define XENO_LANGUAGE_H

#include "xeno/xeno_compiler.h"
#include "xeno/xeno_vm.h"

class XenoLanguage {
 private:
    static constexpr const char* xeno_language_version = "v0.1.2";
    static constexpr const char* xeno_language_date = "07.11.2025";
    static constexpr const char* xeno_language_name = "Xeno Language";

    XenoCompiler compiler;
    XenoVM vm;

 public:
    bool compile(const String& source_code);
    bool run();
    void step();
    void stop();
    bool isRunning() const;
    void dumpState();
    void disassemble();
    void printCompiledCode();
    void setMaxInstructions(uint32_t max_instr);

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
    const char* getCompilerVersion() noexcept;
    const char* getCompilerDate() noexcept;
    const char* getCompilerName() noexcept;

    // Information about language VM
    const char* getVMVersion() noexcept;
    const char* getVMDate() noexcept;
    const char* getVMName() noexcept;
};

#endif