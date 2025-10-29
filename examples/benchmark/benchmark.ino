#include "xenoLang/xeno.h"

class Benchmark {
private:
    unsigned long cpp_time;
    unsigned long xeno_time;

public:
    void runCppBenchmark() {
        Serial.println("=== C++ NATIVE BENCHMARK ===");
        unsigned long start = micros();
        
        // Integer arithmetic
        int sum = 0;
        for (int i = 0; i <= 100; i++) {
            sum = sum + i;
        }
        Serial.println(sum);
        
        // Float arithmetic
        float fsum = 2.2f;
        for (int i = 0; i <= 100; i++) {
            fsum = fsum + (float)i * 1.1;
        }
        Serial.println(fsum);
        
        // String operations - extended
        String result = "";
        String temp1 = "Hello";
        String temp2 = "World";
        String temp3 = "Test";
        result = temp1 + " " + temp2 + " " + temp3;
        Serial.println(result);
        // String concatenation
        for (int i = 0; i <= 100; i++) {
            result = temp1 + " " + temp2 + " " + temp3 + " " + String(i);
        }
        Serial.println(result);
        
        // Multiple concatenation
        String longString = "";
        for (int i = 0; i <= 50; i++) {
            longString = longString + "a" + "b" + "c";
        }
        Serial.println(longString);
        
        // String comparisons
        int compareCount = 0;
        for (int i = 0; i <= 100; i++) {
            if (temp1 == "Hello") compareCount++;
            if (temp2 != temp3) compareCount++;
        }
        Serial.println(compareCount);
        
        unsigned long end = micros();
        cpp_time = end - start;
        
        Serial.println("Time: " + String(cpp_time) + " microseconds");
        Serial.println();
    }

    void runXenoBenchmark() {
        String source_code = 
            "// Integer arithmetic\n"
            "set sum 0\n"
            "set i 0\n"
            "for i = 0 to 100\n"
            "    set sum sum + i\n"
            "endfor\n"
            "print $sum\n"
            "\n"
            "// Float arithmetic\n"
            "set fsum 2.2\n"
            "set j 0\n"
            "for j = 0 to 100\n"
            "    set fsum fsum + j * 1.1\n"
            "endfor\n"
            "print $fsum\n"
            "\n"
            "// String operations\n"
            "set temp1 \"Hello\"\n"
            "set temp2 \"World\"\n"
            "set temp3 \"Test\"\n"
            "set result temp1 + \" \" + temp2 + \" \" + temp3\n"
            "print $result\n"
            "\n"
            "// String concatenation\n"
            "set k 0\n"
            "for k = 0 to 100\n"
            "    set result temp1 + \" \" + temp2 + \" \" + k\n"
            "endfor\n"
            "print $result\n"
            "\n"
            "// Multiple concatenation\n"
            "set longString \"\"\n"
            "set m 0\n"
            "for m = 0 to 50\n"
            "    set longString longString + \"a\" + \"b\" + \"c\"\n"
            "endfor\n"
            "print $longString\n"
            "\n"
            "// String comparison\n"
            "set compareCount 0\n"
            "set n 0\n"
            "for n = 0 to 100\n"
            "    if temp1 == \"Hello\" then\n"
            "        set compareCount compareCount + 1\n"
            "    endif\n"
            "    if temp2 != temp3 then\n"
            "        set compareCount compareCount + 1\n"
            "    endif\n"
            "endfor\n"
            "print $compareCount\n"
            "\n"
            "halt";
            
        
        Xeno xeno;
        
        Serial.println("=== XENO VM BENCHMARK ===");
        xeno.compile(source_code);
        xeno.setMaxInstructions(200000); // Increased instruction limit
        unsigned long start = micros();
        xeno.run();
        
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
    Serial.println("XENO LANGUAGE BENCHMARK WITH STRING OPERATIONS AND ARITHMETIC");
    Serial.println("=================================================");
    Serial.println();
    
    // Run tests
    bench.runCppBenchmark();
    bench.runXenoBenchmark();
    bench.runFinalComparison();
    
    Serial.println();
    Serial.println("Benchmark completed!");
}

void loop() {
    // Empty
}
