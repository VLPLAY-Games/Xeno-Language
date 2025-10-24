#ifndef XENO_SECURITY_H
#define XENO_SECURITY_H


// Security limits
#define MAX_STRING_LENGTH 256
#define MAX_VARIABLE_NAME_LENGTH 32
#define MAX_EXPRESSION_DEPTH 32
#define MAX_LOOP_DEPTH 16
#define MAX_IF_DEPTH 16
#define MAX_STACK_SIZE 256

class XenoSecurity {
private:

    // Allowed pins for safety
    static constexpr std::array<uint8_t, 13> ALLOWED_PINS = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, LED_BUILTIN};
    static constexpr size_t NUM_ALLOWED_PINS = ALLOWED_PINS.size();


protected:
    bool isPinAllowed(uint8_t pin) {
        for (size_t i = 0; i < NUM_ALLOWED_PINS; i++) {
            if (pin == ALLOWED_PINS[i]) {
                return true;
            }
        }
        return false;
    }
    // String sanitization for security
    String sanitizeString(const String& input) {
        String sanitized;
        sanitized.reserve(input.length());
        
        for (size_t i = 0; i < input.length(); i++) {
            char c = input[i];
            
            // Allow only safe printable ASCII characters
            if (c >= 32 && c <= 126) {
                // Escape potentially dangerous characters
                if (c == '\\' || c == '"' || c == '\'' || c == '`') {
                    sanitized += '\\';
                }
                sanitized += c;
            }
            // Allow basic whitespace
            else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                sanitized += c;
            }
            // Replace other characters with safe equivalent
            else {
                sanitized += '?';
            }
            
            // Limit maximum string length for safety
            if (sanitized.length() >= 256) {
                sanitized += "...";
                break;
            }
        }
        
        return sanitized;
    }
    
    // Bytecode verification for security
    bool verifyBytecode(const std::vector<XenoInstruction>& bytecode, 
                       const std::vector<String>& strings) {
        // Check program size limits
        if (bytecode.size() > 10000) {
            Serial.println("SECURITY: Program too large");
            return false;
        }
        
        // Check string table size limits
        if (strings.size() > 1000) {
            Serial.println("SECURITY: String table too large");
            return false;
        }
        
        // Verify each instruction
        for (size_t i = 0; i < bytecode.size(); i++) {
            const XenoInstruction& instr = bytecode[i];
            
            // Check for valid opcode range
            if (instr.opcode > 30 && instr.opcode != 255) {
                Serial.print("SECURITY: Invalid opcode at instruction ");
                Serial.println(i);
                return false;
            }
            
            // Verify jump targets are within program bounds
            if (instr.opcode == OP_JUMP || instr.opcode == OP_JUMP_IF) {
                if (instr.arg1 >= bytecode.size()) {
                    Serial.print("SECURITY: Invalid jump target at instruction ");
                    Serial.println(i);
                    return false;
                }
            }
            
            // Verify string indices are within string table bounds
            if (instr.opcode == OP_PRINT || instr.opcode == OP_STORE || 
                instr.opcode == OP_LOAD || instr.opcode == OP_PUSH_STRING ||
                instr.opcode == OP_INPUT) {
                if (instr.arg1 >= strings.size()) {
                    Serial.print("SECURITY: Invalid string index at instruction ");
                    Serial.println(i);
                    return false;
                }
            }
            
            // Verify pin numbers are allowed
            if (instr.opcode == OP_LED_ON || instr.opcode == OP_LED_OFF) {
                if (!isPinAllowed(instr.arg1)) {
                    Serial.print("SECURITY: Unauthorized pin access at instruction ");
                    Serial.println(i);
                    return false;
                }
            }
            
            // Verify delay values are reasonable
            if (instr.opcode == OP_DELAY) {
                if (instr.arg1 > 60000) { // Max 60 seconds
                    Serial.print("SECURITY: Excessive delay at instruction ");
                    Serial.println(i);
                    return false;
                }
            }
        }
        
        // Verify no infinite loops without conditions
        bool has_halt = false;
        for (const auto& instr : bytecode) {
            if (instr.opcode == OP_HALT) {
                has_halt = true;
                break;
            }
        }
        
        if (!has_halt && bytecode.size() > 10) {
            Serial.println("SECURITY: Program missing HALT instruction");
            return false;
        }
        
        return true;
    }
public:
    friend class XenoVM;
};


#endif // XENO_SECURITY_H