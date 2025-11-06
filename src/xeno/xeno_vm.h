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

#ifndef XENOLANG_CORE_XENO_VM_H_
#define XENOLANG_CORE_XENO_VM_H_

#include <vector>
#include <map>
#include <stack>
#include <cmath>
#include <limits>
#include <memory>
#include <algorithm>
#include "xeno_common.h"
#include "xeno_security.h"

class XenoVM {
 private:
    std::vector<XenoInstruction> program;
    std::vector<String> string_table;
    std::map<String, uint16_t> string_lookup;
    uint32_t program_counter;
    XenoValue stack[MAX_STACK_SIZE];
    uint32_t stack_pointer;
    std::map<String, XenoValue> variables;
    bool running;
    uint32_t instruction_count;
    uint32_t max_instructions;
    uint32_t iteration_count;
    static const uint32_t MAX_ITERATIONS = 100000;
    XenoSecurity security;

    friend class XenoLanguage;

    typedef void (XenoVM::*InstructionHandler)(const XenoInstruction&);
    InstructionHandler dispatch_table[256];

    void initializeDispatchTable();
    void resetState();
    String convertToString(const XenoValue& val);
    float toFloat(const XenoValue& v);
    bool Push(const XenoValue& value);
    bool Pop(XenoValue& value);
    bool PopTwo(XenoValue& a, XenoValue& b);
    bool Peek(XenoValue& value);
    bool Add(int32_t a, int32_t b, int32_t& result);
    bool Sub(int32_t a, int32_t b, int32_t& result);
    bool Mul(int32_t a, int32_t b, int32_t& result);
    bool Pow(int32_t base, int32_t exponent, int32_t& result);
    bool Mod(int32_t a, int32_t b, int32_t& result);
    XenoValue Sqrt(const XenoValue& a);
    XenoValue Max(const XenoValue& a, const XenoValue& b);
    XenoValue Min(const XenoValue& a, const XenoValue& b);
    XenoValue convertToFloat(const XenoValue& val);
    bool bothNumeric(const XenoValue& a, const XenoValue& b);
    XenoValue performAddition(const XenoValue& a, const XenoValue& b);
    XenoValue performSubtraction(const XenoValue& a, const XenoValue& b);
    XenoValue performMultiplication(const XenoValue& a, const XenoValue& b);
    XenoValue performDivision(const XenoValue& a, const XenoValue& b);
    XenoValue performModulo(const XenoValue& a, const XenoValue& b);
    XenoValue performPower(const XenoValue& a, const XenoValue& b);
    XenoValue performAbs(const XenoValue& a);
    bool performComparison(const XenoValue& a, const XenoValue& b, uint8_t op);
    uint16_t addString(const String& str);
    bool isInteger(const String& str);
    bool isFloat(const String& str);

    void handleNOP(const XenoInstruction& instr);
    void handlePRINT(const XenoInstruction& instr);
    void handleLED_ON(const XenoInstruction& instr);
    void handleLED_OFF(const XenoInstruction& instr);
    void handleDELAY(const XenoInstruction& instr);
    void handlePUSH(const XenoInstruction& instr);
    void handlePUSH_FLOAT(const XenoInstruction& instr);
    void handlePUSH_STRING(const XenoInstruction& instr);
    void handlePOP(const XenoInstruction& instr);
    void handleADD(const XenoInstruction& instr);
    void handleSUB(const XenoInstruction& instr);
    void handleMUL(const XenoInstruction& instr);
    void handleDIV(const XenoInstruction& instr);
    void handleMOD(const XenoInstruction& instr);
    void handleABS(const XenoInstruction& instr);
    void handlePOW(const XenoInstruction& instr);
    void handleMAX(const XenoInstruction& instr);
    void handleMIN(const XenoInstruction& instr);
    void handleSQRT(const XenoInstruction& instr);
    void handleINPUT(const XenoInstruction& instr);
    void handleEQ(const XenoInstruction& instr);
    void handleNEQ(const XenoInstruction& instr);
    void handleLT(const XenoInstruction& instr);
    void handleGT(const XenoInstruction& instr);
    void handleLTE(const XenoInstruction& instr);
    void handleGTE(const XenoInstruction& instr);
    void handlePRINT_NUM(const XenoInstruction& instr);
    void handleSTORE(const XenoInstruction& instr);
    void handleLOAD(const XenoInstruction& instr);
    void handleJUMP(const XenoInstruction& instr);
    void handleJUMP_IF(const XenoInstruction& instr);
    void handleHALT(const XenoInstruction& instr);

 protected:
    static constexpr const char* xeno_vm_name = "Xeno Virtual Machine";
    static constexpr const char* xeno_vm_version = "v0.1.0";
    static constexpr const char* xeno_vm_date = "26.10.2025";

    XenoVM();
    void setMaxInstructions(uint32_t max_instr);
    void loadProgram(const std::vector<XenoInstruction>& bytecode,
                    const std::vector<String>& strings);
    bool step();
    void run();
    void stop();
    bool isRunning() const;
    uint32_t getPC() const;
    uint32_t getSP() const;
    uint32_t getInstructionCount() const;
    uint32_t getIterationCount() const;
    void dumpState();
    void disassemble();
};

#endif  // XENOLANG_CORE_XENO_VM_H_