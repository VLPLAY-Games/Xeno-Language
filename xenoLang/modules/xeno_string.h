#ifndef XENO_STRING_H
#define XENO_STRING_H

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

class LString {
private:
    char* _data;
    uint16_t _length;
    uint16_t _capacity;
    
    // Приватные методы для управления памятью
    bool resize(uint16_t new_capacity) {
        if (new_capacity <= _capacity) return true;
        
        char* new_data = (char*)realloc(_data, new_capacity + 1); // +1 для нуль-терминатора
        if (!new_data) return false;
        
        _data = new_data;
        _capacity = new_capacity;
        return true;
    }

    void freeMemory() {
        if (_data) {
            free(_data);
            _data = nullptr;
        }
        _length = 0;
        _capacity = 0;
    }

    // Вспомогательная функция для max
    uint16_t max(uint16_t a, uint16_t b) {
        return (a > b) ? a : b;
    }

public:
    // Конструкторы
    LString() : _data(nullptr), _length(0), _capacity(0) {}
    
    LString(const char* str) : _data(nullptr), _length(0), _capacity(0) {
        if (str) append(str);
    }
    
    LString(char c) : _data(nullptr), _length(0), _capacity(0) {
        append(c);
    }
    
    LString(int value) : _data(nullptr), _length(0), _capacity(0) {
        append(value);
    }
    
    LString(float value, uint8_t decimals = 2) : _data(nullptr), _length(0), _capacity(0) {
        append(value, decimals);
    }

    // Деструктор
    ~LString() {
        freeMemory();
    }

    // Копирующий конструктор
    LString(const LString& other) : _data(nullptr), _length(0), _capacity(0) {
        if (other._data) {
            append(other._data);
        }
    }

    // Move конструктор
    LString(LString&& other) noexcept : _data(other._data), _length(other._length), _capacity(other._capacity) {
        other._data = nullptr;
        other._length = 0;
        other._capacity = 0;
    }

    // Копирующее присваивание
    LString& operator=(const LString& other) {
        if (this != &other) {
            clear();
            if (other._data) {
                append(other._data);
            }
        }
        return *this;
    }

    // Move присваивание
    LString& operator=(LString&& other) noexcept {
        if (this != &other) {
            freeMemory();
            _data = other._data;
            _length = other._length;
            _capacity = other._capacity;
            other._data = nullptr;
            other._length = 0;
            other._capacity = 0;
        }
        return *this;
    }
    
    // Основные методы
    uint16_t length() const { return _length; }
    uint16_t capacity() const { return _capacity; }
    bool isEmpty() const { return _length == 0; }
    const char* c_str() const { return _data ? _data : ""; }
    
    void clear() {
        if (_data) {
            _data[0] = '\0';
            _length = 0;
        }
    }
    
    // Операции добавления данных
    LString& append(const char* str) {
        if (!str) return *this;
        
        uint16_t str_len = strlen(str);
        if (str_len == 0) return *this;
        
        uint16_t new_length = _length + str_len;
        if (new_length > _capacity && !resize(max(_capacity * 2, new_length))) {
            return *this; // Ошибка выделения памяти
        }
        
        // Если _data еще не выделен, нужно выделить
        if (!_data && !resize(new_length)) {
            return *this;
        }
        
        strcpy(_data + _length, str);
        _length = new_length;
        return *this;
    }

    LString& append(char c) {
        uint16_t new_length = _length + 1;
        if (new_length > _capacity && !resize(max(_capacity * 2, new_length + 16))) {
            return *this;
        }
        
        // Если _data еще не выделен, нужно выделить
        if (!_data && !resize(16)) {
            return *this;
        }
        
        _data[_length++] = c;
        _data[_length] = '\0';
        return *this;
    }

    LString& append(int value) {
        char buffer[12]; // Достаточно для int32_t
        snprintf(buffer, sizeof(buffer), "%d", value);
        return append(buffer);
    }

    LString& append(float value, uint8_t decimals = 2) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
        return append(buffer);
    }
    
    // Операторы
    LString& operator+=(const char* str) { return append(str); }
    LString& operator+=(char c) { return append(c); }
    LString& operator+=(int value) { return append(value); }
    LString& operator+=(float value) { return append(value); }
    
    // Операторы доступа к символам
    char operator[](uint16_t index) const {
        return (index < _length && _data) ? _data[index] : '\0';
    }

    char& operator[](uint16_t index) {
        static char dummy = '\0';
        if (index < _length && _data) {
            return _data[index];
        }
        return dummy;
    }
    
    // Сравнения
    bool equals(const char* str) const {
        if (!_data && !str) return true;
        if (!_data || !str) return false;
        return strcmp(_data, str) == 0;
    }

    bool equals(const LString& other) const {
        return equals(other._data);
    }
    
    bool operator==(const char* str) const { return equals(str); }
    bool operator==(const LString& other) const { return equals(other); }
    bool operator!=(const char* str) const { return !equals(str); }
    bool operator!=(const LString& other) const { return !equals(other); }

    // Утилиты
    int toInt() const {
        return _data ? atoi(_data) : 0;
    }

    float toFloat() const {
        return _data ? atof(_data) : 0.0f;
    }

    LString substring(uint16_t start, uint16_t end = 0xFFFF) const {
        if (!_data || start >= _length) return LString();
        
        if (end == 0xFFFF || end > _length) end = _length;
        if (start >= end) return LString();
        
        LString result;
        for (uint16_t i = start; i < end; i++) {
            result.append(_data[i]);
        }
        return result;
    }

    int indexOf(char c) const {
        if (!_data) return -1;
        const char* found = strchr(_data, c);
        return found ? (found - _data) : -1;
    }

    void trim() {
        if (!_data) return;
        
        // Trim справа
        int16_t end = _length - 1;
        while (end >= 0 && isspace(_data[end])) {
            _data[end--] = '\0';
            _length--;
        }
        
        // Trim слева
        uint16_t start = 0;
        while (start < _length && isspace(_data[start])) {
            start++;
        }
        
        if (start > 0) {
            memmove(_data, _data + start, _length - start + 1);
            _length -= start;
        }
    }
};

// Глобальные операторы для конкатенации
inline LString operator+(const LString& lhs, const char* rhs) {
    LString result(lhs);
    return result.append(rhs);
}

inline LString operator+(const LString& lhs, const LString& rhs) {
    LString result(lhs);
    return result.append(rhs.c_str());
}

inline LString operator+(const char* lhs, const LString& rhs) {
    LString result(lhs);
    return result.append(rhs.c_str());
}

#endif // XENO_STRING_H