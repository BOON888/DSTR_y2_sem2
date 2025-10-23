#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <limits>
#include <windows.h>
#include <psapi.h>
using namespace std;
using namespace std::chrono;


// DynamicArray Template

template <typename T>
class DynamicArray {
private:
    T* data;
    int capacity;
    int length;

    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (int i = 0; i < length; ++i)
            newData[i] = data[i];
        delete[] data;
        data = newData;
    }

public:
    DynamicArray(int cap = 16) {
        capacity = cap;
        length = 0;
        data = new T[capacity];
    }

    ~DynamicArray() { delete[] data; }

    void push_back(const T& value) {
        if (length == capacity) resize();
        data[length++] = value;
    }

    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }

    int size() const { return length; }

    void clear() { length = 0; }
};

// Linked list templates (header-only)

template <typename T>
struct Node {
    T data;
    Node* next;
    Node(const T& val) : data(val), next(nullptr) {}
};

template <typename T>
class LinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int length;

public:
    LinkedList() : head(nullptr), tail(nullptr), length(0) {}
    ~LinkedList() {
        Node<T>* curr = head;
        while (curr) {
            Node<T>* tmp = curr;
            curr = curr->next;
            delete tmp;
        }
    }

    void push_back(const T& val) {
        Node<T>* newNode = new Node<T>(val);
        if (!head) head = tail = newNode;
        else {
            tail->next = newNode;
            tail = newNode;
        }
        length++;
    }

    Node<T>* getHead() const { return head; }
    int size() const { return length; }
};


// Shared structures and helpers

struct Item {
    string text;
    string originalText; 
};

inline string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Tokenize (alphanumeric tokens, lowercased)
inline DynamicArray<string> tokenizeLower(const string& text) {
    DynamicArray<string> toks;
    string cur;
    for (char ch : text) {
        if (isalnum((unsigned char)ch)) cur.push_back(ch);
        else if (!cur.empty()) {
            transform(cur.begin(), cur.end(), cur.begin(), ::tolower);
            toks.push_back(cur);
            cur.clear();
        }
    }
    if (!cur.empty()) {
        transform(cur.begin(), cur.end(), cur.begin(), ::tolower);
        toks.push_back(cur);
    }
    return toks;
}

// simple CSV loader for array container
inline bool loadCSV_Array(const string& filename, DynamicArray<Item>& list) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    string line;
    
    if (!getline(file, line)) return false;
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        string raw = line;
        if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"')
            raw = raw.substr(1, raw.size() - 2);
        string lower = raw;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        list.push_back({lower, raw});
    }
    file.close();
    return true;
}

// CSV loader for linked list
inline bool loadCSV_Linked(const string& filename, LinkedList<Item>& list) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    string line;
    if (!getline(file, line)) return false;
    while (getline(file, line)) {
        if (line.empty()) continue;
        string raw = line;
        if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"')
            raw = raw.substr(1, raw.size() - 2);
        string lower = raw;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        list.push_back({lower, raw});
    }
    file.close();
    return true;
}

// match count (uses tokenizeLower)
inline int countMatches(const DynamicArray<string>& rwords, const DynamicArray<string>& jwords) {
    int cnt = 0;
    for (int i = 0; i < rwords.size(); ++i) {
        for (int j = 0; j < jwords.size(); ++j) {
            if (rwords[i] == jwords[j]) { cnt++; break; }
        }
    }
    return cnt;
}

inline size_t getMemoryUsageKB() {
    PROCESS_MEMORY_COUNTERS memInfo;
    GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo));
    return memInfo.WorkingSetSize / 1024; // returns memory in KB
}

#endif
