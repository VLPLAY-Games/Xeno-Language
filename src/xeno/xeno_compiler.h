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

#ifndef SRC_XENO_XENO_COMPILER_H_
#define SRC_XENO_XENO_COMPILER_H_

#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include "xeno_common.h"
#include "xeno_security.h"

class XenoCompiler {
 private:
    std::vector<XenoInstruction> bytecode;
    std::vector<String> string_table;
    std::map<String, XenoValue> variable_map;
    std::vector<int> if_stack;
    std::vector<LoopInfo> loop_stack;

    struct Constant {
        const char* name;
        const char* value;
    };
    static const Constant constants[];
    static const size_t constants_count;

    friend class XenoLanguage;

    bool validateString(const String& str);
    bool validateVariableName(const String& name);
    String cleanLine(const String& line);
    int addString(const String& str);
    int getVariableIndex(const String& var_name);
    bool isInteger(const String& str);
    bool isFloat(const String& str);
    bool isBool(const String& str);
    bool isQuotedString(const String& str);
    bool isValidVariable(const String& str);
    bool isComparisonOperator(const String& str);
    int getPrecedence(const String& op);
    bool isRightAssociative(const String& op);
    String processFunctions(const String& expr);
    int findMatchingParenthesis(const String& expr, int start);
    std::vector<String> infixToPostfix(const std::vector<String>& tokens);
    std::vector<String> tokenizeExpression(const String& expr);
    void compilePostfix(const std::vector<String>& postfix);
    void compileExpression(const String& expr);
    String extractVariableName(const String& text);
    XenoDataType determineValueType(const String& value);
    XenoValue createValueFromString(const String& str, XenoDataType type);
    void emitInstruction(uint8_t opcode, uint32_t arg1 = 0, uint16_t arg2 = 0);
    int getCurrentAddress();
    void compileLine(const String& line, int line_number);
    void processConstants(String& expr);


 protected:
    static constexpr const char* xeno_compiler_name = "Xeno Compiler";
    static constexpr const char* xeno_compiler_version = "v0.1.3";
    static constexpr const char* xeno_compiler_date = "08.11.2025";

    XenoCompiler();
    void compile(const String& source_code);
    const std::vector<XenoInstruction>& getBytecode() const;
    const std::vector<String>& getStringTable() const;
    void printCompiledCode();
};

#endif  // SRC_XENO_XENO_COMPILER_H_
