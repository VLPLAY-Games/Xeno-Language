#ifndef XENO_COMPILER_H
#define XENO_COMPILER_H

#include "xeno_vm.h"
#include <vector>
#include <map>
#include <stack>

// Xeno Compiler - converts source code to bytecode
class XenoCompiler {
private:
    std::vector<XenoInstruction> bytecode;
    std::vector<String> string_table;
    std::map<String, int> variable_map;
    std::vector<int> if_stack; // Stack for tracking if-else jumps
    
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
    
    // Add string to string table
    int addString(const String& str) {
        for (size_t i = 0; i < string_table.size(); i++) {
            if (string_table[i] == str) return i;
        }
        string_table.push_back(str);
        return string_table.size() - 1;
    }
    
    // Get or create variable index
    int getVariableIndex(const String& var_name) {
        return addString(var_name);
    }
    
    // Check if string is a number
    bool isNumber(const String& str) {
        for (size_t i = 0; i < str.length(); i++) {
            if (!isdigit(str[i])) return false;
        }
        return str.length() > 0;
    }
    
    // Check if string is a valid variable name (letters, numbers, and underscores)
    bool isValidVariable(const String& str) {
        if (str.length() == 0) return false;
        
        // First character must be a letter
        if (!isalpha(str[0])) return false;
        
        // Remaining characters can be letters, numbers, or underscores
        for (size_t i = 1; i < str.length(); i++) {
            char c = str[i];
            if (!isalnum(c) && c != '_') {
                return false;
            }
        }
        return true;
    }
    
    // Check if string is a math function
    bool isMathFunction(const String& str) {
        return str == "abs";
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
    
    // Process function calls in expression
    String processFunctions(const String& expr) {
        String result = expr;
        
        // Process abs function
        int absPos = result.indexOf("abs(");
        while (absPos >= 0) {
            int endPos = findMatchingParenthesis(result, absPos + 3);
            if (endPos > absPos) {
                String inner = result.substring(absPos + 4, endPos);
                inner = processFunctions(inner);
                result = result.substring(0, absPos) + "[" + inner + "]" + result.substring(endPos + 1);
            } else {
                break;
            }
            absPos = result.indexOf("abs(");
        }
        
        return result;
    }
    
    // Find matching parenthesis
    int findMatchingParenthesis(const String& expr, int start) {
        int count = 1;
        for (int i = start + 1; i < expr.length(); i++) {
            if (expr[i] == '(') count++;
            else if (expr[i] == ')') count--;
            
            if (count == 0) return i;
        }
        return -1;
    }
    
    // Convert infix expression to postfix (RPN)
    std::vector<String> infixToPostfix(const std::vector<String>& tokens) {
        std::vector<String> output;
        std::stack<String> operators;
        
        for (const String& token : tokens) {
            if (isNumber(token) || isValidVariable(token) || (token.startsWith("[") && token.endsWith("]"))) {
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
            else if (isComparisonOperator(token) || token == "+" || token == "-" || token == "*" || token == "/" || token == "%" || token == "^") {
                while (!operators.empty() && 
                       operators.top() != "(" &&
                       (getPrecedence(operators.top()) > getPrecedence(token) ||
                       (getPrecedence(operators.top()) == getPrecedence(token) && !isRightAssociative(token)))) {
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
    
    // Tokenize expression
    std::vector<String> tokenizeExpression(const String& expr) {
        std::vector<String> tokens;
        String currentToken;
        bool inBrackets = false;
        
        for (size_t i = 0; i < expr.length(); i++) {
            char c = expr[i];
            
            if (c == '[') {
                if (currentToken.length() > 0) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                inBrackets = true;
                currentToken += c;
                continue;
            }
            else if (c == ']') {
                currentToken += c;
                tokens.push_back(currentToken);
                currentToken = "";
                inBrackets = false;
                continue;
            }
            
            if (inBrackets) {
                currentToken += c;
                continue;
            }
            
            if (isspace(c)) {
                if (currentToken.length() > 0) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                continue;
            }
            
            // Handle comparison operators (2 characters)
            if (i + 1 < expr.length()) {
                String twoChar = expr.substring(i, i + 2);
                if (twoChar == "==" || twoChar == "!=" || twoChar == "<=" || twoChar == ">=") {
                    if (currentToken.length() > 0) {
                        tokens.push_back(currentToken);
                        currentToken = "";
                    }
                    tokens.push_back(twoChar);
                    i++; // Skip next character
                    continue;
                }
            }
            
            // Handle single character operators
            if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '<' || c == '>' || c == '(' || c == ')') {
                if (currentToken.length() > 0) {
                    tokens.push_back(currentToken);
                    currentToken = "";
                }
                tokens.push_back(String(c));
            }
            else {
                currentToken += c;
            }
        }
        
        if (currentToken.length() > 0) {
            tokens.push_back(currentToken);
        }
        
        return tokens;
    }
    
    // Compile postfix expression
    void compilePostfix(const std::vector<String>& postfix) {
        for (const String& token : postfix) {
            if (isNumber(token)) {
                int num = token.toInt();
                emitInstruction(OP_PUSH, num);
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
            else if (token == "+") {
                emitInstruction(OP_ADD);
            }
            else if (token == "-") {
                emitInstruction(OP_SUB);
            }
            else if (token == "*") {
                emitInstruction(OP_MUL);
            }
            else if (token == "/") {
                emitInstruction(OP_DIV);
            }
            else if (token == "%") {
                emitInstruction(OP_MOD);
            }
            else if (token == "^") {
                emitInstruction(OP_POW);
            }
            else if (token == "==") {
                emitInstruction(OP_EQ);
            }
            else if (token == "!=") {
                emitInstruction(OP_NEQ);
            }
            else if (token == "<") {
                emitInstruction(OP_LT);
            }
            else if (token == ">") {
                emitInstruction(OP_GT);
            }
            else if (token == "<=") {
                emitInstruction(OP_LTE);
            }
            else if (token == ">=") {
                emitInstruction(OP_GTE);
            }
        }
    }
    
    // Compile expression with proper operator precedence
    void compileExpression(const String& expr) {
        String processedExpr = processFunctions(expr);
        std::vector<String> tokens = tokenizeExpression(processedExpr);
        std::vector<String> postfix = infixToPostfix(tokens);
        compilePostfix(postfix);
    }
    
    // Extract variable name from print command (after $)
    String extractVariableName(const String& text) {
        if (text.startsWith("$")) {
            return text.substring(1);
        }
        return "";
    }
    
    // Create instruction
    void emitInstruction(uint8_t opcode, uint16_t arg1 = 0, uint16_t arg2 = 0) {
        bytecode.push_back(XenoInstruction(opcode, arg1, arg2));
    }
    
    // Get current instruction address
    int getCurrentAddress() {
        return bytecode.size();
    }
    
    // Compile one line
    void compileLine(const String& line, int line_number) {
        String cleanedLine = cleanLine(line);
        if (cleanedLine.length() == 0) {
            return;
        }
        
        String originalLine = cleanedLine;
        
        int firstSpace = originalLine.indexOf(' ');
        String command = (firstSpace > 0) ? originalLine.substring(0, firstSpace) : originalLine;
        String args = (firstSpace > 0) ? originalLine.substring(firstSpace + 1) : "";
        args.trim();
        
        String lowerCommand = command;
        lowerCommand.toLowerCase();
        
        if (lowerCommand == "print") {
            String text = args;
            String var_name = extractVariableName(text);
            if (var_name.length() > 0) {
                if (isValidVariable(var_name)) {
                    int var_index = getVariableIndex(var_name);
                    emitInstruction(OP_LOAD, var_index);
                    emitInstruction(OP_PRINT_NUM);
                } else {
                    Serial.println("ERROR: Invalid variable name in print at line " + String(line_number));
                }
            } else {
                if (text.startsWith("\"") && text.endsWith("\"")) {
                    text = text.substring(1, text.length() - 1);
                }
                int str_id = addString(text);
                emitInstruction(OP_PRINT, str_id);
            }
        }
        else if (lowerCommand == "printnum") {
            emitInstruction(OP_PRINT_NUM);
        }
        else if (lowerCommand == "led") {
            int spaceIndex = args.indexOf(' ');
            if (spaceIndex > 0) {
                String pin_str = args.substring(0, spaceIndex);
                String state_str = args.substring(spaceIndex + 1);
                state_str.trim();
                state_str.toLowerCase();
                
                int pin = pin_str.toInt();
                if (state_str == "on" || state_str == "1") {
                    emitInstruction(OP_LED_ON, pin);
                } else if (state_str == "off" || state_str == "0") {
                    emitInstruction(OP_LED_OFF, pin);
                } else {
                    Serial.println("WARNING: Unknown LED state at line " + String(line_number));
                }
            } else {
                Serial.println("WARNING: Invalid LED command at line " + String(line_number));
            }
        }
        else if (lowerCommand == "delay") {
            int time = args.toInt();
            emitInstruction(OP_DELAY, time);
        }
        else if (lowerCommand == "push") {
            if (isValidVariable(args)) {
                int var_index = getVariableIndex(args);
                emitInstruction(OP_LOAD, var_index);
            } else {
                int num = args.toInt();
                emitInstruction(OP_PUSH, num);
            }
        }
        else if (lowerCommand == "pop") {
            emitInstruction(OP_POP);
        }
        else if (lowerCommand == "add") {
            emitInstruction(OP_ADD);
        }
        else if (lowerCommand == "sub") {
            emitInstruction(OP_SUB);
        }
        else if (lowerCommand == "mul") {
            emitInstruction(OP_MUL);
        }
        else if (lowerCommand == "div") {
            emitInstruction(OP_DIV);
        }
        else if (lowerCommand == "mod") {
            emitInstruction(OP_MOD);
        }
        else if (lowerCommand == "abs") {
            emitInstruction(OP_ABS);
        }
        else if (lowerCommand == "pow") {
            emitInstruction(OP_POW);
        }
        else if (lowerCommand == "set") {
            int space1 = args.indexOf(' ');
            if (space1 > 0) {
                String var_name = args.substring(0, space1);
                String expression = args.substring(space1 + 1);
                
                if (!isValidVariable(var_name)) {
                    Serial.println("ERROR: Invalid variable name '" + var_name + "' at line " + String(line_number));
                    return;
                }
                
                compileExpression(expression);
                
                int var_index = getVariableIndex(var_name);
                emitInstruction(OP_STORE, var_index);
            } else {
                Serial.println("ERROR: Invalid SET command at line " + String(line_number));
            }
        }
        else if (lowerCommand == "if") {
            // Format: if condition then
            int thenPos = args.indexOf(" then");
            if (thenPos > 0) {
                String condition = args.substring(0, thenPos);
                compileExpression(condition);
                
                // Store jump address placeholder (will be filled at endif)
                int jump_addr = getCurrentAddress();
                emitInstruction(OP_JUMP_IF, 0); // Placeholder
                if_stack.push_back(jump_addr);
            } else {
                Serial.println("ERROR: Invalid IF command at line " + String(line_number));
            }
        }
        else if (lowerCommand == "else") {
            if (!if_stack.empty()) {
                // Jump to end of else block
                int else_jump_addr = getCurrentAddress();
                emitInstruction(OP_JUMP, 0); // Placeholder
                
                // Update the previous if jump to point to current position
                int if_jump_addr = if_stack.back();
                bytecode[if_jump_addr].arg1 = getCurrentAddress();
                
                // Replace with else jump address
                if_stack.pop_back();
                if_stack.push_back(else_jump_addr);
            } else {
                Serial.println("ERROR: ELSE without IF at line " + String(line_number));
            }
        }
        else if (lowerCommand == "endif") {
            if (!if_stack.empty()) {
                // Update jump address to current position
                int jump_addr = if_stack.back();
                bytecode[jump_addr].arg1 = getCurrentAddress();
                if_stack.pop_back();
            } else {
                Serial.println("ERROR: ENDIF without IF at line " + String(line_number));
            }
        }
        else if (lowerCommand == "halt") {
            emitInstruction(OP_HALT);
        }
        else {
            Serial.println("WARNING: Unknown command at line " + String(line_number) + ": " + command);
        }
    }
    
public:
    XenoCompiler() {
        bytecode.clear();
        string_table.clear();
        variable_map.clear();
        if_stack.clear();
    }
    
    // Compile Xeno source code to bytecode
    void compile(const String& source_code) {
        bytecode.clear();
        string_table.clear();
        variable_map.clear();
        if_stack.clear();
        
        int line_number = 0;
        int startPos = 0;
        int endPos = source_code.indexOf('\n');
        
        while (endPos >= 0) {
            String line = source_code.substring(startPos, endPos);
            line_number++;
            
            if (line.length() > 0) {
                compileLine(line, line_number);
            }
            
            startPos = endPos + 1;
            endPos = source_code.indexOf('\n', startPos);
        }
        
        String lastLine = source_code.substring(startPos);
        if (lastLine.length() > 0) {
            compileLine(lastLine, ++line_number);
        }
        
        if (bytecode.empty() || bytecode.back().opcode != OP_HALT) {
            bytecode.push_back(XenoInstruction(OP_HALT));
        }
    }
    
    std::vector<XenoInstruction> getBytecode() const { return bytecode; }
    std::vector<String> getStringTable() const { return string_table; }
    
    void printCompiledCode() {
        Serial.println("=== Compiled Xeno Program ===");
        Serial.println("String table:");
        for (size_t i = 0; i < string_table.size(); i++) {
            Serial.println("  " + String(i) + ": \"" + string_table[i] + "\"");
        }
        Serial.println("Bytecode:");
        for (size_t i = 0; i < bytecode.size(); i++) {
            Serial.print("  " + String(i) + ": ");
            switch (bytecode[i].opcode) {
                case OP_NOP: Serial.println("NOP"); break;
                case OP_PRINT: Serial.println("PRINT " + String(bytecode[i].arg1)); break;
                case OP_LED_ON: Serial.println("LED_ON " + String(bytecode[i].arg1)); break;
                case OP_LED_OFF: Serial.println("LED_OFF " + String(bytecode[i].arg1)); break;
                case OP_DELAY: Serial.println("DELAY " + String(bytecode[i].arg1)); break;
                case OP_PUSH: Serial.println("PUSH " + String(bytecode[i].arg1)); break;
                case OP_POP: Serial.println("POP"); break;
                case OP_ADD: Serial.println("ADD"); break;
                case OP_SUB: Serial.println("SUB"); break;
                case OP_MUL: Serial.println("MUL"); break;
                case OP_DIV: Serial.println("DIV"); break;
                case OP_MOD: Serial.println("MOD"); break;
                case OP_ABS: Serial.println("ABS"); break;
                case OP_POW: Serial.println("POW"); break;
                case OP_EQ: Serial.println("EQ"); break;
                case OP_NEQ: Serial.println("NEQ"); break;
                case OP_LT: Serial.println("LT"); break;
                case OP_GT: Serial.println("GT"); break;
                case OP_LTE: Serial.println("LTE"); break;
                case OP_GTE: Serial.println("GTE"); break;
                case OP_PRINT_NUM: Serial.println("PRINT_NUM"); break;
                case OP_STORE: 
                    if (bytecode[i].arg1 < string_table.size()) {
                        Serial.println("STORE " + string_table[bytecode[i].arg1]);
                    } else {
                        Serial.println("STORE <invalid>");
                    }
                    break;
                case OP_LOAD: 
                    if (bytecode[i].arg1 < string_table.size()) {
                        Serial.println("LOAD " + string_table[bytecode[i].arg1]);
                    } else {
                        Serial.println("LOAD <invalid>");
                    }
                    break;
                case OP_JUMP: Serial.println("JUMP " + String(bytecode[i].arg1)); break;
                case OP_JUMP_IF: Serial.println("JUMP_IF " + String(bytecode[i].arg1)); break;
                case OP_HALT: Serial.println("HALT"); break;
                default: Serial.println("UNKNOWN " + String(bytecode[i].opcode)); break;
            }
        }
    }
};

#endif // XENO_COMPILER_H