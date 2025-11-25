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

const XenoCompiler::FunctionInfo XenoCompiler::math_functions[] = {
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

void XenoCompiler::compileMathFunction(const String& token, const FunctionInfo& func) {
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
        }
    }
}

void XenoCompiler::compileSimpleCommand(const String& command, uint8_t opcode) {
    emitInstruction(opcode);
}

bool XenoCompiler::validateString(const String& str) {
    if (str.length() > security_config.MAX_STRING_LENGTH) {
        Serial.println("ERROR: String too long");
        return false;
    }
    return true;
}

bool XenoCompiler::validateVariableName(const String& name) {
    if (name.length() > security_config.MAX_VARIABLE_NAME_LENGTH) {
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
    if (str.isEmpty() || str.length() > security_config.MAX_VARIABLE_NAME_LENGTH) return false;

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

    for (size_t i = 0; i < math_functions_count && depth < security_config.MAX_EXPRESSION_DEPTH; i++) {
        const FunctionInfo& func = math_functions[i];
        int pos = result.indexOf(func.name);
        
        while (pos >= 0 && depth < security_config.MAX_EXPRESSION_DEPTH) {
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

    if (depth >= security_config.MAX_EXPRESSION_DEPTH) {
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

std::vector<String> XenoCompiler::infixToPostfix(const std::vector<String>& tokens) {
    std::vector<String> output;
    std::stack<String> operators;
    output.reserve(tokens.size());

    if (tokens.size() > 100) {
        Serial.println("ERROR: Too many tokens in expression");
        return output;
    }

    for (const String& token : tokens) {
        if (isInteger(token) || isFloat(token) || isBool(token) || isQuotedString(token) ||
            isValidVariable(token) ||
            (token.startsWith("[") && token.endsWith("]")) ||
            (token.startsWith("{") && token.endsWith("}")) ||
            (token.startsWith("|") && token.endsWith("|")) ||
            (token.startsWith("~") && token.endsWith("~"))) {
            output.push_back(token);
        } else if (token == "(") {
            operators.push(token);
        } else if (token == ")") {
            while (!operators.empty() && operators.top() != "(") {
                output.push_back(operators.top());
                operators.pop();
            }
            if (!operators.empty()) operators.pop();
        } else {
            int token_precedence = getPrecedence(token);
            while (!operators.empty() &&
                    operators.top() != "(" &&
                    (getPrecedence(operators.top()) > token_precedence ||
                    (getPrecedence(operators.top()) == token_precedence &&
                    !isRightAssociative(token))))  {
                output.push_back(operators.top());
                operators.pop();
            }
            operators.push(token);
        }
    }

    while (!operators.empty()) {
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
                twoChar == "<=" || twoChar == ">=")  {
                if (!currentToken.isEmpty()) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                tokens.push_back(twoChar);
                ++i;
                continue;
            }
        }

        if (c == '+' || c == '-' || c == '*' || c == '/' ||
            c == '%' || c == '^' || c == '<' || c == '>' ||
            c == '(' || c == ')') {
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

void XenoCompiler::compilePostfix(const std::vector<String>& postfix) {
    if (postfix.size() > 100) {
        Serial.println("ERROR: Postfix expression too complex");
        return;
    }

    for (const String& token : postfix) {
        if (isInteger(token)) {
            int32_t value = token.toInt();
            emitInstruction(OP_PUSH, static_cast<uint32_t>(value));
        } else if (isFloat(token)) {
            float fval = token.toFloat();
            uint32_t fbits;
            memcpy(&fbits, &fval, sizeof(float));
            emitInstruction(OP_PUSH_FLOAT, fbits);
        } else if (isBool(token)) {
            bool bval = (token == "true");
            emitInstruction(OP_PUSH_BOOL, bval);
        } else if (isQuotedString(token)) {
            String str = token.substring(1, token.length() - 1);
            if (!validateString(str)) str = "";
            int str_id = addString(str);
            emitInstruction(OP_PUSH_STRING, str_id);
        } else if (isValidVariable(token)) {
            int var_index = getVariableIndex(token);
            emitInstruction(OP_LOAD, var_index);
        } else {
            bool function_processed = false;
            for (size_t i = 0; i < math_functions_count; i++) {
                const FunctionInfo& func = math_functions[i];
                if (token.startsWith(String(func.open_bracket)) && 
                    token.endsWith(String(func.close_bracket))) {
                    compileMathFunction(token, func);
                    function_processed = true;
                    break;
                }
            }
            
            if (!function_processed) {
                if (token == "+") emitInstruction(OP_ADD);
                else if (token == "-") emitInstruction(OP_SUB);
                else if (token == "*") emitInstruction(OP_MUL);
                else if (token == "/") emitInstruction(OP_DIV);
                else if (token == "%") emitInstruction(OP_MOD);
                else if (token == "^") emitInstruction(OP_POW);
                else if (token == "==") emitInstruction(OP_EQ);
                else if (token == "!=") emitInstruction(OP_NEQ);
                else if (token == "<") emitInstruction(OP_LT);
                else if (token == ">") emitInstruction(OP_GT);
                else if (token == "<=") emitInstruction(OP_LTE);
                else if (token == ">=") emitInstruction(OP_GTE);
            }
        }
    }
}

void XenoCompiler::compileExpression(const String& expr) {
    if (expr.isEmpty() || expr.length() > 1024) {
        Serial.println("ERROR: Invalid expression");
        return;
    }

    String processedExpr = processFunctions(expr);
    std::vector<String> tokens = tokenizeExpression(processedExpr);
    std::vector<String> postfix = infixToPostfix(tokens);
    compilePostfix(postfix);
}

String XenoCompiler::extractVariableName(const String& text) {
    return text.startsWith("$") ? text.substring(1) : "";
}

XenoDataType XenoCompiler::determineValueType(const String& value) {
    if (isQuotedString(value)) return TYPE_STRING;
    if (isFloat(value)) return TYPE_FLOAT;
    if (isInteger(value)) return TYPE_INT;
    if (isBool(value)) return TYPE_BOOL;
    if (isValidVariable(value)) {
        auto it = variable_map.find(value);
        return it != variable_map.end() ? it->second.type : TYPE_INT;
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
    }
    return value;
}

void XenoCompiler::emitInstruction(uint8_t opcode, uint32_t arg1, uint16_t arg2) {
    if (bytecode.size() >= 65535) {
        Serial.println("ERROR: Program too large");
        return;
    }
    bytecode.emplace_back(opcode, arg1, arg2);
}

int XenoCompiler::getCurrentAddress() {
    return bytecode.size();
}

void XenoCompiler::compileLine(const String& line, int line_number) {
    String cleanedLine = cleanLine(line);
    if (cleanedLine.isEmpty()) return;

    if (cleanedLine.length() > 512) {
        Serial.print("ERROR: Line too long at line ");
        Serial.println(line_number);
        return;
    }

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
            return;
        }
        int var_index = getVariableIndex(var_name);
        emitInstruction(OP_INPUT, var_index);
    } else if (command == "set") {
        int space1 = args.indexOf(' ');
        if (space1 > 0) {
            String var_name = args.substring(0, space1);
            String expression = args.substring(space1 + 1);

            if (!validateVariableName(var_name)) {
                Serial.print("ERROR: Invalid variable name '");
                Serial.print(var_name);
                Serial.print("' at line ");
                Serial.print(line_number);
                return;
            }

            XenoDataType value_type = determineValueType(expression);
            if (isInteger(expression) || isFloat(expression) || isQuotedString(expression) || isBool(expression)) {
                variable_map[var_name] = createValueFromString(expression, value_type);
            }

            compileExpression(expression);
            emitInstruction(OP_STORE, getVariableIndex(var_name));
        } else {
            Serial.print("ERROR: Invalid SET command at line ");
            Serial.println(line_number);
        }
    } else if (command == "if") {
        if (if_stack.size() >= security_config.MAX_IF_DEPTH) {
            Serial.print("ERROR: IF nesting too deep at line ");
            Serial.println(line_number);
            return;
        }

        int thenPos = args.indexOf(" then");
        if (thenPos > 0) {
            String condition = args.substring(0, thenPos);
            compileExpression(condition);

            int jump_addr = getCurrentAddress();
            emitInstruction(OP_JUMP_IF, 0);
            if_stack.push_back(jump_addr);
        } else {
            Serial.print("ERROR: Invalid IF command at line ");
            Serial.println(line_number);
        }
    } else if (command == "else") {
        if (!if_stack.empty()) {
            int else_jump_addr = getCurrentAddress();
            emitInstruction(OP_JUMP, 0);

            int if_jump_addr = if_stack.back();
            if (if_jump_addr < bytecode.size()) {
                bytecode[if_jump_addr].arg1 = getCurrentAddress();
            }

            if_stack.pop_back();
            if_stack.push_back(else_jump_addr);
        } else {
            Serial.print("ERROR: ELSE without IF at line ");
            Serial.println(line_number);
        }
    } else if (command == "endif") {
        if (!if_stack.empty()) {
            int jump_addr = if_stack.back();
            if (jump_addr < bytecode.size()) {
                bytecode[jump_addr].arg1 = getCurrentAddress();
            }
            if_stack.pop_back();
        } else {
            Serial.print("ERROR: ENDIF without IF at line ");
            Serial.println(line_number);
        }
    } else if (command == "for") {
        if (loop_stack.size() >= security_config.MAX_LOOP_DEPTH) {
            Serial.print("ERROR: Loop nesting too deep at line ");
            Serial.println(line_number);
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
                return;
            }

            String start_expr = args.substring(equalsPos + 1, toPos);
            start_expr.trim();
            String end_expr = args.substring(toPos + 4);
            end_expr.trim();

            compileExpression(start_expr);
            int var_index = getVariableIndex(var_name);
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
        }
    } else {
        Serial.print("WARNING: Unknown command at line ");
        Serial.print(line_number);
        Serial.print(": ");
        Serial.println(command);
    }
}

XenoCompiler::XenoCompiler(XenoSecurityConfig& config) 
    : security_config(config), security(config) {
    bytecode.reserve(128);
    string_table.reserve(32);
    if_stack.reserve(security_config.MAX_IF_DEPTH);
    loop_stack.reserve(security_config.MAX_LOOP_DEPTH);
}

void XenoCompiler::compile(const String& source_code) {
    bytecode.clear();
    string_table.clear();
    variable_map.clear();
    if_stack.clear();
    loop_stack.clear();

    int line_number = 0;
    int startPos = 0;
    int endPos = source_code.indexOf('\n');

    while (endPos >= 0) {
        String line = source_code.substring(startPos, endPos);
        ++line_number;

        if (!line.isEmpty()) {
            compileLine(line, line_number);
        }

        startPos = endPos + 1;
        endPos = source_code.indexOf('\n', startPos);
    }

    String lastLine = source_code.substring(startPos);
    if (!lastLine.isEmpty()) {
        compileLine(lastLine, ++line_number);
    }

    if (bytecode.empty() || bytecode.back().opcode != OP_HALT) {
        bytecode.emplace_back(OP_HALT);
    }
}

const std::vector<XenoInstruction>& XenoCompiler::getBytecode() const { return bytecode; }
const std::vector<String>& XenoCompiler::getStringTable() const { return string_table; }

void XenoCompiler::printCompiledCode() {
    Debugger::disassemble(bytecode, string_table, "Compiled Xeno Program", true);
}
