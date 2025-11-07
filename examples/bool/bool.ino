#include <XenoLanguage.h>

XenoLanguage xeno;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with Boolean Support");
    Serial.println("============================");

    // Example program using boolean variables
    String boolProgram = R"(
        set flag1 true
        set flag2 false
        
        print "Boolean Operations:"
        print "flag1 = "
        print $flag1
        print "flag2 = "
        print $flag2
        
        // Boolean comparison
        if flag1 == true then
            print "flag1 is true"
        endif
        
        if flag2 != true then
            print "flag2 is false"
        endif
        
        halt
    )";

    Serial.println("\n--- Compiling boolean program ---");
    xeno.compile(boolProgram);
    
    Serial.println("\n--- Executing boolean program ---");
    xeno.run();
}

void loop() {
    delay(1000);
}