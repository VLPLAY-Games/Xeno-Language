#include <XenoLanguage.h>

XenoLanguage xeno;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with Float and String Support");
    Serial.println("=====================================");
    
    // Example with float and string variables
    String mixedTypeProgram = R"(
        print "Mixed Type Demo"
        
        // Integer variable
        set a 10
        print "Integer a = "
        print $a
        
        // Float variable  
        set b 3.14
        print "Float b = "
        print $b
        
        // String variable
        set name "XenoLang"
        print "String name = "
        print $name
        
        // Math operations with floats
        set result a + b
        print "a + b = "
        print $result
        
        set result b * 2.0
        print "b * 2.0 = "
        print $result
        
        // String concatenation
        set greeting "Hello " + name
        print $greeting
        
        // Float comparisons
        if b > 3.0 then
            print "b is greater than 3.0"
        endif
        
        // Mixed type operations
        set mixed a + b
        print "Mixed type result: "
        print $mixed
        
        // For loop with float
        print "Counting with float:"
        set start 1.0
        set end 3.0
        for i = start to end
            print $i
        endfor
        
        print "Demo completed!"
        halt
    )";
    
    Serial.println("\n--- Compiling mixed type program ---");
    xeno.compile(mixedTypeProgram);
    
    Serial.println("\n--- Executing mixed type program ---");
    xeno.run();
}

void loop() {
    delay(1000);
}