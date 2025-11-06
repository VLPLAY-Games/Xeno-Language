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

#include "xeno_common.h"

XenoValue::XenoValue() : type(TYPE_INT), int_val(0) {}

XenoValue XenoValue::makeInt(int32_t val) {
    XenoValue v;
    v.type = TYPE_INT;
    v.int_val = val;
    return v;
}

XenoValue XenoValue::makeFloat(float val) {
    XenoValue v;
    v.type = TYPE_FLOAT;
    v.float_val = val;
    return v;
}

XenoValue XenoValue::makeString(uint16_t str_idx) {
    XenoValue v;
    v.type = TYPE_STRING;
    v.string_index = str_idx;
    return v;
}

XenoInstruction::XenoInstruction(uint8_t op, uint32_t a1, uint16_t a2)
    : opcode(op), arg1(a1), arg2(a2) {}