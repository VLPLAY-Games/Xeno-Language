#include "xenoLang/xeno.h"

Xeno xeno;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with Math Operations");
    Serial.println("==================================");
    
    // Example: Fixed math operations
    String mathProgram = R"(
        print "Math Operations Demo"
        
        // Modulo (remainder) operations
        set a 17 % 5
        print "17 % 5 = "
        print $a
        
        set b 23 % 7
        print "23 % 7 = "
        print $b
        
        set c 100 % 3
        print "100 % 3 = "
        print $c
        
        // Absolute value operations
        set d abs(-10)
        print "abs(-10) = "
        print $d
        
        set e abs(15)
        print "abs(15) = "
        print $e
        
        set f abs(0)
        print "abs(0) = "
        print $f
        
        // Power (exponentiation) operations
        set g 2 ^ 3
        print "2 ^ 3 = "
        print $g
        
        set h 5 ^ 2
        print "5 ^ 2 = "
        print $h
        
        set i 3 ^ 4
        print "3 ^ 4 = "
        print $i
        
        // Complex expressions with new operations
        set j (17 % 5) ^ 2
        print "(17 % 5) ^ 2 = "
        print $j
        
        set k abs(-10) + abs(20)
        print "abs(-10) + abs(20) = "
        print $k
        
        set m (2 ^ 3) % 5
        print "(2 ^ 3) % 5 = "
        print $m
        
        set n abs(-15) ^ 2
        print "abs(-15) ^ 2 = "
        print $n
        
        // Combined operations
        set result (10 + abs(-5)) % 3 ^ 2
        print "(10 + abs(-5)) % 3 ^ 2 = "
        print $result
        
        print "All math operations work correctly!"
        halt
    )";
    
    Serial.println("\n--- Compiling math program ---");
    xeno.compile(mathProgram);
    
    Serial.println("\n--- Executing math program ---");
    xeno.run();
}

void loop() {
    delay(1000);
}