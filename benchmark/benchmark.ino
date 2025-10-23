#include <Arduino.h>
#include "../xenoLang/xeno_vm.h"
#include "../xenoLang/xeno_compiler.h"

class Benchmark {
private:
    unsigned long cpp_time;
    unsigned long xeno_time;

public:
    void runCppBenchmark() {
        unsigned long start = micros();
        
        // Integer arithmetic
        int sum = 0;
        for (int i = 0; i < 100; i++) {
            sum = sum + i;
        }
        
        // Float arithmetic
        float fsum = 0.0f;
        for (int i = 0; i < 100; i++) {
            fsum = fsum + (float)i;
        }
        
        // String operations - расширенные
        String result = "";
        String temp1 = "Hello";
        String temp2 = "World";
        String temp3 = "Test";
        
        // Конкатенация строк
        for (int i = 0; i < 50; i++) {
            result = temp1 + " " + temp2 + " " + String(i);
        }
        
        // Множественная конкатенация
        String longString = "";
        for (int i = 0; i < 30; i++) {
            longString = longString + "a" + "b" + "c";
        }
        
        // Сравнение строк
        int compareCount = 0;
        for (int i = 0; i < 50; i++) {
            if (temp1 == "Hello") compareCount++;
            if (temp2 != temp3) compareCount++;
        }
        
        unsigned long end = micros();
        cpp_time = end - start;
        
        Serial.println("=== C++ NATIVE BENCHMARK ===");
        Serial.println("Time: " + String(cpp_time) + " microseconds");
        Serial.println("Sum: " + String(sum));
        Serial.println("Float Sum: " + String(fsum, 2));
        Serial.println("String length: " + String(longString.length()));
        Serial.println("Compare count: " + String(compareCount));
        Serial.println();
    }

    void runXenoBenchmark() {
        // Расширенный код с работой со строками
        String source_code = 
            "// Integer arithmetic\n"
            "set sum 0\n"
            "set i 0\n"
            "for i = 0 to 100\n"
            "    set sum sum + i\n"
            "endfor\n"
            "\n"
            "// Float arithmetic\n"
            "set fsum 0\n"
            "set j 0\n"
            "for j = 0 to 100\n"
            "    set fsum fsum + j\n"
            "endfor\n"
            "\n"
            "// String operations\n"
            "set temp1 \"Hello\"\n"
            "set temp2 \"World\"\n"
            "set temp3 \"Test\"\n"
            "set result \"\"\n"
            "\n"
            "// Конкатенация строк\n"
            "set k 0\n"
            "for k = 0 to 50\n"
            "    set result temp1 + \" \" + temp2 + \" \" + k\n"
            "endfor\n"
            "\n"
            "// Множественная конкатенация\n"
            "set longString \"\"\n"
            "set m 0\n"
            "for m = 0 to 30\n"
            "    set longString longString + \"a\" + \"b\" + \"c\"\n"
            "endfor\n"
            "\n"
            "// Сравнение строк\n"
            "set compareCount 0\n"
            "set n 0\n"
            "for n = 0 to 50\n"
            "    if temp1 == \"Hello\" then\n"
            "        set compareCount compareCount + 1\n"
            "    endif\n"
            "    if temp2 != temp3 then\n"
            "        set compareCount compareCount + 1\n"
            "    endif\n"
            "endfor\n"
            "\n"
            "halt";
        
        XenoCompiler compiler;
        
        Serial.println("=== XENO VM BENCHMARK ===");
        compiler.compile(source_code);
        XenoVM vm;
        vm.setMaxInstructions(200000); // Увеличили лимит для строковых операций
        unsigned long start = micros();
        vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
        vm.run();
        
        unsigned long end = micros();
        xeno_time = end - start;
        
        Serial.println("Time: " + String(xeno_time) + " microseconds");
        Serial.println();
    }

    void runFinalComparison() {
        Serial.println("=== FINAL PERFORMANCE COMPARISON ===");
        
        if (cpp_time == 0) cpp_time = 1;
        if (xeno_time == 0) xeno_time = 1;
        
        Serial.println("C++ execution time: " + String(cpp_time) + " μs");
        Serial.println("Xeno execution time: " + String(xeno_time) + " μs");
        
        float ratio = (float)xeno_time / (float)cpp_time;
        float percentage = ((float)cpp_time / (float)xeno_time) * 100.0f;
        
        Serial.println();
        Serial.println("Xeno is " + String(ratio, 1) + "x slower than C++");
        Serial.println("Xeno achieves " + String(percentage, 1) + "% of C++ performance");
        
        Serial.println();
        Serial.println("=== PERFORMANCE ANALYSIS ===");
        if (ratio < 5) {
            Serial.println("✅ Excellent");
        } else if (ratio < 20) {
            Serial.println("⚡ Good");
        } else if (ratio < 50) {
            Serial.println("📊 Acceptable");
        } else if (ratio < 100) {
            Serial.println("🐌 Slow");
        } else {
            Serial.println("🚨 Very Slow");
        }
    }
};

Benchmark bench;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println();
    Serial.println("XENO LANGUAGE BENCHMARK WITH STRING OPERATIONS");
    Serial.println("=================================================");
    Serial.println();
    
    // Запускаем тесты
    bench.runCppBenchmark();
    bench.runXenoBenchmark();
    bench.runFinalComparison();
    
    Serial.println();
    Serial.println("Benchmark completed!");
}

void loop() {
    // Empty
}