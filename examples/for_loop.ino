#include "xenoLang/xeno_vm.h"
#include "xenoLang/xeno_compiler.h"

XenoCompiler compiler;
XenoVM vm;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with For Loops");
    Serial.println("======================");
    
    // Example: For loops
    String forLoopProgram = R"(
        print "For Loop Demo"
        
        // Simple for loop
        print "Counting from 1 to 5:"
        for i = 1 to 5
            print $i
        endfor
        
        // For loop with calculations
        print "Squares from 1 to 4:"
        for x = 1 to 4
            set square x * x
            print $square
        endfor
        
        // Nested for loops
        print "Multiplication table (2-3):"
        for a = 2 to 3
            for b = 1 to 3
                set result a * b
                print $result
            endfor
        endfor
        
        // For loop with hardware
        print "Blinking LED 3 times:"
        for blink_count = 1 to 3
            print "Blink"
            led 2 on
            delay 300
            led 2 off
            delay 300
        endfor
        
        // Complex for loop with conditions
        print "Even numbers from 2 to 10:"
        for num = 2 to 10
            if num % 2 == 0 then
                print $num
            endif
        endfor
        
        print "For loop demo completed!"
        halt
    )";
    
    Serial.println("\n--- Compiling for loop program ---");
    compiler.compile(forLoopProgram);
    compiler.printCompiledCode();
    
    Serial.println("\n--- Executing for loop program ---");
    vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
    vm.run();
}

void loop() {
    delay(1000);
}
