// simple CSV loader for array container
inline bool loadCSV_Array(const string& filename, DynamicArray<Item>& list) {
    ifstream file(filename);
    if (!file.is_open()) return false;
    string line;
    // try to detect header; if first line contains comma or "desc" assume header
    if (!getline(file, line)) return false;
    // If header looks like data (no comma and not quoted), still fine — we keep behavior
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