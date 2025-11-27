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


#ifndef SRC_XENO_DEBUG_H_
#define SRC_XENO_DEBUG_H_

#include <vector>
#include "../xeno_common.h"

class Debugger {
 protected:
    friend class XenoCompiler;
    friend class XenoVM;
    static void disassemble(const std::vector<XenoInstruction>& instructions,
                          const std::vector<String>& string_table,
                          const String& title = "Disassembly",
                          bool show_string_table = false);

 private:
    static void printInstruction(size_t index, const XenoInstruction& instr,
                               const std::vector<String>& string_table);

    static void printStringArg(uint32_t arg, const std::vector<String>& string_table, bool quoted = true);
};

#endif
