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

#include <stack>
#include <algorithm>
#include <vector>
#include "xeno_compiler.h"
#include "../debug/xeno_debug_tools.h"

const XenoCompiler::Constant XenoCompiler::constants[] = {
    {"M_PI", "3.141592653589793"},
    {"M_E", "2.718281828459045"},
    {"M_TAU", "6.283185307179586"},
    {"M_SQRT2", "1.4142135623730951"},
    {"M_SQRT3", "1.7320508075688772"},
    {"P_LIGHT_SPEED", "299792458"}
};

const size_t XenoCompiler::constants_count = std::size(constants);

const XenoCompiler::MathFunctionInfo XenoCompiler::math_functions[] = {
    {"abs(", '[', ']', OP_ABS, 1},
    {"max(", '{', '}', OP_MAX, 2},
    {"min(", '|', '|', OP_MIN, 2},
    {"sqrt(", '~', '~', OP_SQRT, 1},
    {"sin(", '#', '#', OP_SIN, 1},
    {"cos(", '@', '@', OP_COS, 1},
    {"tan(", '&', '&', OP_TAN, 1}
};

const size_t XenoCompiler::math_functions_count = sizeof(math_functions) / sizeof(math_functions[0]);

const XenoCompiler::SimpleCommand XenoCompiler::simple_commands[] = {
    {"pop", OP_POP},
    {"add", OP_ADD},
    {"sub", OP_SUB},
    {"mul", OP_MUL},
    {"div", OP_DIV},
    {"mod", OP_MOD},
    {"abs", OP_ABS},
    {"pow", OP_POW},
    {"max", OP_MAX},
    {"min", OP_MIN},
    {"sqrt", OP_SQRT},
    {"printnum", OP_PRINT_NUM},
    {"halt", OP_HALT}
};

const size_t XenoCompiler::simple_commands_count = sizeof(simple_commands) / sizeof(simple_commands[0]);

// ------------------------------------------------------------------
// Вспомогательные функции для проверки типов (без изменений)
// ------------------------------------------------------------------
static inline bool isNumericType(XenoDataType t) {
    return t == TYPE_INT || t == TYPE_FLOAT;
}

static inline bool isCompatibleForAssignment(XenoDataType varType, XenoDataType exprType) {
    if (varType == exprType) return true;
    if (varType == TYPE_FLOAT && exprType == TYPE_INT) return true;
    return false;
}

static XenoDataType getBinaryResultType(XenoDataType left, XenoDataType right, uint8_t opcode) {
    if (opcode == OP_EQ || opcode == OP_NEQ || opcode == OP_LT || opcode == OP_GT ||
        opcode == OP_LTE || opcode == OP_GTE) {
        return TYPE_BOOL;
    }
    if (opcode == OP_AND || opcode == OP_OR) {
        return TYPE_BOOL;
    }
    if (left == TYPE_STRING || right == TYPE_STRING) {
        if (opcode == OP_ADD) return TYPE_STRING;
        return TYPE_ANY;
    }
    if (isNumericType(left) && isNumericType(right)) {
        if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
            return TYPE_FLOAT;
        } else {
            return TYPE_INT;
        }
    }
    return TYPE_ANY;
}

static bool isValidBinaryOp(XenoDataType left, XenoDataType right, uint8_t opcode) {
    if (opcode == OP_EQ || opcode == OP_NEQ) {
        return true;
    }
    if (opcode == OP_LT || opcode == OP_GT || opcode == OP_LTE || opcode == OP_GTE) {
        if (isNumericType(left) && isNumericType(right)) return true;
        if (left == TYPE_STRING && right == TYPE_STRING) return true;
        if (left == TYPE_BOOL && right == TYPE_BOOL) return true;
        return false;
    }
    if (opcode == OP_AND || opcode == OP_OR) {
        return true;
    }
    if (opcode == OP_ADD) {
        if (isNumericType(left) && isNumericType(right)) return true;
        if (left == TYPE_STRING || right == TYPE_STRING) return true;
        return false;
    }
    if (opcode == OP_SUB || opcode == OP_MUL || opcode == OP_DIV || opcode == OP_POW) {
        return isNumericType(left) && isNumericType(right);
    }
    if (opcode == OP_MOD) {
        return left == TYPE_INT && right == TYPE_INT;
    }
    if (opcode == OP_MAX || opcode == OP_MIN) {
        return isNumericType(left) && isNumericType(right);
    }
    return false;
}

static bool isValidUnaryOp(XenoDataType operand, uint8_t opcode) {
    switch (opcode) {
        case OP_ABS:
        case OP_NEG:
        case OP_SQRT:
        case OP_SIN:
        case OP_COS:
        case OP_TAN:
            return isNumericType(operand);
        case OP_NOT:
            return true;
        default:
            return false;
    }
}

static XenoDataType getUnaryResultType(XenoDataType operand, uint8_t opcode) {
    switch (opcode) {
        case OP_ABS:
        case OP_NEG:
            return operand;
        case OP_SQRT:
        case OP_SIN:
        case OP_COS:
        case OP_TAN:
            return TYPE_FLOAT;
        case OP_NOT:
            return TYPE_BOOL;
        default:
            return TYPE_ANY;
    }
}

// ------------------------------------------------------------------
// Реализация методов компилятора
// ------------------------------------------------------------------

XenoCompiler::XenoCompiler(XenoSecurityConfig& config)
    : security_config(config), security(config), compile_error(false) {
    bytecode.reserve(128);
    string_table.reserve(32);
    if_chain_stack.reserve(security_config.getMaxIfDepth());
    loop_stack.reserve(security_config.getMaxLoopDepth());
    while_stack.reserve(security_config.getMaxLoopDepth());
    inside_function_declaration = false;
}

void XenoCompiler::compile(const String& source_code) {
    bytecode.clear();
    string_table.clear();
    variable_map.clear();
    is_array.clear();
    if_chain_stack.clear();
    loop_stack.clear();
    while_stack.clear();
    functions.clear();
    inside_function_declaration = false;
    compile_error = false;

    int line_number = 0;
    int startPos = 0;
    int endPos = source_code.indexOf('\n');

    while (endPos >= 0) {
        String line = source_code.substring(startPos, endPos);
        ++line_number;

        if (!line.isEmpty()) {
            compileLine(line, line_number);
            if (compile_error) {
                Serial.println("Compilation aborted due to errors.");
                return;
            }
        }

        startPos = endPos + 1;
        endPos = source_code.indexOf('\n', startPos);
    }

    String lastLine = source_code.substring(startPos);
    if (!lastLine.isEmpty()) {
        compileLine(lastLine, ++line_number);
        if (compile_error) {
            Serial.println("Compilation aborted due to errors.");
            return;
        }
    }

    if (bytecode.empty() || bytecode.back().opcode != OP_HALT) {
        bytecode.emplace_back(OP_HALT);
    }
}

// ------------------------------------------------------------------
// Компиляция выражений с проверкой типов
// ------------------------------------------------------------------

void XenoCompiler::compileExpression(const String& expr) {
    compileExpressionWithType(expr);
}

XenoDataType XenoCompiler::compileExpressionWithType(const String& expr) {
    if (expr.isEmpty() || expr.length() > 1024) {
        Serial.println("ERROR: Invalid expression");
        compile_error = true;
        return TYPE_ANY;
    }

    String processedExpr = processFunctions(expr);
    std::vector<String> tokens = tokenizeExpression(processedExpr);
    std::vector<String> postfix = infixToPostfix(tokens);
    return compilePostfix(postfix);
}

// ----- НОВАЯ ВЕРСИЯ compilePostfix с поддержкой CALL -----
XenoDataType XenoCompiler::compilePostfix(const std::vector<String>& postfix) {
    if (postfix.size() > 100) {
        Serial.println("ERROR: Postfix expression too complex");
        compile_error = true;
        return TYPE_ANY;
    }

    std::stack<XenoDataType> typeStack;

    for (const String& token : postfix) {
        // ---- ОПЕРАНДЫ ----
        if (isInteger(token)) {
            int32_t value = token.toInt();
            emitInstruction(OP_PUSH, static_cast<uint32_t>(value));
            typeStack.push(TYPE_INT);
        }
        else if (isFloat(token)) {
            float fval = token.toFloat();
            uint32_t fbits;
            memcpy(&fbits, &fval, sizeof(float));
            emitInstruction(OP_PUSH_FLOAT, fbits);
            typeStack.push(TYPE_FLOAT);
        }
        else if (isBool(token)) {
            bool bval = (token == "true");
            emitInstruction(OP_PUSH_BOOL, bval);
            typeStack.push(TYPE_BOOL);
        }
        else if (isQuotedString(token)) {
            String str = token.substring(1, token.length() - 1);
            if (!validateString(str)) str = "";
            int str_id = addString(str);
            emitInstruction(OP_PUSH_STRING, str_id);
            typeStack.push(TYPE_STRING);
        }
        else if (token == "UNARY_NOT") {
            if (typeStack.empty()) {
                Serial.println("ERROR: Type stack underflow in UNARY_NOT");
                compile_error = true;
                return TYPE_ANY;
            }
            XenoDataType operand = typeStack.top(); typeStack.pop();
            if (!isValidUnaryOp(operand, OP_NOT)) {
                Serial.println("ERROR: Invalid operand type for NOT");
                compile_error = true;
                return TYPE_ANY;
            }
            emitInstruction(OP_NOT);
            typeStack.push(TYPE_BOOL);
        }
        else if (token == "UNARY_NEG") {
            if (typeStack.empty()) {
                Serial.println("ERROR: Type stack underflow in UNARY_NEG");
                compile_error = true;
                return TYPE_ANY;
            }
            XenoDataType operand = typeStack.top(); typeStack.pop();
            if (!isValidUnaryOp(operand, OP_NEG)) {
                Serial.println("ERROR: Negation requires numeric operand");
                compile_error = true;
                return TYPE_ANY;
            }
            emitInstruction(OP_NEG);
            typeStack.push(operand);
        }
        else if (isValidVariable(token)) {
            auto it = variable_map.find(token);
            if (it == variable_map.end()) {
                Serial.print("ERROR: Variable '");
                Serial.print(token);
                Serial.println("' used before assignment");
                compile_error = true;
                return TYPE_ANY;
            }
            XenoDataType varType = it->second.type;
            int var_index = getVariableIndex(token);
            emitInstruction(OP_LOAD, var_index);
            typeStack.push(varType);
        }
        // ---- БИНАРНЫЕ ОПЕРАТОРЫ ----
        else if (token == "+" || token == "-" || token == "*" || token == "/" ||
                 token == "%" || token == "^" || token == "==" || token == "!=" ||
                 token == "<" || token == ">" || token == "<=" || token == ">=" ||
                 token == "&&" || token == "||") {
            if (typeStack.size() < 2) {
                Serial.print("ERROR: Type stack underflow for binary operator ");
                Serial.println(token);
                compile_error = true;
                return TYPE_ANY;
            }
            XenoDataType right = typeStack.top(); typeStack.pop();
            XenoDataType left  = typeStack.top(); typeStack.pop();

            uint8_t opcode;
            if (token == "+") opcode = OP_ADD;
            else if (token == "-") opcode = OP_SUB;
            else if (token == "*") opcode = OP_MUL;
            else if (token == "/") opcode = OP_DIV;
            else if (token == "%") opcode = OP_MOD;
            else if (token == "^") opcode = OP_POW;
            else if (token == "==") opcode = OP_EQ;
            else if (token == "!=") opcode = OP_NEQ;
            else if (token == "<") opcode = OP_LT;
            else if (token == ">") opcode = OP_GT;
            else if (token == "<=") opcode = OP_LTE;
            else if (token == ">=") opcode = OP_GTE;
            else if (token == "&&") opcode = OP_AND;
            else if (token == "||") opcode = OP_OR;
            else {
                Serial.print("ERROR: Unknown binary operator ");
                Serial.println(token);
                compile_error = true;
                return TYPE_ANY;
            }

            if (!isValidBinaryOp(left, right, opcode)) {
                Serial.print("ERROR: Type mismatch for operator ");
                Serial.println(token);
                compile_error = true;
                return TYPE_ANY;
            }

            emitInstruction(opcode);

            XenoDataType resultType = getBinaryResultType(left, right, opcode);
            if (resultType == TYPE_ANY) {
                Serial.print("ERROR: Cannot determine result type for ");
                Serial.println(token);
                compile_error = true;
                return TYPE_ANY;
            }
            typeStack.push(resultType);
        }
        // ---- МАТЕМАТИЧЕСКИЕ ФУНКЦИИ (специальные скобки) ----
        else {
            bool function_processed = false;
            for (size_t i = 0; i < math_functions_count; i++) {
                const MathFunctionInfo& func = math_functions[i];
                if (token.startsWith(String(func.open_bracket)) &&
                    token.endsWith(String(func.close_bracket))) {
                    // Проверка типов аргументов (они уже скомпилированы)
                    if (func.num_args == 1) {
                        if (typeStack.empty()) {
                            Serial.println("ERROR: Type stack underflow for math function");
                            compile_error = true;
                            return TYPE_ANY;
                        }
                        XenoDataType argType = typeStack.top(); typeStack.pop();
                        if (!isValidUnaryOp(argType, func.opcode)) {
                            Serial.print("ERROR: Invalid argument type for ");
                            Serial.println(func.name);
                            compile_error = true;
                            return TYPE_ANY;
                        }
                        XenoDataType resultType = getUnaryResultType(argType, func.opcode);
                        compileMathFunction(token, func);
                        typeStack.push(resultType);
                    } else if (func.num_args == 2) {
                        if (typeStack.size() < 2) {
                            Serial.println("ERROR: Type stack underflow for binary math function");
                            compile_error = true;
                            return TYPE_ANY;
                        }
                        XenoDataType right = typeStack.top(); typeStack.pop();
                        XenoDataType left  = typeStack.top(); typeStack.pop();
                        if (!isValidBinaryOp(left, right, func.opcode)) {
                            Serial.print("ERROR: Type mismatch for function ");
                            Serial.println(func.name);
                            compile_error = true;
                            return TYPE_ANY;
                        }
                        XenoDataType resultType = getBinaryResultType(left, right, func.opcode);
                        compileMathFunction(token, func);
                        typeStack.push(resultType);
                    } else {
                        compileMathFunction(token, func);
                        typeStack.push(TYPE_ANY);
                    }
                    function_processed = true;
                    break;
                }
            }
            // ---- НОВОЕ: обработка вызова пользовательской функции ----
            if (!function_processed && token.startsWith("CALL:")) {
                String funcName = token.substring(5); // после "CALL:"
                auto it = functions.find(funcName);
                if (it == functions.end()) {
                    Serial.print("ERROR: Function '");
                    Serial.print(funcName);
                    Serial.println("' not defined");
                    compile_error = true;
                    return TYPE_ANY;
                }
                const FunctionInfo& funcInfo = it->second;
                // Проверка количества аргументов
                if (typeStack.size() < (size_t)funcInfo.arity) {
                    Serial.print("ERROR: Not enough arguments for function '");
                    Serial.print(funcName);
                    Serial.print("' (expected ");
                    Serial.print(funcInfo.arity);
                    Serial.print(", got ");
                    Serial.print(typeStack.size());
                    Serial.println(")");
                    compile_error = true;
                    return TYPE_ANY;
                }
                // Проверка типов аргументов (пока только количество, типы позже)
                // Для простоты убираем со стека типы аргументов (они нам не нужны для проверки)
                // Однако мы должны убедиться, что аргументы совместимы (пока пропускаем)
                // Тип результата считаем ANY (позже будем выводить)
                // Удаляем аргументы из стека типов (они уже скомпилированы)
                for (int i = 0; i < funcInfo.arity; ++i) {
                    typeStack.pop();
                }
                // Добавляем имя функции в строковую таблицу и получаем индекс
                int funcNameIndex = addString(funcName);
                emitInstruction(OP_CALL, funcNameIndex);
                // Результат функции - пока ANY
                typeStack.push(TYPE_ANY);
            }
            else if (!function_processed) {
                Serial.print("ERROR: Unknown operator in expression: ");
                Serial.println(token);
                compile_error = true;
                return TYPE_ANY;
            }
        }
    }

    if (typeStack.empty()) {
        Serial.println("ERROR: Expression did not produce a value");
        compile_error = true;
        return TYPE_ANY;
    }

    return typeStack.top();
}

void XenoCompiler::compileMathFunction(const String& token, const MathFunctionInfo& func) {
    String innerExpr = token.substring(1, token.length() - 1);

    if (func.num_args == 1) {
        compileExpression(innerExpr);
        emitInstruction(func.opcode);
    } else if (func.num_args == 2) {
        int commaPos = innerExpr.indexOf(',');
        if (commaPos > 0) {
            String arg1 = innerExpr.substring(0, commaPos);
            String arg2 = innerExpr.substring(commaPos + 1);
            compileExpression(arg1);
            compileExpression(arg2);
            emitInstruction(func.opcode);
        } else {
            Serial.println("ERROR: Function requires two arguments");
            compile_error = true;
        }
    }
}

void XenoCompiler::compileSimpleCommand(const String& command, uint8_t opcode) {
    emitInstruction(opcode);
}

// ------------------------------------------------------------------
// Обработчики команд (без изменений)
// ------------------------------------------------------------------

void XenoCompiler::handleSetCommand(const String& args, int line_number) {
    int space = args.indexOf(' ');
    if (space < 0) {
        Serial.print("ERROR: Invalid SET command at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    String var_name = args.substring(0, space);
    String expression = args.substring(space + 1);
    var_name.trim();
    expression.trim();

    if (!validateVariableName(var_name)) {
        Serial.print("ERROR: Invalid variable name '");
        Serial.print(var_name);
        Serial.print("' at line ");
        Serial.print(line_number);
        compile_error = true;
        return;
    }

    if (is_array.find(var_name) != is_array.end()) {
        Serial.print("ERROR: Use 'array set' for array elements at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }

    XenoDataType exprType = compileExpressionWithType(expression);
    if (compile_error) return;

    auto it = variable_map.find(var_name);
    bool var_exists = (it != variable_map.end());
    XenoDataType varType;
    if (var_exists) {
        varType = it->second.type;
    } else {
        varType = exprType;
    }

    if (var_exists) {
        if (!isCompatibleForAssignment(varType, exprType)) {
            Serial.print("ERROR: Type mismatch in assignment to variable '");
            Serial.print(var_name);
            Serial.print("'. Expected ");
            Serial.print(varType == TYPE_INT ? "INT" : (varType == TYPE_FLOAT ? "FLOAT" : "OTHER"));
            Serial.print(", got ");
            Serial.println(exprType == TYPE_INT ? "INT" : (exprType == TYPE_FLOAT ? "FLOAT" : "OTHER"));
            compile_error = true;
            return;
        }
        if (varType == TYPE_FLOAT && exprType == TYPE_INT) {
            emitInstruction(OP_CONVERT_TO_FLOAT);
        }
    } else {
        variable_map[var_name] = createValueFromString("0", varType);
    }

    int var_index = getVariableIndex(var_name);
    emitInstruction(OP_STORE, var_index);
}

void XenoCompiler::handleAnalogWrite(const String& args, int line_number) {
    int space = args.indexOf(' ');
    if (space < 0) {
        Serial.print("ERROR: analogWrite requires pin and value at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    String pinStr = args.substring(0, space);
    String valStr = args.substring(space + 1);
    pinStr.trim();
    valStr.trim();
    if (!isInteger(pinStr)) {
        Serial.print("ERROR: analogWrite pin must be integer at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    uint8_t pin = pinStr.toInt();
    if (!security.isPinAllowed(pin)) {
        Serial.print("ERROR: Pin not allowed at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }

    XenoDataType valType = compileExpressionWithType(valStr);
    if (compile_error) return;
    if (!isNumericType(valType)) {
        Serial.print("ERROR: analogWrite value must be numeric at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }

    emitInstruction(OP_ANALOG_WRITE, pin);
}

void XenoCompiler::handleArrayCommand(const String& args, int line_number) {
    int firstSpace = args.indexOf(' ');
    if (firstSpace < 0) {
        Serial.print("ERROR: Invalid array command at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    String subCmd = args.substring(0, firstSpace);
    subCmd.toLowerCase();
    String rest = args.substring(firstSpace + 1);
    rest.trim();

    if (subCmd == "new") {
        int secondSpace = rest.indexOf(' ');
        if (secondSpace < 0) {
            Serial.print("ERROR: array new requires name and size at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        String var_name = rest.substring(0, secondSpace);
        String sizeStr = rest.substring(secondSpace + 1);
        var_name.trim();
        sizeStr.trim();
        if (!validateVariableName(var_name)) {
            Serial.print("ERROR: Invalid array name at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        if (!isInteger(sizeStr)) {
            Serial.print("ERROR: Array size must be integer at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        int size = sizeStr.toInt();
        if (size < 0 || size > 1024) {
            Serial.print("ERROR: Array size out of range at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        emitInstruction(OP_PUSH, static_cast<uint32_t>(size));
        emitInstruction(OP_ARRAY_NEW);
        int var_index = getVariableIndex(var_name);
        emitInstruction(OP_STORE, var_index);
        is_array[var_name] = true;
        variable_map[var_name] = XenoValue::makeArray(0);
    }
    else if (subCmd == "set") {
        int first = rest.indexOf(' ');
        if (first < 0) {
            Serial.print("ERROR: array set requires name, index and value at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        String var_name = rest.substring(0, first);
        String rest2 = rest.substring(first + 1);
        rest2.trim();
        int second = rest2.indexOf(' ');
        if (second < 0) {
            Serial.print("ERROR: array set requires index and value at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        String indexStr = rest2.substring(0, second);
        String valueStr = rest2.substring(second + 1);
        indexStr.trim();
        valueStr.trim();
        if (!validateVariableName(var_name)) {
            Serial.print("ERROR: Invalid array name at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        if (!isInteger(indexStr)) {
            Serial.print("ERROR: Array index must be integer at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        int index = indexStr.toInt();
        auto it = variable_map.find(var_name);
        if (it == variable_map.end() || it->second.type != TYPE_ARRAY) {
            Serial.print("ERROR: Variable '");
            Serial.print(var_name);
            Serial.println("' is not an array");
            compile_error = true;
            return;
        }
        int var_index = getVariableIndex(var_name);
        emitInstruction(OP_LOAD, var_index);
        emitInstruction(OP_PUSH, static_cast<uint32_t>(index));
        compileExpression(valueStr);
        if (compile_error) return;
        emitInstruction(OP_ARRAY_SET);
    }
    else if (subCmd == "get") {
        Serial.print("ERROR: 'array get' should be used inside expressions, not as a command at line ");
        Serial.println(line_number);
        compile_error = true;
    }
    else if (subCmd == "len") {
        String var_name = rest;
        var_name.trim();
        if (!validateVariableName(var_name)) {
            Serial.print("ERROR: Invalid array name at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        auto it = variable_map.find(var_name);
        if (it == variable_map.end() || it->second.type != TYPE_ARRAY) {
            Serial.print("ERROR: Variable '");
            Serial.print(var_name);
            Serial.println("' is not an array");
            compile_error = true;
            return;
        }
        int var_index = getVariableIndex(var_name);
        emitInstruction(OP_LOAD, var_index);
        emitInstruction(OP_ARRAY_LEN);
    }
    else {
        Serial.print("ERROR: Unknown array subcommand at line ");
        Serial.println(line_number);
        compile_error = true;
    }
}

void XenoCompiler::handleAnalogRead(const String& args, int line_number) {
    String pinStr = args;
    pinStr.trim();
    if (!isInteger(pinStr)) {
        Serial.print("ERROR: analogRead requires pin number at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    uint8_t pin = pinStr.toInt();
    if (!security.isPinAllowed(pin)) {
        Serial.print("ERROR: Pin not allowed at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    emitInstruction(OP_ANALOG_READ, pin);
}

void XenoCompiler::handleDigitalRead(const String& args, int line_number) {
    String pinStr = args;
    pinStr.trim();
    if (!isInteger(pinStr)) {
        Serial.print("ERROR: digitalRead requires pin number at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    uint8_t pin = pinStr.toInt();
    if (!security.isPinAllowed(pin)) {
        Serial.print("ERROR: Pin not allowed at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }
    emitInstruction(OP_DIGITAL_READ, pin);
}

// ------------------------------------------------------------------
// Остальные методы (validateString, etc.) без изменений
// ------------------------------------------------------------------

bool XenoCompiler::validateString(const String& str) {
    if (str.length() > security_config.getMaxStringLength()) {
        Serial.println("ERROR: String too long");
        return false;
    }
    return true;
}

bool XenoCompiler::validateVariableName(const String& name) {
    if (name.length() > security_config.getMaxVariableNameLength()) {
        Serial.println("ERROR: Variable name too long");
        return false;
    }
    if (!isValidVariable(name)) {
        Serial.println("ERROR: Invalid variable name");
        return false;
    }
    return true;
}

String XenoCompiler::cleanLine(const String& line) {
    String cleaned = line;
    int commentIndex = cleaned.indexOf("//");
    if (commentIndex >= 0) {
        cleaned = cleaned.substring(0, commentIndex);
    }
    cleaned.trim();
    return cleaned;
}

int XenoCompiler::addString(const String& str) {
    if (!validateString(str)) {
        return 0;
    }

    for (int i = string_table.size() - 1; i >= 0; --i) {
        if (string_table[i] == str) return i;
    }

    if (string_table.size() >= 65535) {
        Serial.println("ERROR: String table overflow");
        return 0;
    }

    string_table.push_back(str);
    return string_table.size() - 1;
}

int XenoCompiler::getVariableIndex(const String& var_name) {
    return validateVariableName(var_name) ? addString(var_name) : 0;
}

bool XenoCompiler::isInteger(const String& str) {
    if (str.isEmpty() || str.length() > 16) return false;

    const char* cstr = str.c_str();
    size_t start = 0;
    if (cstr[0] == '-') start = 1;

    for (size_t i = start; i < str.length(); ++i) {
        if (!isdigit(cstr[i])) return false;
    }

    long long_val = str.toInt();
    return !(long_val > 2147483647L || long_val < -2147483648L);
}

bool XenoCompiler::isFloat(const String& str) {
    if (str.isEmpty() || str.length() > 32) return false;

    const char* cstr = str.c_str();
    bool has_decimal = false;
    size_t start = 0;

    if (cstr[0] == '-') start = 1;

    for (size_t i = start; i < str.length(); ++i) {
        if (cstr[i] == '.') {
            if (has_decimal) return false;
            has_decimal = true;
        } else if (!isdigit(cstr[i])) {
            return false;
        }
    }
    return has_decimal && str.length() > 1;
}

bool XenoCompiler::isBool(const String& str) {
    return str == "true" || str == "false";
}

bool XenoCompiler::isQuotedString(const String& str) {
    return str.length() >= 2 &&
           str[0] == '"' &&
           str[str.length() - 1] == '"';
}

bool XenoCompiler::isValidVariable(const String& str) {
    if (str.isEmpty() || str.length() > security_config.getMaxVariableNameLength()) return false;

    const char first = str[0];
    if (!isalpha(first) && first != '_') return false;

    for (size_t i = 1; i < str.length(); ++i) {
        const char c = str[i];
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool XenoCompiler::isComparisonOperator(const String& str) {
    return str == "==" || str == "!=" ||
           str == "<"  || str == ">"  ||
           str == "<=" || str == ">=";
}

int XenoCompiler::getPrecedence(const String& op) {
    if (op == "^") return 4;
    if (op == "*" || op == "/" || op == "%") return 3;
    if (op == "+" || op == "-") return 2;
    if (isComparisonOperator(op)) return 1;
    if (op == "&&") return 2;
    if (op == "||") return 1;
    // Запятая имеет самый низкий приоритет
    if (op == ",") return 0;
    return 0;
}

bool XenoCompiler::isRightAssociative(const String& op) {
    return op == "^";
}

String XenoCompiler::processFunctions(const String& expr) {
    if (expr.length() > 1024) {
        Serial.println("ERROR: Expression too long");
        return expr;
    }

    String result = expr;
    int depth = 0;

    processConstants(result);

    for (size_t i = 0; i < math_functions_count && depth < security_config.getMaxExpressionDepth(); i++) {
        const MathFunctionInfo& func = math_functions[i];
        int pos = result.indexOf(func.name);

        while (pos >= 0 && depth < security_config.getMaxExpressionDepth()) {
            bool isolated = true;
            if (pos > 0) {
                char prev = result[pos - 1];
                if (isalnum(prev) || prev == '_') {
                    isolated = false;
                }
            }
            if (!isolated) {
                pos = result.indexOf(func.name, pos + 1);
                continue;
            }

            int endPos = findMatchingParenthesis(result, pos + strlen(func.name) - 1);
            if (endPos > pos) {
                String inner = result.substring(pos + strlen(func.name), endPos);
                inner = processFunctions(inner);
                result = result.substring(0, pos)
                        + String(func.open_bracket) + inner + String(func.close_bracket)
                        + result.substring(endPos + 1);
            } else {
                break;
            }
            pos = result.indexOf(func.name);
            depth++;
        }
    }

    if (depth >= security_config.getMaxExpressionDepth()) {
        Serial.println("ERROR: Expression too complex");
    }

    return result;
}

int XenoCompiler::findMatchingParenthesis(const String& expr, int start) {
    int count = 1;
    for (int i = start + 1; i < expr.length(); ++i) {
        if (expr[i] == '(') ++count;
        else if (expr[i] == ')') --count;

        if (count == 0) return i;
    }
    return -1;
}

// ---- НОВАЯ ВЕРСИЯ infixToPostfix с поддержкой вызовов функций ----
std::vector<String> XenoCompiler::infixToPostfix(const std::vector<String>& tokens) {
    std::vector<String> output;
    std::stack<String> operators;
    output.reserve(tokens.size());

    if (tokens.size() > 100) {
        Serial.println("ERROR: Too many tokens in expression");
        return output;
    }

    bool expect_operand = true;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const String& token = tokens[i];

        // ---- ОПЕРАНДЫ ----
        if (isInteger(token) || isFloat(token) || isBool(token) || isQuotedString(token) ||
            isValidVariable(token) ||
            (token.startsWith("[") && token.endsWith("]")) ||
            (token.startsWith("{") && token.endsWith("}")) ||
            (token.startsWith("|") && token.endsWith("|")) ||
            (token.startsWith("~") && token.endsWith("~"))) {
            // Проверяем, не является ли это именем функции, за которым следует '('
            if (isValidVariable(token) && functions.find(token) != functions.end() &&
                i + 1 < tokens.size() && tokens[i + 1] == "(") {
                // Это вызов функции: помещаем маркер в стек операторов
                operators.push("FUNC:" + token);
                // Пропускаем '(' (она не нужна, так как маркер выполняет её роль)
                ++i;
                // После маркера ожидаем аргументы
                expect_operand = true;
                continue;
            }
            // Обычный операнд
            output.push_back(token);
            expect_operand = false;
        }
        // ---- ОТКРЫВАЮЩАЯ СКОБКА (не связанная с функцией) ----
        else if (token == "(") {
            operators.push(token);
            expect_operand = true;
        }
        // ---- ЗАКРЫВАЮЩАЯ СКОБКА ----
        else if (token == ")") {
            // Выталкиваем операторы до маркера функции или до '('
            while (!operators.empty() && operators.top() != "(" &&
                   !operators.top().startsWith("FUNC:")) {
                output.push_back(operators.top());
                operators.pop();
            }
            if (operators.empty()) {
                Serial.println("ERROR: Mismatched parentheses");
                compile_error = true;
                return output;
            }
            if (operators.top().startsWith("FUNC:")) {
                // Это закрытие вызова функции
                String funcMarker = operators.top();
                operators.pop();
                String funcName = funcMarker.substring(5); // после "FUNC:"
                // Генерируем CALL токен
                output.push_back("CALL:" + funcName);
                expect_operand = false;
            } else {
                // Обычная скобка
                operators.pop();
                expect_operand = false;
            }
        }
        // ---- ЗАПЯТАЯ (разделитель аргументов) ----
        else if (token == ",") {
            // Выталкиваем операторы до ближайшего маркера функции или '('
            while (!operators.empty() && operators.top() != "(" &&
                   !operators.top().startsWith("FUNC:")) {
                output.push_back(operators.top());
                operators.pop();
            }
            if (operators.empty() ||
                (operators.top() != "(" && !operators.top().startsWith("FUNC:"))) {
                Serial.println("ERROR: Comma outside function call");
                compile_error = true;
                return output;
            }
            expect_operand = true;
        }
        // ---- УНАРНЫЕ ОПЕРАТОРЫ ----
        else if (token == "!" || token == "-") {
            if (expect_operand) {
                operators.push(token == "!" ? "UNARY_NOT" : "UNARY_NEG");
            } else {
                int token_precedence = getPrecedence(token);
                while (!operators.empty() &&
                        operators.top() != "(" &&
                        !operators.top().startsWith("FUNC:") &&
                        (getPrecedence(operators.top()) > token_precedence ||
                        (getPrecedence(operators.top()) == token_precedence &&
                        !isRightAssociative(token))))  {
                    output.push_back(operators.top());
                    operators.pop();
                }
                operators.push(token);
                expect_operand = true;
            }
        }
        // ---- БИНАРНЫЕ ОПЕРАТОРЫ ----
        else {
            int token_precedence = getPrecedence(token);
            while (!operators.empty() &&
                    operators.top() != "(" &&
                    !operators.top().startsWith("FUNC:") &&
                    (getPrecedence(operators.top()) > token_precedence ||
                    (getPrecedence(operators.top()) == token_precedence &&
                    !isRightAssociative(token))))  {
                output.push_back(operators.top());
                operators.pop();
            }
            operators.push(token);
            expect_operand = true;
        }
    }

    // Выталкиваем оставшиеся операторы
    while (!operators.empty()) {
        if (operators.top() == "(" || operators.top().startsWith("FUNC:")) {
            Serial.println("ERROR: Mismatched parentheses or function call");
            compile_error = true;
            return output;
        }
        output.push_back(operators.top());
        operators.pop();
    }

    return output;
}

std::vector<String> XenoCompiler::tokenizeExpression(const String& expr) {
    std::vector<String> tokens;
    String currentToken;
    bool inQuotes = false;
    bool inSpecial = false;
    char specialChar = 0;
    tokens.reserve(expr.length() / 2);

    if (expr.length() > 1024) {
        Serial.println("ERROR: Expression too long");
        return tokens;
    }

    for (size_t i = 0; i < expr.length(); ++i) {
        char c = expr[i];

        if (c == '"' && !inSpecial) {
            if (inQuotes) {
                currentToken += c;
                if (!validateString(currentToken)) {
                    currentToken = "\"\"";
                }
                tokens.push_back(currentToken);
                currentToken = "";
                inQuotes = false;
            } else {
                if (!currentToken.isEmpty()) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                inQuotes = true;
                currentToken += c;
            }
            continue;
        }

        if (inQuotes) {
            currentToken += c;
            continue;
        }

        if ((c == '[' || c == '{' || c == '|' || c == '~' || c == '#' || c == '@' || c == '&') && !inSpecial) {
            if (!currentToken.isEmpty()) {
                tokens.push_back(currentToken);
                currentToken = "";
            }
            inSpecial = true;
            specialChar = c;
            currentToken += c;
            continue;
        } else if (inSpecial && c == specialChar) {
            currentToken += c;
            tokens.push_back(currentToken);
            currentToken = "";
            inSpecial = false;
            specialChar = 0;
            continue;
        }

        if (inSpecial) {
            currentToken += c;
            continue;
        }

        if (isspace(c)) {
            if (!currentToken.isEmpty()) {
                tokens.push_back(currentToken);
                currentToken = "";
            }
            continue;
        }

        if (i + 1 < expr.length()) {
            String twoChar = expr.substring(i, i + 2);
            if (twoChar == "==" || twoChar == "!=" ||
                twoChar == "<=" || twoChar == ">=" ||
                twoChar == "&&" || twoChar == "||") {
                if (!currentToken.isEmpty()) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                tokens.push_back(twoChar);
                ++i;
                continue;
            }
        }

        // Добавляем запятую как отдельный токен
        if (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '%' || c == '^' || c == '<' || c == '>' ||
            c == '(' || c == ')' || c == '!' || c == '[' || c == ']' ||
            c == ',') {
            if (!currentToken.isEmpty()) {
                tokens.push_back(currentToken);
                currentToken = "";
            }
            tokens.push_back(String(c));
        } else {
            currentToken += c;
        }
    }

    if (!currentToken.isEmpty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

// ------------------------------------------------------------------
// Парсинг функций (без изменений)
// ------------------------------------------------------------------

void XenoCompiler::parseFunctionDeclaration(const String& args, int line_number) {
    int openParen = args.indexOf('(');
    int closeParen = args.lastIndexOf(')');
    if (openParen <= 0 || closeParen <= openParen) {
        Serial.print("ERROR: Invalid function declaration at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }

    String funcName = args.substring(0, openParen);
    funcName.trim();
    if (!validateVariableName(funcName)) {
        Serial.print("ERROR: Invalid function name '");
        Serial.print(funcName);
        Serial.print("' at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }

    if (functions.find(funcName) != functions.end()) {
        Serial.print("ERROR: Function '");
        Serial.print(funcName);
        Serial.print("' already defined at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }

    String paramsStr = args.substring(openParen + 1, closeParen);
    paramsStr.trim();
    std::vector<String> parameters;
    if (!paramsStr.isEmpty()) {
        int start = 0;
        while (start < paramsStr.length()) {
            int comma = paramsStr.indexOf(',', start);
            String param;
            if (comma >= 0) {
                param = paramsStr.substring(start, comma);
                start = comma + 1;
            } else {
                param = paramsStr.substring(start);
                start = paramsStr.length();
            }
            param.trim();
            if (!param.isEmpty()) {
                if (!validateVariableName(param)) {
                    Serial.print("ERROR: Invalid parameter name '");
                    Serial.print(param);
                    Serial.print("' at line ");
                    Serial.println(line_number);
                    compile_error = true;
                    return;
                }
                for (const String& p : parameters) {
                    if (p == param) {
                        Serial.print("ERROR: Duplicate parameter name '");
                        Serial.print(param);
                        Serial.print("' at line ");
                        Serial.println(line_number);
                        compile_error = true;
                        return;
                    }
                }
                parameters.push_back(param);
            }
        }
    }

    FunctionInfo funcInfo;
    funcInfo.name = funcName;
    funcInfo.parameters = parameters;
    funcInfo.arity = parameters.size();
    funcInfo.address = getCurrentAddress();

    functions[funcName] = funcInfo;

    inside_function_declaration = true;
    pending_function = funcInfo;
}

// ------------------------------------------------------------------
// Главный метод компиляции строки (без изменений, кроме обработки func)
// ------------------------------------------------------------------

void XenoCompiler::compileLine(const String& line, int line_number) {
    String cleanedLine = cleanLine(line);
    if (cleanedLine.isEmpty()) return;

    if (cleanedLine.length() > 512) {
        Serial.print("ERROR: Line too long at line ");
        Serial.println(line_number);
        compile_error = true;
        return;
    }

    // ---- Если внутри объявления функции, обрабатываем только endfunc ----
    if (inside_function_declaration) {
        if (cleanedLine.equalsIgnoreCase("endfunc")) {
            inside_function_declaration = false;
            return;
        } else {
            return;
        }
    }

    // ---- Обработка команды func ----
    if (cleanedLine.startsWith("func ")) {
        String args = cleanedLine.substring(5);
        args.trim();
        parseFunctionDeclaration(args, line_number);
        return;
    }

    // ---- Обычные команды (если не внутри функции) ----
    int firstSpace = cleanedLine.indexOf(' ');
    String command = (firstSpace > 0) ? cleanedLine.substring(0, firstSpace) : cleanedLine;
    String args = (firstSpace > 0) ? cleanedLine.substring(firstSpace + 1) : "";
    args.trim();
    command.toLowerCase();

    bool simple_command_found = false;
    for (size_t i = 0; i < simple_commands_count; i++) {
        if (command == simple_commands[i].name) {
            emitInstruction(simple_commands[i].opcode);
            simple_command_found = true;
            break;
        }
    }

    if (simple_command_found) {
        return;
    }

    if (command == "array") {
        handleArrayCommand(args, line_number);
        return;
    }
    if (command == "analogread") {
        handleAnalogRead(args, line_number);
        return;
    }
    if (command == "analogwrite") {
        handleAnalogWrite(args, line_number);
        return;
    }
    if (command == "digitalread") {
        handleDigitalRead(args, line_number);
        return;
    }

    if (command == "print") {
        String text = args;
        String var_name = extractVariableName(text);
        if (!var_name.isEmpty()) {
            if (isValidVariable(var_name)) {
                int var_index = getVariableIndex(var_name);
                emitInstruction(OP_LOAD, var_index);
                emitInstruction(OP_PRINT_NUM);
            } else {
                Serial.print("ERROR: Invalid variable name in print at line ");
                Serial.println(line_number);
                compile_error = true;
            }
        } else {
            if (text.startsWith("\"") && text.endsWith("\"")) {
                text = text.substring(1, text.length() - 1);
            }
            if (!validateString(text)) {
                text = "";
            }
            int str_id = addString(text);
            emitInstruction(OP_PRINT, str_id);
        }
    } else if (command == "led") {
        int spaceIndex = args.indexOf(' ');
        if (spaceIndex > 0) {
            String pin_str = args.substring(0, spaceIndex);
            String state_str = args.substring(spaceIndex + 1);
            state_str.trim();
            state_str.toLowerCase();

            int pin = pin_str.toInt();
            if (pin < 0 || pin > 255) {
                Serial.print("ERROR: Invalid pin number at line ");
                Serial.println(line_number);
                compile_error = true;
                return;
            }

            if (state_str == "on" || state_str == "1" || state_str == "true") {
                emitInstruction(OP_LED_ON, pin);
            } else if (state_str == "off" || state_str == "0" || state_str == "false") {
                emitInstruction(OP_LED_OFF, pin);
            } else {
                Serial.print("WARNING: Unknown LED state at line ");
                Serial.println(line_number);
            }
        } else {
            Serial.print("WARNING: Invalid LED command at line ");
            Serial.println(line_number);
        }
    } else if (command == "delay") {
        int delay_time = args.toInt();
        if (delay_time < 0 || delay_time > 60000) {
            Serial.print("WARNING: Delay time out of range at line ");
            Serial.println(line_number);
            delay_time = min(max(delay_time, 0), 60000);
        }
        emitInstruction(OP_DELAY, delay_time);
    } else if (command == "push") {
        if (isValidVariable(args)) {
            int var_index = getVariableIndex(args);
            emitInstruction(OP_LOAD, var_index);
        } else if (isFloat(args)) {
            float fval = args.toFloat();
            uint32_t fbits;
            memcpy(&fbits, &fval, sizeof(float));
            emitInstruction(OP_PUSH_FLOAT, fbits);
        } else if (isBool(args)) {
            bool bval = (args == "true");
            emitInstruction(OP_PUSH_BOOL, bval);
        } else if (isQuotedString(args)) {
            String str = args.substring(1, args.length() - 1);
            if (!validateString(str)) {
                str = "";
            }
            int str_id = addString(str);
            emitInstruction(OP_PUSH_STRING, str_id);
        } else {
            int32_t value = args.toInt();
            emitInstruction(OP_PUSH, static_cast<uint32_t>(value));
        }
    } else if (command == "input") {
        String var_name = args;
        if (!validateVariableName(var_name)) {
            Serial.print("ERROR: Invalid variable name for input at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        int var_index = getVariableIndex(var_name);
        emitInstruction(OP_INPUT, var_index);
    } else if (command == "set") {
        handleSetCommand(args, line_number);
    } else if (command == "if") {
        if (if_chain_stack.size() >= security_config.getMaxIfDepth()) {
            Serial.print("ERROR: IF nesting too deep at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }

        int thenPos = args.indexOf(" then");
        if (thenPos > 0) {
            String condition = args.substring(0, thenPos);
            compileExpression(condition);

            int jump_addr = getCurrentAddress();
            emitInstruction(OP_JUMP_IF, 0);
            IfContext ctx;
            ctx.if_jumps.push_back(jump_addr);
            if_chain_stack.push_back(ctx);
        } else {
            Serial.print("ERROR: Invalid IF command at line ");
            Serial.println(line_number);
            compile_error = true;
        }
    } else if (command == "else") {
        if (if_chain_stack.empty()) {
            Serial.print("ERROR: ELSE without IF at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }

        String trimmedArgs = args;
        trimmedArgs.trim();
        if (trimmedArgs.startsWith("if")) {
            int jump_else = getCurrentAddress();
            emitInstruction(OP_JUMP, 0);
            IfContext& ctx = if_chain_stack.back();
            if (!ctx.if_jumps.empty()) {
                int last_if = ctx.if_jumps.back();
                bytecode[last_if].arg1 = getCurrentAddress();
            } else {
                Serial.print("ERROR: No if jump to fix at line ");
                Serial.println(line_number);
                compile_error = true;
                return;
            }
            ctx.else_jumps.push_back(jump_else);
            String rest = trimmedArgs.substring(2);
            rest.trim();
            int thenPos = rest.indexOf(" then");
            if (thenPos > 0) {
                String condition = rest.substring(0, thenPos);
                compileExpression(condition);
                int jump_addr = getCurrentAddress();
                emitInstruction(OP_JUMP_IF, 0);
                ctx.if_jumps.push_back(jump_addr);
            } else {
                Serial.print("ERROR: Invalid ELSE IF command at line ");
                Serial.println(line_number);
                compile_error = true;
            }
        } else {
            int jump_else = getCurrentAddress();
            emitInstruction(OP_JUMP, 0);
            IfContext& ctx = if_chain_stack.back();
            if (!ctx.if_jumps.empty()) {
                int last_if = ctx.if_jumps.back();
                bytecode[last_if].arg1 = getCurrentAddress();
            } else {
                Serial.print("ERROR: No if jump to fix at line ");
                Serial.println(line_number);
                compile_error = true;
                return;
            }
            ctx.else_jumps.push_back(jump_else);
        }
    } else if (command == "endif") {
        if (if_chain_stack.empty()) {
            Serial.print("ERROR: ENDIF without IF at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }
        IfContext ctx = if_chain_stack.back();
        if_chain_stack.pop_back();
        int end_addr = getCurrentAddress();
        for (int addr : ctx.if_jumps) {
            if (addr < bytecode.size()) {
                bytecode[addr].arg1 = end_addr;
            }
        }
        for (int addr : ctx.else_jumps) {
            if (addr < bytecode.size()) {
                bytecode[addr].arg1 = end_addr;
            }
        }
    } else if (command == "while") {
        if (while_stack.size() >= security_config.getMaxLoopDepth()) {
            Serial.print("ERROR: While loop nesting too deep at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }

        int loop_start = getCurrentAddress();
        compileExpression(args);
        if (compile_error) return;

        int jump_if_addr = getCurrentAddress();
        emitInstruction(OP_JUMP_IF, 0);

        LoopInfo info;
        info.start_address = loop_start;
        info.condition_address = jump_if_addr;
        while_stack.push_back(info);
    } else if (command == "endwhile") {
        if (while_stack.empty()) {
            Serial.print("ERROR: ENDWHILE without WHILE at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }

        LoopInfo info = while_stack.back();
        while_stack.pop_back();

        emitInstruction(OP_JUMP, info.start_address);

        int end_addr = getCurrentAddress();
        if (info.condition_address < bytecode.size()) {
            bytecode[info.condition_address].arg1 = end_addr;
        } else {
            Serial.print("ERROR: Invalid condition address in ENDWHILE at line ");
            Serial.println(line_number);
            compile_error = true;
        }
    } else if (command == "for") {
        if (loop_stack.size() >= security_config.getMaxLoopDepth()) {
            Serial.print("ERROR: Loop nesting too deep at line ");
            Serial.println(line_number);
            compile_error = true;
            return;
        }

        int equalsPos = args.indexOf('=');
        int toPos = args.indexOf(" to ");

        if (equalsPos > 0 && toPos > equalsPos) {
            String var_name = args.substring(0, equalsPos);
            var_name.trim();

            if (!validateVariableName(var_name)) {
                Serial.print("ERROR: Invalid variable name in FOR at line ");
                Serial.println(line_number);
                compile_error = true;
                return;
            }

            String start_expr = args.substring(equalsPos + 1, toPos);
            start_expr.trim();
            String end_expr = args.substring(toPos + 4);
            end_expr.trim();

            XenoDataType startType = compileExpressionWithType(start_expr);
            if (compile_error) return;
            if (!isNumericType(startType)) {
                Serial.print("ERROR: FOR start value must be numeric at line ");
                Serial.println(line_number);
                compile_error = true;
                return;
            }
            XenoDataType endType = compileExpressionWithType(end_expr);
            if (compile_error) return;
            if (!isNumericType(endType)) {
                Serial.print("ERROR: FOR end value must be numeric at line ");
                Serial.println(line_number);
                compile_error = true;
                return;
            }

            int var_index = getVariableIndex(var_name);
            compileExpression(start_expr);
            emitInstruction(OP_STORE, var_index);

            int loop_start = getCurrentAddress();
            emitInstruction(OP_LOAD, var_index);
            compileExpression(end_expr);
            emitInstruction(OP_LTE);

            int condition_jump = getCurrentAddress();
            emitInstruction(OP_JUMP_IF, 0);

            LoopInfo loop_info;
            loop_info.var_name = var_name;
            loop_info.start_address = loop_start;
            loop_info.condition_address = condition_jump;
            loop_info.end_jump_address = getCurrentAddress();
            loop_stack.push_back(loop_info);
        } else {
            Serial.print("ERROR: Invalid FOR command at line ");
            Serial.println(line_number);
            compile_error = true;
        }
    } else if (command == "endfor") {
        if (!loop_stack.empty()) {
            LoopInfo loop_info = loop_stack.back();
            loop_stack.pop_back();

            emitInstruction(OP_LOAD, getVariableIndex(loop_info.var_name));
            auto var_it = variable_map.find(loop_info.var_name);
            if (var_it != variable_map.end() &&
                var_it->second.type == TYPE_FLOAT) {
                float increment = 1.0f;
                uint32_t increment_bits;
                memcpy(&increment_bits, &increment, sizeof(float));
                emitInstruction(OP_PUSH_FLOAT, increment_bits);
            } else {
                emitInstruction(OP_PUSH, 1);
            }
            emitInstruction(OP_ADD);
            emitInstruction(OP_STORE, getVariableIndex(loop_info.var_name));
            emitInstruction(OP_JUMP, loop_info.start_address);

            if (loop_info.condition_address < bytecode.size()) {
                bytecode[loop_info.condition_address].arg1 = getCurrentAddress();
            }
        } else {
            Serial.print("ERROR: ENDFOR without FOR at line ");
            Serial.println(line_number);
            compile_error = true;
        }
    } else {
        Serial.print("WARNING: Unknown command at line ");
        Serial.print(line_number);
        Serial.print(": ");
        Serial.println(command);
    }
}

XenoDataType XenoCompiler::determineValueType(const String& value) {
    if (isQuotedString(value)) return TYPE_STRING;
    if (isFloat(value)) return TYPE_FLOAT;
    if (isInteger(value)) return TYPE_INT;
    if (isBool(value)) return TYPE_BOOL;
    if (isValidVariable(value)) {
        auto it = variable_map.find(value);
        if (it != variable_map.end()) {
            return it->second.type;
        }
        if (is_array.find(value) != is_array.end()) {
            return TYPE_ARRAY;
        }
        return TYPE_INT;
    }
    return TYPE_INT;
}

XenoValue XenoCompiler::createValueFromString(const String& str, XenoDataType type) {
    XenoValue value;
    value.type = type;

    switch (type) {
        case TYPE_INT:
            value.int_val = str.toInt();
            break;
        case TYPE_FLOAT:
            value.float_val = str.toFloat();
            break;
        case TYPE_STRING:
            value.string_index = addString(
                str.substring(1, str.length() - 1));
            break;
        case TYPE_BOOL:
            value.bool_val = (str == "true");
            break;
        case TYPE_ARRAY:
            break;
    }
    return value;
}

void XenoCompiler::emitInstruction(uint8_t opcode, uint32_t arg1, uint16_t arg2) {
    if (bytecode.size() >= 65535) {
        Serial.println("ERROR: Program too large");
        compile_error = true;
        return;
    }
    bytecode.emplace_back(opcode, arg1, arg2);
}

int XenoCompiler::getCurrentAddress() {
    return bytecode.size();
}

void XenoCompiler::processConstants(String& expr) {
    int pos = 0;
    while (pos < expr.length()) {
        if (expr[pos] == 'M' || expr[pos] == 'P') {
            int start_pos = pos;
            for (size_t i = 0; i < constants_count; i++) {
                const char* name = constants[i].name;
                size_t name_len = strlen(name);
                if (pos + name_len <= expr.length()) {
                    bool match = true;
                    for (size_t j = 0; j < name_len; j++) {
                        if (expr[pos + j] != name[j]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        bool is_isolated = true;
                        if (start_pos > 0) {
                            char prev_char = expr[start_pos - 1];
                            if (isalnum(prev_char) || prev_char == '_') {
                                is_isolated = false;
                            }
                        }
                        if (start_pos + name_len < expr.length()) {
                            char next_char = expr[start_pos + name_len];
                            if (isalnum(next_char) || next_char == '_') {
                                is_isolated = false;
                            }
                        }
                        if (is_isolated) {
                            const char* value = constants[i].value;
                            size_t value_len = strlen(value);
                            expr = expr.substring(0, start_pos) +
                                   value +
                                   expr.substring(start_pos + name_len);
                            pos = start_pos + value_len;
                            break;
                        }
                    }
                }
            }
        }
        pos++;
    }
}

String XenoCompiler::extractVariableName(const String& text) {
    return text.startsWith("$") ? text.substring(1) : "";
}

const std::vector<XenoInstruction>& XenoCompiler::getBytecode() const { return bytecode; }
const std::vector<String>& XenoCompiler::getStringTable() const { return string_table; }

void XenoCompiler::printCompiledCode() {
    Debugger::disassemble(bytecode, string_table, "Compiled Xeno Program", true);
}