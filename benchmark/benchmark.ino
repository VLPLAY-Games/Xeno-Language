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
        
        // Integer arithmetic - очень простые операции
        int sum = 0;
        for (int i = 0; i < 100; i++) {
            sum = sum + i; // Простое сложение, без сложных выражений
        }
        
        // Float arithmetic
        float fsum = 0.0f;
        for (int i = 0; i < 100; i++) {
            fsum = fsum + (float)i; // Простое сложение
        }
        
        // String operations
        String result = "";
        for (int i = 0; i < 10; i++) {
            result = result + "a"; // Минимальная конкатенация
        }
        
        unsigned long end = micros();
        cpp_time = end - start;
        
        Serial.println("=== C++ NATIVE BENCHMARK ===");
        Serial.println("Time: " + String(cpp_time) + " microseconds");
        Serial.println("Sum: " + String(sum));
        Serial.println("Float Sum: " + String(fsum, 2));
        Serial.println("String length: " + String(result.length()));
        Serial.println();
    }

    void runXenoBenchmark() {
        // СУПЕР-ПРОСТОЙ код, минимальное использование стека
        String source_code = 
            "set sum 0\n"
            "set i 0\n"
            "for i = 0 to 100\n"
            "    set sum sum + i\n"  // Только одна операция за раз
            "endfor\n"
            "set fsum 0\n"
            "set j 0\n"
            "for j = 0 to 100\n"
            "    set fsum fsum + j\n"  // Только одна операция за раз
            "endfor\n"
            "set s \"\"\n"
            "set k 0\n"
            "for k = 0 to 10\n"
            "    set s s + \"a\"\n"    // Минимальные строковые операции
            "endfor\n"
            "halt";
        
        XenoCompiler compiler;
        unsigned long start = micros();
        Serial.println("=== XENO VM BENCHMARK ===");
        compiler.compile(source_code);
        XenoVM vm;
        vm.setMaxInstructions(100000); // Очень высокий лимит
        vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
        vm.run();
        
        unsigned long end = micros();
        xeno_time = end - start;
        
        Serial.println("Time: " + String(xeno_time) + " microseconds");
        Serial.println();
    }

    void runFinalComparison() {
        Serial.println("=== FINAL PERFORMANCE COMPARISON ===");
        
        if (cpp_time == 0) cpp_time = 1; // Избегаем деления на ноль
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
        } else {
            Serial.println("🐌 Slow");
        }
    }
};

Benchmark bench;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println();
    Serial.println("🚀 XENO LANGUAGE BENCHMARK");
    Serial.println("===========================");
    Serial.println();
    
    // Запускаем тесты
    bench.runCppBenchmark();
    bench.runXenoBenchmark();
    bench.runFinalComparison();
    
    Serial.println();
    Serial.println("✨ Benchmark completed!");
    Serial.println();
}

void loop() {
    // Empty
}