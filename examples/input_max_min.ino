#include "xenoLang/xeno_vm.h"
#include "xenoLang/xeno_compiler.h"

XenoCompiler compiler;
XenoVM vm;
XenoSecurity sec;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Xeno VM with Input and Math");
    Serial.println("=====================================");
    
    // Example with float and string variables
    String mixedTypeProgram = R"(
        // Пример программы с новыми функциями
        set x 16
        set result sqrt(x)        // result = 4
        set a 10
        set b 20
        set maximum max(a, b)     // maximum = 20
        set minimum min(a, b)     // minimum = 10

        input name               // Запрос ввода от пользователя
        print "Hello, "
        print $name

        print "Maximum is: "
        print $maximum
        halt
    )";
    
    Serial.println("\n--- Compiling mixed type program ---");
    compiler.compile(mixedTypeProgram);
    compiler.printCompiledCode();
    
    Serial.println("\n--- Executing mixed type program ---");
    vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
    vm.run();
}

void loop() {
    delay(1000);
}