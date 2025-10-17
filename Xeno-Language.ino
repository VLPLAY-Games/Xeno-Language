// Example of using Xeno VM with proper variable names
#include "xenoLang/xeno_vm.h"
#include "xenoLang/xeno_compiler.h"

XenoCompiler compiler;
XenoVM vm;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with Variables Example");
    Serial.println("==============================");
    
    // Example: Using proper variable names (letters only, no numbers)
    String variableProgram = R"(
        print "Variables Demo"
        
        // Simple expressions with proper variable names
        set value_a1 300 + 300
        print "value_a1 = 300 + 300 = "
        print $value_a1
        
        set value_b 100
        set result_a value_a1 - value_b
        print "result_a = value_a1 - value_b = "
        print $result_a
        
        set num_a 10
        set num_b 5
        set product num_a * num_b
        print "product = num_a * num_b = "
        print $product
        
        set dividend 100
        set divisor 4
        set quotient dividend / divisor
        print "quotient = dividend / divisor = "
        print $quotient
        
        // Complex expressions with operator precedence
        set x_val 50
        set y_val 25
        set z_result x_val + y_val * 2
        print "z_result = x_val + y_val * 2 = "
        print $z_result
        
        set w_result (x_val + y_val) * 2
        print "w_result = (x_val + y_val) * 2 = "
        print $w_result
        
        set final_result 10 + 20 * 3 - 15 / 3
        print "final_result = 10 + 20 * 3 - 15 / 3 = "
        print $final_result
        
        // Expressions with multiple variables
        set first_num 8
        set second_num 4
        set calc_result first_num * second_num + 10
        print "calc_result = first_num * second_num + 10 = "
        print $calc_result
        
        set complex_result (first_num + second_num) * (first_num - second_num)
        print "complex_result = (first_num + second_num) * (first_num - second_num) = "
        print $complex_result
        
        print "All tests completed successfully!"
        halt
    )";
    
    Serial.println("\n--- Compiling variable program ---");
    compiler.compile(variableProgram);
    compiler.printCompiledCode();
    
    Serial.println("\n--- Executing variable program ---");
    vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
    vm.run();
}

void loop() {
    delay(1000);
}