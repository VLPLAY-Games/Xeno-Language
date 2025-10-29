#include "xenoLang/xeno.h"

Xeno xeno;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with Comparisons and If-Else");
    Serial.println("====================================");
    
    // Example: Comparisons and conditional logic
    String logicProgram = R"(
        print "Comparisons and If-Else Demo"
        
        // Simple comparisons
        set a 10
        set b 20
        
        print "Testing a=10, b=20:"
        
        if a == b then
            print "a equals b"
        else
            print "a does not equal b"
        endif
        
        if a < b then
            print "a is less than b"
        else
            print "a is not less than b"
        endif
        
        if a > b then
            print "a is greater than b"
        else
            print "a is not greater than b"
        endif
        
        // Complex conditions
        set x 15
        set y 15
        
        if x == y then
            print "x equals y"
            set result 100
            print "Setting result to 100"
        else
            print "x does not equal y"
            set result 200
            print "Setting result to 200"
        endif
        
        print "Final result: "
        print $result
        
        // Multiple conditions with math
        set num1 25
        set num2 30
        
        if num1 + 5 == num2 then
            print "num1 + 5 equals num2"
        else
            print "num1 + 5 does not equal num2"
        endif
        
        if num1 * 2 > num2 then
            print "num1 * 2 is greater than num2"
        else
            print "num1 * 2 is not greater than num2"
        endif
        
        // Testing with hardware
        set counter 0
        
        if counter == 0 then
            print "Counter is zero - turning LED on"
            led 2 on
            delay 1000
            led 2 off
        else
            print "Counter is not zero"
        endif
        
        // Complex expression in condition
        set value 17
        
        if value % 2 == 0 then
            print "Value is even"
        else
            print "Value is odd"
        endif
        
        // Testing all comparison operators
        set test1 10
        set test2 20
        
        print "Testing all comparison operators:"
        
        if test1 == test2 then
            print "10 == 20: true"
        else
            print "10 == 20: false"
        endif
        
        if test1 != test2 then
            print "10 != 20: true"
        else
            print "10 != 20: false"
        endif
        
        if test1 < test2 then
            print "10 < 20: true"
        else
            print "10 < 20: false"
        endif
        
        if test1 > test2 then
            print "10 > 20: true"
        else
            print "10 > 20: false"
        endif
        
        if test1 <= test2 then
            print "10 <= 20: true"
        else
            print "10 <= 20: false"
        endif
        
        if test1 >= test2 then
            print "10 >= 20: true"
        else
            print "10 >= 20: false"
        endif
        
        print "Conditional logic demo completed!"
        halt
    )";
    
    Serial.println("\n--- Compiling logic program ---");
    xeno.compile(logicProgram);
    
    Serial.println("\n--- Executing logic program ---");
    xeno.run();
}

void loop() {
    delay(1000);
}