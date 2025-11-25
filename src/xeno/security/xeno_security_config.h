#ifndef SRC_XENO_XENO_SECURITY_CONFIG_H_
#define SRC_XENO_XENO_SECURITY_CONFIG_H_

#include <Arduino.h>
#include <vector>

class XenoSecurityConfig {
public:
    XenoSecurityConfig();
    
    size_t MAX_STRING_LENGTH = 256;
    size_t MAX_VARIABLE_NAME_LENGTH = 32;
    size_t MAX_EXPRESSION_DEPTH = 32;
    size_t MAX_LOOP_DEPTH = 16;
    size_t MAX_IF_DEPTH = 16;
    size_t MAX_STACK_SIZE = 256;
    
    uint32_t MIN_INSTRUCTIONS = 1000;
    uint32_t MAX_INSTRUCTIONS = 1000000;
    
    std::vector<uint8_t> ALLOWED_PINS = { LED_BUILTIN };
    
    uint32_t max_instructions = 100000;
    
    bool setMaxStringLength(size_t length);
    bool setMaxVariableNameLength(size_t length);
    bool setMaxExpressionDepth(size_t depth);
    bool setMaxLoopDepth(size_t depth);
    bool setMaxIfDepth(size_t depth);
    bool setMaxStackSize(size_t size);
    bool setMaxInstructions(uint32_t max_instr);
    void setAllowedPins(const std::vector<uint8_t>& pins);
    
private:
    bool validateSizeLimit(size_t value, size_t min_val, size_t max_val, const char* param_name);
};

#endif  // SRC_XENO_XENO_SECURITY_CONFIG_H_