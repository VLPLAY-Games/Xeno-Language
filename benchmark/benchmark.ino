#include <Arduino.h>
#include "../xenoLang/xeno_vm.h"
#include "../xenoLang/xeno_compiler.h"
#include <chrono>

class Benchmark {
private:
    unsigned long cpp_time;
    unsigned long xeno_time;
    unsigned long xeno_optimized_time;

public:
    void runCppBenchmark() {
        unsigned long start = micros();
        
        // Integer arithmetic
        int sum = 0;
        for (int i = 0; i < 10000; i++) {
            sum += i * 2 - 1;
        }
        
        // Float arithmetic
        float fsum = 0.0f;
        for (int i = 0; i < 10000; i++) {
            fsum += i * 1.5f - 0.5f;
        }
        
        // String operations
        String result = "";
        for (int i = 0; i < 1000; i++) {
            result += "test";
        }
        
        unsigned long end = micros();
        cpp_time = end - start;
        
        Serial.println("C++ Benchmark:");
        Serial.println("  Sum: " + String(sum));
        Serial.println("  Float Sum: " + String(fsum, 2));
        Serial.println("  String length: " + String(result.length()));
        Serial.println("  Time: " + String(cpp_time) + " microseconds");
    }

    void runXenoBenchmark() {
        String source_code = 
            "set sum 0\n"
            "set fsum 0.0\n"
            "set i 0\n"
            "for i = 0 to 10000\n"
            "    set sum sum + i * 2 - 1\n"
            "    set fsum fsum + i * 1.5 - 0.5\n"
            "endfor\n"
            "set result \"\"\n"
            "set j 0\n"
            "for j = 0 to 1000\n"
            "    set result result + \"test\"\n"
            "endfor\n"
            "print \"Xeno Benchmark:\"\n"
            "print \"Sum: $\"\n"
            "print $sum\n"
            "print \"Float Sum: $\"\n"
            "print $fsum\n"
            "print \"String operations completed\"\n"
            "halt";
        
        XenoCompiler compiler;
        unsigned long start = micros();
        
        compiler.compile(source_code);
        XenoVM vm;
        vm.loadProgram(compiler.getBytecode(), compiler.getStringTable());
        vm.run();
        
        unsigned long end = micros();
        xeno_time = end - start;
        
        Serial.println("Xeno Time: " + String(xeno_time) + " microseconds");
    }

    void runComparison() {
        Serial.println("\n=== PERFORMANCE COMPARISON ===");
        Serial.println("C++ execution time: " + String(cpp_time) + " μs");
        Serial.println("Xeno execution time: " + String(xeno_time) + " μs");
        
        if (cpp_time > 0) {
            float ratio = (float)xeno_time / (float)cpp_time;
            Serial.println("Xeno is " + String(ratio, 1) + "x slower than C++");
        }
        
        Serial.println("=== MEMORY USAGE ===");
        Serial.println("Stack size: 64 values");
        Serial.println("String table: dynamic");
        Serial.println("Variable storage: std::map");
    }

    void runSpecificTests() {
        Serial.println("\n=== SPECIFIC OPERATION TESTS ===");
        
        // Test 1: Integer arithmetic
        unsigned long start = micros();
        int a = 0;
        for (int i = 0; i < 100000; i++) {
            a = a * 1.1 + i - 5;
        }
        unsigned long int_time = micros() - start;
        
        // Test 2: Float arithmetic  
        start = micros();
        float b = 0.0f;
        for (int i = 0; i < 100000; i++) {
            b = b * 1.1f + i - 5.0f;
        }
        unsigned long float_time = micros() - start;
        
        // Test 3: Function calls
        start = micros();
        for (int i = 0; i < 10000; i++) {
            testFunction(i);
        }
        unsigned long func_time = micros() - start;
        
        Serial.println("Integer ops: " + String(int_time) + " μs");
        Serial.println("Float ops: " + String(float_time) + " μs");
        Serial.println("Function calls: " + String(func_time) + " μs");
    }

private:
    int testFunction(int x) {
        return x * x + x - 1;
    }
};

Benchmark bench;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== XENO LANGUAGE BENCHMARK ===");
    
    bench.runCppBenchmark();
    Serial.println();
    bench.runXenoBenchmark();
    bench.runComparison();
    bench.runSpecificTests();
}

void loop() {
    // Empty
}