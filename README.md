[![Active](https://img.shields.io/badge/Project-Active-brightgreen.svg)](#)
[![Last Commit](https://img.shields.io/github/last-commit/VLPLAY-Games/Xeno-Language)](#)

[![Version](https://img.shields.io/badge/Version-0.1.4-lightgrey.svg)](#)
[![Platform](https://img.shields.io/badge/Platform-ESP32-orange.svg)](#)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-brightgreen.svg)](#)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

[![Arduino Library](https://www.ardu-badge.com/badge/Xeno-Language.svg?)](https://www.ardu-badge.com/Xeno%20Language)

[![Arduino Lint](https://github.com/VLPLAY-Games/Xeno-Language/actions/workflows/arduino-lint.yml/badge.svg)](https://github.com/VLPLAY-Games/Xeno-Language/actions/workflows/arduino-lint.yml)
[![cpplint](https://github.com/VLPLAY-Games/Xeno-Language/actions/workflows/cpplint.yml/badge.svg)](https://github.com/VLPLAY-Games/Xeno-Language/actions/workflows/cpplint.yml)

- [Read in Russian](README.ru.md) 
- [Read in Japanese](README.ja.md)

# Xeno Language

**Xeno Language** — a compact, safe interpreted language and virtual machine for the **ESP32 (Arduino)** ecosystem.  
Implemented as: **compiler → bytecode → VM** with built-in commands for numbers, strings, branching, loops and basic GPIO control.

**Developed by VL_PLAY Games**

---

## 🚀 Quick Pitch

✨ **Highlights**
- Execute code from `.xeno` files or embedded source strings.  
- Lightweight and easy to embed into nearly any ESP32 project.  
- Designed for hobbyists, education, and small automation tasks.

⚡️ **Performance**
- Benchmarks show Xeno is roughly **~26× slower than equivalent native C++**, depending on workload (see `benchmark.ino`).  
- Compared to other interpreted MCU languages (MicroPython, Lua), Xeno's performance is in the same ballpark — exact differences depend heavily on the type of code and usage patterns. Use `benchmark.ino` for workload-specific comparisons.
### ⚔️ Language Performance Comparison

| Feature / Language            | **Xeno** 🧠       | **MicroPython** 🐍       | **Lua (NodeMCU)** 🌙    | **C++ (Native)** ⚙️     |
|-------------------------------|:-----------------:|:------------------------:|:----------------------:|:-----------------------:|
| Execution Speed (vs C++)      | ~26× slower       | ~18× slower *(approx.)*  | ~20× slower *(approx.)*| 🥇 Baseline             |
| Memory Usage (RAM)            | Low (~20 KB)      | Medium (~30 KB)          | Medium (~25 KB)        | High control (manual)   |
| File-based Execution          | ✅ `.xeno` files   | ✅ `.py` files           | ✅ `.lua` files        | ⚠️ Compiled only        |
| Hardware Access (GPIO, etc.)  | ⚠️ Basic (LED only, in dev) | ✅ Rich | ✅ Rich | ✅ Full |
| Ease of Embedding             | ⭐️⭐️⭐️⭐️⭐️        | ⭐️⭐️⭐️⭐️                 | ⭐️⭐️⭐️⭐️               | ⭐️⭐️                    |
| Language Simplicity           | Moderately easy   | Easy                     | Moderate               | Complex                 |
| Embeddable in C++ Projects    | ✅ Fully embeddable| ⚠️ Limited (separate runtime) | ⚠️ Limited (separate runtime) | —                  |
| Ideal Use Cases               | Teaching, quick logic, in-app scripting | Prototyping, IoT | Scripting automation | Performance-critical    |

> ⚡ **Notes:**  
> - Speed ratios for **Xeno** were measured on **ESP32-C3 @160 MHz**. Values for MicroPython and Lua are approximate comparisons against C++ and will vary by workload and firmware. See `benchmark.ino` for full details.  
> - **Xeno advantage:** Unlike MicroPython or Lua on many ESP32 workflows, **Xeno can be embedded directly inside an existing C++ firmware** — no separate interpreter firmware required.  
> - **GPIO note:** Hardware access in Xeno is currently limited to LED control, with broader GPIO and peripheral support planned for future updates.

---

## System Requirements

### Hardware
- Any ESP32-class microcontroller (ESP32, ESP32-S2, etc.)  
- Recommended free Flash: **≥ 60 KB**  
- Recommended free RAM: **≥ 20 KB**

### Software
- ESP32 Arduino core: **3.2.0+**  
- Arduino IDE: **2.0+**

> Note: actual memory usage depends on enabled features, strings, and included examples. If your project is tight on RAM/Flash, strip unused features and reduce `MAX_*` values in config headers.

---

## Quickstart

### Installation
**Method 1: Library Manager (Recommended)**
- In Arduino IDE: Tools → Manage Libraries...
- Search for "Xeno Language"
- Click "Install"

**Method 2: Manual Installation**
- Download the latest library version from [Releases section](https://github.com/VLPLAY-Games/Xeno-Language/releases)
- In Arduino IDE: Sketch → Include Library → Add .ZIP Library...
- Select the downloaded `Xeno-Language-vX.X.X.zip` file

Usage examples can be found in: File → Examples → Xeno Language

### Minimal Arduino / ESP32 example
```cpp
#include <XenoLanguage.h>

XenoLanguage xeno;

void setup() {
  Serial.begin(115200);

  String program = R"(
    print "Hello from Xeno!"
    halt
  )";

  xeno.compile(program);
  xeno.run();
}

void loop() {
  // Optionally use xeno.step() for single-step execution or poll status
}
```

---

## Language overview

### Basic commands
- `print "text"` — print a literal string.  
- `print $var` — print variable value.  
- `set var expr` — assign variable (supports expressions).  
- `input var` — request input via Serial (stored as string, number or boolean).  
- `halt` — stop program execution.  
- `led <pin> on|off` — toggle an allowed GPIO pin.  
- `delay <ms>` — delay in milliseconds (bounded).  
- Arithmetic operators: `+`, `-`, `*`, `/`, `%`, `^` (power).  
- Functions: `abs()`, `sqrt()`, `sin()`, `cos()`, `tan()`, `max()`, `min()`.  
- Constants: `M_PI`, `M_E`, `M_TAU`, `M_SQRT2`, `M_SQRT3`, `P_LIGHT_SPEED`.  
- Control flow: `if ... then ... else ... endif`, `for var = start to end ... endfor`.  
- Boolean values: `true`, `false`.  
- Single-line comments use `//`.

### Quick syntax snippets
```
// Comments
print "Hello World"
led 13 on
delay 1000

// Variables
set x 10
set name "Xeno"
set flag true
print $x
print $flag

// Arithmetic
set result (x + 5) * 2

// Conditionals
if x > 5 then
print "Larger than 5"
endif

// Loops
for i = 1 to 10
print $i
endfor

// Boolean operations
set a true
set b false
if a == true then
print "a is true"
endif
```

### Operations & Comparisons
- Arithmetic: `+ - * / % ^`, `abs()`, `sqrt()`, `sin()`, `cos()`, `tan()`, `max()`, `min()`  
- Constants: `M_PI`, `M_E`, `M_TAU`, `M_SQRT2`, `M_SQRT3`, `P_LIGHT_SPEED`
- Comparisons: `== != < > <= >=`
- Control: `if/then/else/endif`, `for/endfor`  
- Peripherals: `print`, `led on/off`, `delay`  
- Data types: integers, floats, strings, booleans

---

## Virtual Machine (VM)

* Executes compiled bytecode using a runtime stack, variable table, and string table.
* Built-in safety checks: stack overflow, invalid opcode, divide-by-zero, bounds checking.
* API highlights (`class XenoLanguage`):

  * `bool compile(const String& source)` — compile source to bytecode.
  * `bool run()` — execute compiled bytecode.
  * `void step()` — execute a single VM instruction.
  * `void stop()` — stop execution.
  * `bool isRunning() const` — check running state.
  * `void printCompiledCode()` — print bytecode + string table / debug info.
  * `void setMaxInstructions(uint32_t max_instr)` — raise instruction limit.

  * `setStringLimit(256)`           // Max string length  
  * `setVariableNameLimit(32)`      // Max variable name length  
  * `setExpressionDepth(32)`        // Max expression nesting  
  * `setLoopDepth(16)`              // Max loop nesting  
  * `setIfDepth(16)`                // Max if/else nesting  
  * `setStackSize(256)`             // Stack memory size  

  * `setAllowedPins({2,3,13})`      // Configure allowed pins  
  * `addAllowedPin(5)`              // Add pin  
  * `removeAllowedPin(13)`          // Remove pin  

  * `compile_and_run("print 'Hello World'")` — convenience wrapper.

  * Version getters:  
    `getLanguageVersion()`.

---

## Security & Limits

### Default runtime settings

* `max_string_length = 256`
* `max_variable_name_length = 32`
* `max_expression_depth = 32`
* `max_loop_depth = 16`
* `max_if_depth = 16`
* `max_stack_size = 256`
* `current_max_instructions = 10000`
* `allowed_pins = { LED_BUILTIN }`

### Bounds (minimum / maximum limits)

* `MIN_STRING_LENGTH = 1`, `MAX_STRING_LENGTH_LIMIT = 4096`
* `MIN_VARIABLE_NAME_LENGTH = 1`, `MAX_VARIABLE_NAME_LENGTH_LIMIT = 256`
* `MIN_EXPRESSION_DEPTH = 1`, `MAX_EXPRESSION_DEPTH_LIMIT = 256`
* `MIN_LOOP_DEPTH = 1`, `MAX_LOOP_DEPTH_LIMIT = 64`
* `MIN_IF_DEPTH = 1`, `MAX_IF_DEPTH_LIMIT = 64`
* `MIN_STACK_SIZE = 16`, `MAX_STACK_SIZE_LIMIT = 2048`
* `MIN_INSTRUCTIONS_LIMIT = 1000`, `MAX_INSTRUCTIONS_LIMIT = 1000000`
* `MIN_PIN_NUMBER = 0`, `MAX_PIN_NUMBER = 255`

> All setters validate values; out-of-range values are rejected.

---

## Bytecode / Opcodes (summary)

Common VM opcodes include:
```
OP_NOP, OP_PRINT, OP_PRINT_NUM, OP_PUSH, OP_POP,
OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
OP_POW, OP_ABS, OP_SQRT, OP_MAX, OP_MIN,
OP_SIN, OP_COS, OP_TAN,
OP_STORE, OP_LOAD,
OP_JUMP, OP_JUMP_IF,
OP_INPUT, OP_DELAY, OP_LED_ON, OP_LED_OFF,
OP_PUSH_FLOAT, OP_PUSH_STRING, OP_PUSH_BOOL,
OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE,
OP_HALT
```

---

## Examples & Benchmarks
See `examples/` in the repository:
- `comparison.ino` — if/else + comparisons.  
- `float_string.ino` — floats and string handling.  
- `for_loop.ino` — for loops and blinking examples.  
- `input_max_min.ino` — input handling + math functions.  
- `math.ino`, `math2.ino` — math tests and benchmarks.  
- `benchmark.ino` — performance comparison between native C++ and Xeno VM.
- `bool.ino` — boolean variables and operations.

---

## Debugging tips
- Always open Serial (115200) for logs and input prompts.  
- If you see `CRITICAL ERROR: Stack overflow`, reduce expression complexity or increase `MAX_STACK_SIZE` carefully.  
- If GPIO commands fail, check the allowed pin list in `xeno_security.h`.  
- Use `printCompiledCode()` to debug compiled bytecode.

---

## Roadmap & Plans
🎯 Planned next steps:
- Continue actively developing core functionality.  
- Improve optimization and reduce VM overhead.  
- Add integration with **XenoOS** and better tooling for editing `.xeno` files.  
- Add more examples, CI pipelines, and PlatformIO templates.

---

## Contributing
Contributions, issues and pull requests are welcome. If you contribute, please:
- Respect the Apache 2.0 license and include clear commit messages.  
- Add small, focused PRs with tests/examples when possible.

---

## License
This project is licensed under the **Apache License 2.0**. See the `LICENSE` file for details.

**Xeno Language** — developed by **VL_PLAY Games**.
