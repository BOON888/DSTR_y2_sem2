#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm> // transform, remove_if, ispunct
#include <iomanip>
#include <chrono>    // for milliseconds timing
#include <cctype>
using namespace std;

// =============================
// Dynamic Array (small copy)
// =============================
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
    DynamicArray(int cap = 10) {
        capacity = cap;
        length = 0;
        data = new T[capacity];
    }

    ~DynamicArray() {
        delete[] data;
    }

    void push_back(const T& value) {
        if (length == capacity) resize();
        data[length++] = value;
    }

    T& operator[](int index) {
        return data[index];
    }

    const T& operator[](int index) const {
        return data[index];
    }

    int size() const { return length; }
};

// =============================
// Data containers
// =============================
struct Item {
    string text; // lowercased searchable text
    string originalText; // original quoted text
};

// =============================
// Helpers: load CSV, tokenize, matching
// =============================
bool loadCSV(const string& filename, DynamicArray<Item>& list) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return false;
    }

    string line;
    getline(file, line); // skip header
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

// Tokenize: split on whitespace, remove punctuation, return array of lower-case tokens
DynamicArray<string> tokenize(const string& text) {
    DynamicArray<string> tokens;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        if (word.empty()) continue;
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        tokens.push_back(word);
    }
    return tokens;
}

bool findInArray(const DynamicArray<string>& arr, const string& target) {
    for (int i = 0; i < arr.size(); ++i) if (arr[i] == target) return true;
    return false;
}

int countMatches(const DynamicArray<string>& rwords, const DynamicArray<string>& jwords) {
    int count = 0;
    for (int i = 0; i < rwords.size(); ++i) {
        if (findInArray(jwords, rwords[i])) count++;
    }
    return count;
}

// =============================
// MAIN PROGRAM (array-based)
// =============================
int main() {
    DynamicArray<Item> resumes;
    DynamicArray<Item> jobs;

    string resumePath = "resume.csv";
    string jobPath = "job_description.csv";

    if (!loadCSV(resumePath, resumes)) {
        cout << "Cannot open resume.csv. Please check the file path.\n";
        return 0;
    }
    if (!loadCSV(jobPath, jobs)) {
        cout << "Cannot open job_description.csv. Please check the file path.\n";
        return 0;
    }

    cout << "Loaded " << resumes.size() << " resumes." << endl;
    cout << "Loaded " << jobs.size() << " jobs." << endl;

    string skill;
    cout << "\nEnter skill to search: ";
    getline(cin, skill);
    transform(skill.begin(), skill.end(), skill.begin(), ::tolower);

    cout << "\n===============================" << endl;
    cout << "STAGE 1: FILTER BY SKILL (" << skill << ")" << endl;
    cout << "===============================" << endl;

    DynamicArray<int> skillResumesIndices;
    for (int i = 0; i < resumes.size(); ++i) {
        if (resumes[i].text.find(skill) != string::npos) {
            skillResumesIndices.push_back(i);
            cout << "Resume " << (i+1) << ": " << resumes[i].originalText << endl;
        }
    }

    cout << "\nTotal resumes found with skill '" << skill << "': " << skillResumesIndices.size() << endl;

    if (skillResumesIndices.size() == 0) {
        cout << "No resumes contain this skill. Exiting program.\n";
        return 0;
    }

    cout << "\n=========================================" << endl;
    cout << "STAGE 1 SUMMARY" << endl;
    cout << "=========================================" << endl;
    cout << "Skill searched: " << skill << endl;
    cout << "Matching resumes found: " << skillResumesIndices.size() << endl;
    cout << "-----------------------------------------\n";

    int chosenIndex = 0;
    cout << "Enter resume number to match with jobs (0 to exit): ";
    if (!(cin >> chosenIndex)) return 0;
    if (chosenIndex == 0) { cout << "Exiting program.\n"; return 0; }

    // validate chosenIndex exists in skillResumesIndices
    bool validChoice = false;
    for (int k = 0; k < skillResumesIndices.size(); ++k) {
        if (skillResumesIndices[k] == chosenIndex - 1) { validChoice = true; break; }
    }
    if (!validChoice) {
        cout << "Invalid choice. Please select one of the resumes listed above.\n";
        return 0;
    }

    double matchThreshold;
    cout << "Enter percentage threshold for job matching: ";
    if (!(cin >> matchThreshold)) return 0;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n===============================" << endl;
    cout << "STAGE 2: JOB MATCHING FOR RESUME " << chosenIndex << endl;
    cout << "===============================" << endl;

    auto start = chrono::high_resolution_clock::now();

    int idx = chosenIndex - 1;
    cout << "Resume " << chosenIndex << ": " << resumes[idx].originalText << endl;

    DynamicArray<string> rwords = tokenize(resumes[idx].text);
    bool foundQualified = false;
    int qualifiedCount = 0;

    for (int j = 0; j < jobs.size(); ++j) {
        DynamicArray<string> jwords = tokenize(jobs[j].text);
        int matches = countMatches(rwords, jwords);
        double percent = (rwords.size() == 0) ? 0.0 : ((double)matches / (double)rwords.size()) * 100.0;
        if (percent >= matchThreshold) {
            if (!foundQualified) cout << "\nJobs matching ≥ " << matchThreshold << "%:\n";
            foundQualified = true;
            qualifiedCount++;
            cout << "   - Job " << (j+1) << " (" << fixed << setprecision(2) << percent << "% )\n";
            cout << "     " << jobs[j].originalText << "\n";
        }
    }

    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "\n=========================================\n";
    cout << "STAGE 2 SUMMARY (RESUME " << chosenIndex << ")\n";
    cout << "=========================================\n";
    cout << "Total jobs checked: " << jobs.size() << endl;
    cout << "Jobs " << matchThreshold << "% and above match: " << qualifiedCount << endl;
    if (qualifiedCount == 0) cout << "This resume did not qualify for any job.\n";
    cout << "Time Taken: " << elapsed << " milliseconds\n";

    return 0;
}
