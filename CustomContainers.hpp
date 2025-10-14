#ifndef CUSTOM_CONTAINERS_HPP
#define CUSTOM_CONTAINERS_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>

/**
 * CUSTOM CONTAINER IMPLEMENTATIONS
 * 
 * This file provides custom implementations of dynamic containers
 * to replace std::vector and other STL containers, demonstrating
 * mastery of data structure implementation and memory management.
 */

// Custom Dynamic Array Implementation
template<typename T>
class DynamicArray {
private:
    T* data;
    size_t size_;
    size_t capacity_;
    static const size_t INITIAL_CAPACITY = 4;
    static constexpr double GROWTH_FACTOR = 1.5;

    void resize() {
        size_t newCapacity = static_cast<size_t>(capacity_ * GROWTH_FACTOR);
        if (newCapacity <= capacity_) {
            newCapacity = capacity_ + 1; // Ensure growth
        }
        
        T* newData = new T[newCapacity];
        
        // Copy existing elements
        for (size_t i = 0; i < size_; ++i) {
            newData[i] = data[i];
        }
        
        // Clean up old data
        delete[] data;
        data = newData;
        capacity_ = newCapacity;
    }

public:
    // Constructors
    DynamicArray() : data(nullptr), size_(0), capacity_(0) {
        data = new T[INITIAL_CAPACITY];
        capacity_ = INITIAL_CAPACITY;
    }
    
    DynamicArray(size_t initialSize) : size_(initialSize), capacity_(initialSize) {
        if (capacity_ < INITIAL_CAPACITY) {
            capacity_ = INITIAL_CAPACITY;
        }
        data = new T[capacity_];
    }
    
    DynamicArray(std::initializer_list<T> init) : size_(init.size()), capacity_(init.size()) {
        if (capacity_ < INITIAL_CAPACITY) {
            capacity_ = INITIAL_CAPACITY;
        }
        data = new T[capacity_];
        
        size_t i = 0;
        for (const T& item : init) {
            data[i++] = item;
        }
    }
    
    // Copy constructor
    DynamicArray(const DynamicArray& other) : size_(other.size_), capacity_(other.capacity_) {
        data = new T[capacity_];
        for (size_t i = 0; i < size_; ++i) {
            data[i] = other.data[i];
        }
    }
    
    // Move constructor
    DynamicArray(DynamicArray&& other) noexcept : data(other.data), size_(other.size_), capacity_(other.capacity_) {
        other.data = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    
    // Destructor
    ~DynamicArray() {
        delete[] data;
    }
    
    // Assignment operators
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] data;
            size_ = other.size_;
            capacity_ = other.capacity_;
            data = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }
    
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    // Element access
    T& operator[](size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }
    
    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }
    
    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }
    
    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }
    
    T& front() {
        if (size_ == 0) {
            throw std::out_of_range("Array is empty");
        }
        return data[0];
    }
    
    const T& front() const {
        if (size_ == 0) {
            throw std::out_of_range("Array is empty");
        }
        return data[0];
    }
    
    T& back() {
        if (size_ == 0) {
            throw std::out_of_range("Array is empty");
        }
        return data[size_ - 1];
    }
    
    const T& back() const {
        if (size_ == 0) {
            throw std::out_of_range("Array is empty");
        }
        return data[size_ - 1];
    }
    
    // Capacity
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    
    void reserve(size_t newCapacity) {
        if (newCapacity > capacity_) {
            T* newData = new T[newCapacity];
            for (size_t i = 0; i < size_; ++i) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity_ = newCapacity;
        }
    }
    
    void shrink_to_fit() {
        if (size_ < capacity_) {
            T* newData = new T[size_];
            for (size_t i = 0; i < size_; ++i) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity_ = size_;
        }
    }
    
    // Modifiers
    void clear() {
        size_ = 0;
    }
    
    void push_back(const T& value) {
        if (size_ >= capacity_) {
            resize();
        }
        data[size_++] = value;
    }
    
    void push_back(T&& value) {
        if (size_ >= capacity_) {
            resize();
        }
        data[size_++] = std::move(value);
    }
    
    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ >= capacity_) {
            resize();
        }
        new (data + size_) T(std::forward<Args>(args)...);
        ++size_;
    }
    
    void pop_back() {
        if (size_ > 0) {
            --size_;
        }
    }
    
    void insert(size_t pos, const T& value) {
        if (pos > size_) {
            throw std::out_of_range("Insert position out of range");
        }
        
        if (size_ >= capacity_) {
            resize();
        }
        
        // Shift elements to the right
        for (size_t i = size_; i > pos; --i) {
            data[i] = data[i - 1];
        }
        
        data[pos] = value;
        ++size_;
    }
    
    void erase(size_t pos) {
        if (pos >= size_) {
            throw std::out_of_range("Erase position out of range");
        }
        
        // Shift elements to the left
        for (size_t i = pos; i < size_ - 1; ++i) {
            data[i] = data[i + 1];
        }
        --size_;
    }
    
    void erase(size_t first, size_t last) {
        if (first >= size_ || last > size_ || first >= last) {
            throw std::out_of_range("Invalid erase range");
        }
        
        size_t count = last - first;
        for (size_t i = first; i < size_ - count; ++i) {
            data[i] = data[i + count];
        }
        size_ -= count;
    }
    
    // Iterators (basic implementation)
    T* begin() { return data; }
    const T* begin() const { return data; }
    T* end() { return data + size_; }
    const T* end() const { return data + size_; }
    
    // Utility methods
    void resize(size_t newSize) {
        if (newSize > capacity_) {
            reserve(newSize);
        }
        size_ = newSize;
    }
    
    void resize(size_t newSize, const T& value) {
        size_t oldSize = size_;
        resize(newSize);
        for (size_t i = oldSize; i < newSize; ++i) {
            data[i] = value;
        }
    }
    
    // Comparison operators
    bool operator==(const DynamicArray& other) const {
        if (size_ != other.size_) return false;
        for (size_t i = 0; i < size_; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
    
    bool operator!=(const DynamicArray& other) const {
        return !(*this == other);
    }
    
    // Debug and utility
    void print() const {
        std::cout << "DynamicArray[";
        for (size_t i = 0; i < size_; ++i) {
            std::cout << data[i];
            if (i < size_ - 1) std::cout << ", ";
        }
        std::cout << "] (size=" << size_ << ", capacity=" << capacity_ << ")" << std::endl;
    }
};

// Custom String Array (specialized for strings)
class StringArray {
private:
    DynamicArray<std::string> data;

public:
    StringArray() = default;
    StringArray(std::initializer_list<std::string> init) : data(init) {}
    
    void push_back(const std::string& str) { data.push_back(str); }
    void pop_back() { data.pop_back(); }
    void clear() { data.clear(); }
    
    std::string& operator[](size_t index) { return data[index]; }
    const std::string& operator[](size_t index) const { return data[index]; }
    
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
    
    std::string& front() { return data.front(); }
    const std::string& front() const { return data.front(); }
    std::string& back() { return data.back(); }
    const std::string& back() const { return data.back(); }
    
    // String-specific methods
    bool contains(const std::string& str) const {
        for (size_t i = 0; i < data.size(); ++i) {
            if (data[i] == str) return true;
        }
        return false;
    }
    
    int find(const std::string& str) const {
        for (size_t i = 0; i < data.size(); ++i) {
            if (data[i] == str) return static_cast<int>(i);
        }
        return -1;
    }
    
    void remove(const std::string& str) {
        int index = find(str);
        if (index >= 0) {
            data.erase(static_cast<size_t>(index));
        }
    }
    
    // Iterator support
    std::string* begin() { return data.begin(); }
    const std::string* begin() const { return data.begin(); }
    std::string* end() { return data.end(); }
    const std::string* end() const { return data.end(); }
};

// Custom Pair Array for storing pairs
template<typename T1, typename T2>
class PairArray {
private:
    DynamicArray<std::pair<T1, T2>> data;

public:
    PairArray() = default;
    PairArray(std::initializer_list<std::pair<T1, T2>> init) : data(init) {}
    
    void push_back(const std::pair<T1, T2>& pair) { data.push_back(pair); }
    void push_back(const T1& first, const T2& second) { data.push_back({first, second}); }
    void pop_back() { data.pop_back(); }
    void clear() { data.clear(); }
    
    std::pair<T1, T2>& operator[](size_t index) { return data[index]; }
    const std::pair<T1, T2>& operator[](size_t index) const { return data[index]; }
    
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
    
    std::pair<T1, T2>& front() { return data.front(); }
    const std::pair<T1, T2>& front() const { return data.front(); }
    std::pair<T1, T2>& back() { return data.back(); }
    const std::pair<T1, T2>& back() const { return data.back(); }
    
    // Iterator support
    std::pair<T1, T2>* begin() { return data.begin(); }
    const std::pair<T1, T2>* begin() const { return data.begin(); }
    std::pair<T1, T2>* end() { return data.end(); }
    const std::pair<T1, T2>* end() const { return data.end(); }
};

#endif // CUSTOM_CONTAINERS_HPP
