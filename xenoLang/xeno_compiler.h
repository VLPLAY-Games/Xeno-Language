#ifndef XENO_COMPILER_H
#define XENO_COMPILER_H

#include "xeno_common.h"
#include "xeno_vm.h"
#include <vector>
#include <map>
#include <stack>


// Xeno Compiler - converts source code to bytecode
class XenoCompiler {
private:
    std::vector<XenoInstruction> bytecode;
    std::vector<String> string_table;
    std::map<String, XenoValue> variable_map;
    std::vector<int> if_stack;
    std::vector<LoopInfo> loop_stack;
    
    // Security validation
    bool validateString(const String& str) {
        return !(str.length() > MAX_STRING_LENGTH) || (Serial.println("ERROR: String too long"), false);
    }
    
    bool validateVariableName(const String& name) {
        if (name.length() > MAX_VARIABLE_NAME_LENGTH) {
            Serial.println("ERROR: Variable name too long");
            return false;
        }
        return isValidVariable(name) || (Serial.println("ERROR: Invalid variable name"), false);
    }
    
    // Remove comments and trim whitespace
    String cleanLine(const String& line) {
        String cleaned = line;
        int commentIndex = cleaned.indexOf("//");
        if (commentIndex >= 0) {
            cleaned = cleaned.substring(0, commentIndex);
        }
        cleaned.trim();
        return cleaned;
    }
    
    // Add string to string table with security checks
    int addString(const String& str) {
        if (!validateString(str)) {
            return 0;
        }
        
        // Optimized search using reverse iteration (newer strings more likely to match)
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
    
    // Get or create variable index with security checks
    int getVariableIndex(const String& var_name) {
        return validateVariableName(var_name) ? addString(var_name) : 0;
    }
    
    // Check if string is an integer with bounds checking
    bool isInteger(const String& str) {
        if (str.isEmpty() || str.length() > 16) return false;
        
        const char* cstr = str.c_str();
        size_t start = 0;
        if (cstr[0] == '-') start = 1;
        
        for (size_t i = start; i < str.length(); ++i) {
            if (!isdigit(cstr[i])) return false;
        }
        
        // Check for integer overflow
        long long_val = str.toInt();
        return !(long_val > 2147483647L || long_val < -2147483648L);
    }
    
    // Check if string is a float with bounds checking
    bool isFloat(const String& str) {
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
    
    // Check if string is a quoted string
    bool isQuotedString(const String& str) {
        return str.length() >= 2 && str[0] == '"' && str[str.length() - 1] == '"';
    }
    
    // Check if string is a valid variable name
    bool isValidVariable(const String& str) {
        if (str.isEmpty() || str.length() > MAX_VARIABLE_NAME_LENGTH) return false;
        
        const char first = str[0];
        if (!isalpha(first) && first != '_') return false;
        
        for (size_t i = 1; i < str.length(); ++i) {
            const char c = str[i];
            if (!isalnum(c) && c != '_') return false;
        }
        return true;
    }
    
    // Check if string is a comparison operator
    bool isComparisonOperator(const String& str) {
        return str == "==" || str == "!=" || str == "<" || str == ">" || str == "<=" || str == ">=";
    }
    
    // Get operator precedence
    int getPrecedence(const String& op) {
        if (op == "^") return 4;
        if (op == "*" || op == "/" || op == "%") return 3;
        if (op == "+" || op == "-") return 2;
        if (isComparisonOperator(op)) return 1;
        return 0;
    }
    
    // Check if operator is right-associative
    bool isRightAssociative(const String& op) {
        return op == "^";
    }
    
    // Process function calls in expression with depth limit
    String processFunctions(const String& expr) {
        if (expr.length() > 1024) {
            Serial.println("ERROR: Expression too long");
            return expr;
        }
        
        String result = expr;
        int depth = 0;
        
        // Process abs()
        int absPos = result.indexOf("abs(");
        while (absPos >= 0 && depth < MAX_EXPRESSION_DEPTH) {
            int endPos = findMatchingParenthesis(result, absPos + 3);
            if (endPos > absPos) {
                String inner = result.substring(absPos + 4, endPos);
                inner = processFunctions(inner);
                result = result.substring(0, absPos) + "[" + inner + "]" + result.substring(endPos + 1);
            } else {
                break;
            }
            absPos = result.indexOf("abs(");
            depth++;
        }
        
        // Process max()
        int maxPos = result.indexOf("max(");
        while (maxPos >= 0 && depth < MAX_EXPRESSION_DEPTH) {
            int endPos = findMatchingParenthesis(result, maxPos + 3);
            if (endPos > maxPos) {
                String inner = result.substring(maxPos + 4, endPos);
                inner = processFunctions(inner);
                result = result.substring(0, maxPos) + "{" + inner + "}" + result.substring(endPos + 1);
            } else {
                break;
            }
            maxPos = result.indexOf("max(");
            depth++;
        }
        
        // Process min()
        int minPos = result.indexOf("min(");
        while (minPos >= 0 && depth < MAX_EXPRESSION_DEPTH) {
            int endPos = findMatchingParenthesis(result, minPos + 3);
            if (endPos > minPos) {
                String inner = result.substring(minPos + 4, endPos);
                inner = processFunctions(inner);
                result = result.substring(0, minPos) + "|" + inner + "|" + result.substring(endPos + 1);
            } else {
                break;
            }
            minPos = result.indexOf("min(");
            depth++;
        }
        
        // Process sqrt()
        int sqrtPos = result.indexOf("sqrt(");
        while (sqrtPos >= 0 && depth < MAX_EXPRESSION_DEPTH) {
            int endPos = findMatchingParenthesis(result, sqrtPos + 4);
            if (endPos > sqrtPos) {
                String inner = result.substring(sqrtPos + 5, endPos);
                inner = processFunctions(inner);
                result = result.substring(0, sqrtPos) + "~" + inner + "~" + result.substring(endPos + 1);
            } else {
                break;
            }
            sqrtPos = result.indexOf("sqrt(");
            depth++;
        }
        
        if (depth >= MAX_EXPRESSION_DEPTH) {
            Serial.println("ERROR: Expression too complex");
        }
        
        return result;
    }
    
    // Find matching parenthesis
    int findMatchingParenthesis(const String& expr, int start) {
        int count = 1;
        for (int i = start + 1; i < expr.length(); ++i) {
            if (expr[i] == '(') ++count;
            else if (expr[i] == ')') --count;
            
            if (count == 0) return i;
        }
        return -1;
    }
    
    // Convert infix expression to postfix (RPN) with safety limits
    std::vector<String> infixToPostfix(const std::vector<String>& tokens) {
        std::vector<String> output;
        std::stack<String> operators;
        output.reserve(tokens.size());
        
        if (tokens.size() > 100) {
            Serial.println("ERROR: Too many tokens in expression");
            return output;
        }
        
        for (const String& token : tokens) {
            if (isInteger(token) || isFloat(token) || isQuotedString(token) || isValidVariable(token) || 
                (token.startsWith("[") && token.endsWith("]")) ||
                (token.startsWith("{") && token.endsWith("}")) ||
                (token.startsWith("|") && token.endsWith("|")) ||
                (token.startsWith("~") && token.endsWith("~"))) {
                output.push_back(token);
            }
            else if (token == "(") {
                operators.push(token);
            }
            else if (token == ")") {
                while (!operators.empty() && operators.top() != "(") {
                    output.push_back(operators.top());
                    operators.pop();
                }
                if (!operators.empty()) operators.pop();
            }
            else {
                int token_precedence = getPrecedence(token);
                while (!operators.empty() && 
                       operators.top() != "(" &&
                       (getPrecedence(operators.top()) > token_precedence ||
                       (getPrecedence(operators.top()) == token_precedence && !isRightAssociative(token)))) {
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
    
    // Tokenize expression with safety limits
    std::vector<String> tokenizeExpression(const String& expr) {
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
            
            // Handle special function markers
            if ((c == '[' || c == '{' || c == '|' || c == '~') && !inSpecial) {
                if (!currentToken.isEmpty()) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                inSpecial = true;
                specialChar = c;
                currentToken += c;
                continue;
            }
            else if (inSpecial && c == specialChar) {
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
            
            // Handle multi-character operators
            if (i + 1 < expr.length()) {
                String twoChar = expr.substring(i, i + 2);
                if (twoChar == "==" || twoChar == "!=" || twoChar == "<=" || twoChar == ">=") {
                    if (!currentToken.isEmpty()) {
                        tokens.push_back(currentToken);
                        currentToken = "";
                    }
                    tokens.push_back(twoChar);
                    ++i;
                    continue;
                }
            }
            
            // Handle single character operators
            if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || 
                c == '<' || c == '>' || c == '(' || c == ')') {
                if (!currentToken.isEmpty()) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                tokens.push_back(String(c));
            }
            else {
                currentToken += c;
            }
        }
        
        if (!currentToken.isEmpty()) {
            tokens.push_back(currentToken);
        }
        
        return tokens;
    }
    
    void compilePostfix(const std::vector<String>& postfix) {
        if (postfix.size() > 100) {
            Serial.println("ERROR: Postfix expression too complex");
            return;
        }
        
        for (const String& token : postfix) {
            if (isInteger(token)) {
                int32_t value = token.toInt();
                emitInstruction(OP_PUSH, static_cast<uint32_t>(value));
            }
            else if (isFloat(token)) {
                float fval = token.toFloat();
                uint32_t fbits;
                memcpy(&fbits, &fval, sizeof(float));
                emitInstruction(OP_PUSH_FLOAT, fbits);
            }
            else if (isQuotedString(token)) {
                String str = token.substring(1, token.length() - 1);
                if (!validateString(str)) {
                    str = "";
                }
                int str_id = addString(str);
                emitInstruction(OP_PUSH_STRING, str_id);
            }
            else if (isValidVariable(token)) {
                int var_index = getVariableIndex(token);
                emitInstruction(OP_LOAD, var_index);
            }
            else if (token.startsWith("[") && token.endsWith("]")) {
                String innerExpr = token.substring(1, token.length() - 1);
                compileExpression(innerExpr);
                emitInstruction(OP_ABS);
            }
            else if (token.startsWith("{") && token.endsWith("}")) {
                String innerExpr = token.substring(1, token.length() - 1);
                // max function takes two arguments separated by comma
                int commaPos = innerExpr.indexOf(',');
                if (commaPos > 0) {
                    String arg1 = innerExpr.substring(0, commaPos);
                    String arg2 = innerExpr.substring(commaPos + 1);
                    compileExpression(arg1);
                    compileExpression(arg2);
                    emitInstruction(OP_MAX);
                } else {
                    Serial.println("ERROR: max function requires two arguments");
                }
            }
            else if (token.startsWith("|") && token.endsWith("|")) {
                String innerExpr = token.substring(1, token.length() - 1);
                // min function takes two arguments separated by comma
                int commaPos = innerExpr.indexOf(',');
                if (commaPos > 0) {
                    String arg1 = innerExpr.substring(0, commaPos);
                    String arg2 = innerExpr.substring(commaPos + 1);
                    compileExpression(arg1);
                    compileExpression(arg2);
                    emitInstruction(OP_MIN);
                } else {
                    Serial.println("ERROR: min function requires two arguments");
                }
            }
            else if (token.startsWith("~") && token.endsWith("~")) {
                String innerExpr = token.substring(1, token.length() - 1);
                compileExpression(innerExpr);
                emitInstruction(OP_SQRT);
            }
            else if (token == "+") emitInstruction(OP_ADD);
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
    
    // Compile expression with proper operator precedence and safety checks
    void compileExpression(const String& expr) {
        if (expr.isEmpty() || expr.length() > 1024) {
            Serial.println("ERROR: Invalid expression");
            return;
        }
        
        String processedExpr = processFunctions(expr);
        std::vector<String> tokens = tokenizeExpression(processedExpr);
        std::vector<String> postfix = infixToPostfix(tokens);
        compilePostfix(postfix);
    }
    
    // Extract variable name from print command
    String extractVariableName(const String& text) {
        return text.startsWith("$") ? text.substring(1) : "";
    }
    
    // Determine value type from string
    XenoDataType determineValueType(const String& value) {
        if (isQuotedString(value)) return TYPE_STRING;
        if (isFloat(value)) return TYPE_FLOAT;
        if (isInteger(value)) return TYPE_INT;
        if (isValidVariable(value)) {
            auto it = variable_map.find(value);
            return it != variable_map.end() ? it->second.type : TYPE_INT;
        }
        return TYPE_INT;
    }
    
    // Create value from string
    XenoValue createValueFromString(const String& str, XenoDataType type) {
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
                value.string_index = addString(str.substring(1, str.length() - 1));
                break;
        }
        return value;
    }
    
    // Create instruction with bounds checking
    void emitInstruction(uint8_t opcode, uint32_t arg1 = 0, uint16_t arg2 = 0) {
        if (bytecode.size() >= 65535) {
            Serial.println("ERROR: Program too large");
            return;
        }
        bytecode.emplace_back(opcode, arg1, arg2);
    }
    
    // Get current instruction address
    int getCurrentAddress() {
        return bytecode.size();
    }
    
    // Compile one line with security validation
    void compileLine(const String& line, int line_number) {
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
        }
        else if (command == "printnum") {
            emitInstruction(OP_PRINT_NUM);
        }
        else if (command == "led") {
            int spaceIndex = args.indexOf(' ');
            if (spaceIndex > 0) {
                String pin_str = args.substring(0, spaceIndex);
                String state_str = args.substring(spaceIndex + 1);
                state_str.trim();
                state_str.toLowerCase();
                
                int pin = pin_str.toInt();
                // Validate pin number for safety
                if (pin < 0 || pin > 255) {
                    Serial.print("ERROR: Invalid pin number at line ");
                    Serial.println(line_number);
                    return;
                }
                
                if (state_str == "on" || state_str == "1") {
                    emitInstruction(OP_LED_ON, pin);
                } else if (state_str == "off" || state_str == "0") {
                    emitInstruction(OP_LED_OFF, pin);
                } else {
                    Serial.print("WARNING: Unknown LED state at line ");
                    Serial.println(line_number);
                }
            } else {
                Serial.print("WARNING: Invalid LED command at line ");
                Serial.println(line_number);
            }
        }
        else if (command == "delay") {
            int delay_time = args.toInt();
            if (delay_time < 0 || delay_time > 60000) { // Max 60 seconds
                Serial.print("WARNING: Delay time out of range at line ");
                Serial.println(line_number);
                delay_time = min(max(delay_time, 0), 60000);
            }
            emitInstruction(OP_DELAY, delay_time);
        }
        else if (command == "push") {
            if (isValidVariable(args)) {
                int var_index = getVariableIndex(args);
                emitInstruction(OP_LOAD, var_index);
            } else if (isFloat(args)) {
                float fval = args.toFloat();
                uint32_t fbits;
                memcpy(&fbits, &fval, sizeof(float));
                emitInstruction(OP_PUSH_FLOAT, fbits);
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
        }
        else if (command == "pop") emitInstruction(OP_POP);
        else if (command == "add") emitInstruction(OP_ADD);
        else if (command == "sub") emitInstruction(OP_SUB);
        else if (command == "mul") emitInstruction(OP_MUL);
        else if (command == "div") emitInstruction(OP_DIV);
        else if (command == "mod") emitInstruction(OP_MOD);
        else if (command == "abs") emitInstruction(OP_ABS);
        else if (command == "pow") emitInstruction(OP_POW);
        else if (command == "max") emitInstruction(OP_MAX);
        else if (command == "min") emitInstruction(OP_MIN);
        else if (command == "sqrt") emitInstruction(OP_SQRT);
        else if (command == "input") {
            String var_name = args;
            if (!validateVariableName(var_name)) {
                Serial.print("ERROR: Invalid variable name for input at line ");
                Serial.println(line_number);
                return;
            }
            int var_index = getVariableIndex(var_name);
            emitInstruction(OP_INPUT, var_index);
        }
        else if (command == "set") {
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
                if (isInteger(expression) || isFloat(expression) || isQuotedString(expression)) {
                    variable_map[var_name] = createValueFromString(expression, value_type);
                }
                
                compileExpression(expression);
                emitInstruction(OP_STORE, getVariableIndex(var_name));
            } else {
                Serial.print("ERROR: Invalid SET command at line ");
                Serial.println(line_number);
            }
        }
        else if (command == "if") {
            if (if_stack.size() >= MAX_IF_DEPTH) {
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
        }
        else if (command == "else") {
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
        }
        else if (command == "endif") {
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
        }
        else if (command == "for") {
            if (loop_stack.size() >= MAX_LOOP_DEPTH) {
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
        }
        else if (command == "endfor") {
            if (!loop_stack.empty()) {
                LoopInfo loop_info = loop_stack.back();
                loop_stack.pop_back();
                
                emitInstruction(OP_LOAD, getVariableIndex(loop_info.var_name));
                auto var_it = variable_map.find(loop_info.var_name);
                if (var_it != variable_map.end() && var_it->second.type == TYPE_FLOAT) {
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
        }
        else if (command == "halt") {
            emitInstruction(OP_HALT);
        }
        else {
            Serial.print("WARNING: Unknown command at line ");
            Serial.print(line_number);
            Serial.print(": ");
            Serial.println(command);
        }
    }
    
public:
    XenoCompiler() {
        bytecode.reserve(128);
        string_table.reserve(32);
        if_stack.reserve(8);
        loop_stack.reserve(4);
    }
    
    void compile(const String& source_code) {
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
    
    const std::vector<XenoInstruction>& getBytecode() const { return bytecode; }
    const std::vector<String>& getStringTable() const { return string_table; }
    
    void printCompiledCode() {
        Serial.println("=== Compiled Xeno Program ===");
        Serial.println("String table:");
        for (size_t i = 0; i < string_table.size(); ++i) {
            Serial.print("  ");
            Serial.print(i);
            Serial.print(": \"");
            Serial.print(string_table[i]);
            Serial.println("\"");
        }
        Serial.println("Bytecode:");
        for (size_t i = 0; i < bytecode.size(); ++i) {
            Serial.print("  ");
            Serial.print(i);
            Serial.print(": ");
            const XenoInstruction& instr = bytecode[i];
            
            switch (instr.opcode) {
                case OP_NOP: Serial.println("NOP"); break;
                case OP_PRINT: 
                    Serial.print("PRINT ");
                    Serial.println(instr.arg1);
                    break;
                case OP_LED_ON: 
                    Serial.print("LED_ON ");
                    Serial.println(instr.arg1);
                    break;
                case OP_LED_OFF: 
                    Serial.print("LED_OFF ");
                    Serial.println(instr.arg1);
                    break;
                case OP_DELAY: 
                    Serial.print("DELAY ");
                    Serial.println(instr.arg1);
                    break;
                case OP_PUSH: 
                    Serial.print("PUSH ");
                    Serial.println(instr.arg1);
                    break;
                case OP_PUSH_FLOAT: {
                    float fval;
                    memcpy(&fval, &instr.arg1, sizeof(float));
                    Serial.print("PUSH_FLOAT ");
                    Serial.println(fval, 4);
                    break;
                }
                case OP_PUSH_STRING: 
                    Serial.print("PUSH_STRING ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.print("\"");
                        Serial.print(string_table[instr.arg1]);
                        Serial.println("\"");
                    } else {
                        Serial.println("<invalid>");
                    }
                    break;
                case OP_POP: Serial.println("POP"); break;
                case OP_ADD: Serial.println("ADD"); break;
                case OP_SUB: Serial.println("SUB"); break;
                case OP_MUL: Serial.println("MUL"); break;
                case OP_DIV: Serial.println("DIV"); break;
                case OP_MOD: Serial.println("MOD"); break;
                case OP_ABS: Serial.println("ABS"); break;
                case OP_POW: Serial.println("POW"); break;
                case OP_MAX: Serial.println("MAX"); break;
                case OP_MIN: Serial.println("MIN"); break;
                case OP_SQRT: Serial.println("SQRT"); break;
                case OP_INPUT: 
                    Serial.print("INPUT ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.println(string_table[instr.arg1]);
                    } else {
                        Serial.println("<invalid>");
                    }
                    break;
                case OP_EQ: Serial.println("EQ"); break;
                case OP_NEQ: Serial.println("NEQ"); break;
                case OP_LT: Serial.println("LT"); break;
                case OP_GT: Serial.println("GT"); break;
                case OP_LTE: Serial.println("LTE"); break;
                case OP_GTE: Serial.println("GTE"); break;
                case OP_PRINT_NUM: Serial.println("PRINT_NUM"); break;
                case OP_STORE: 
                    Serial.print("STORE ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.println(string_table[instr.arg1]);
                    } else {
                        Serial.println("<invalid>");
                    }
                    break;
                case OP_LOAD: 
                    Serial.print("LOAD ");
                    if (instr.arg1 < string_table.size()) {
                        Serial.println(string_table[instr.arg1]);
                    } else {
                        Serial.println("<invalid>");
                    }
                    break;
                case OP_JUMP: 
                    Serial.print("JUMP ");
                    Serial.println(instr.arg1);
                    break;
                case OP_JUMP_IF: 
                    Serial.print("JUMP_IF ");
                    Serial.println(instr.arg1);
                    break;
                case OP_HALT: Serial.println("HALT"); break;
                default: 
                    Serial.print("UNKNOWN ");
                    Serial.println(instr.opcode);
                    break;
            }
        }
    }
};

#endif // XENO_COMPILER_H