#include <XenoLanguage.h>

XenoLanguage xeno;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with Input and Math");
    Serial.println("=====================================");
    
    // Example with input and math (max, min)
    String mixedTypeProgram = R"(
        set x 16
        set result sqrt(x)        // result = 4
        set a 10
        set b 20
        set maximum max(a, b)     // maximum = 20
        set minimum min(a, b)     // minimum = 10

        input name               // Input required
        print "Hello, "
        print $name

        print "Maximum is: "
        print $maximum
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