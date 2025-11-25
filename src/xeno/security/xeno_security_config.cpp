#include "xeno_security_config.h"

XenoSecurityConfig::XenoSecurityConfig() {
    // Значения уже инициализированы в заголовке
}

bool XenoSecurityConfig::setMaxStringLength(size_t length) {
    return validateSizeLimit(length, 1, 4096, "MAX_STRING_LENGTH");
}

bool XenoSecurityConfig::setMaxVariableNameLength(size_t length) {
    return validateSizeLimit(length, 1, 256, "MAX_VARIABLE_NAME_LENGTH");
}

bool XenoSecurityConfig::setMaxExpressionDepth(size_t depth) {
    return validateSizeLimit(depth, 1, 256, "MAX_EXPRESSION_DEPTH");
}

bool XenoSecurityConfig::setMaxLoopDepth(size_t depth) {
    return validateSizeLimit(depth, 1, 64, "MAX_LOOP_DEPTH");
}

bool XenoSecurityConfig::setMaxIfDepth(size_t depth) {
    return validateSizeLimit(depth, 1, 64, "MAX_IF_DEPTH");
}

bool XenoSecurityConfig::setMaxStackSize(size_t size) {
    return validateSizeLimit(size, 16, 2048, "MAX_STACK_SIZE");
}

bool XenoSecurityConfig::setMaxInstructions(uint32_t max_instr) {
    if (max_instr < MIN_INSTRUCTIONS || max_instr > MAX_INSTRUCTIONS) {
        Serial.print("SECURITY: max_instructions must be between ");
        Serial.print(MIN_INSTRUCTIONS);
        Serial.print(" and ");
        Serial.println(MAX_INSTRUCTIONS);
        return false;
    }
    max_instructions = max_instr;
    return true;
}

void XenoSecurityConfig::setAllowedPins(const std::vector<uint8_t>& pins) {
    ALLOWED_PINS = pins;
}

bool XenoSecurityConfig::validateSizeLimit(size_t value, size_t min_val, size_t max_val, const char* param_name) {
    if (value < min_val || value > max_val) {
        Serial.print("SECURITY: ");
        Serial.print(param_name);
        Serial.print(" must be between ");
        Serial.print(min_val);
        Serial.print(" and ");
        Serial.println(max_val);
        return false;
    }
    return true;
}