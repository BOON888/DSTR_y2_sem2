#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <limits>
using namespace std;
using namespace std::chrono;

// ======================= Dynamic Array =======================
template<typename T>
class DynamicArray {
private:
    T* data;
    int capacity;
    int length;

    void grow() {
        int newCap = capacity * 2;
        T* nd = new T[newCap];
        for (int i = 0; i < length; ++i) nd[i] = data[i];
        delete[] data;
        data = nd;
        capacity = newCap;
    }

public:
    DynamicArray(int cap = 16) : capacity(cap), length(0) {
        data = new T[capacity];
    }
    ~DynamicArray() { delete[] data; }

    void push_back(const T& v) {
        if (length == capacity) grow();
        data[length++] = v;
    }

    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }

    int size() const { return length; }

    void clear() {
        delete[] data;
        data = new T[capacity];
        length = 0;
    }
};

// ======================= Helper Structures =======================
struct Result {
    string text;
    double score;
};

// ======================= Utility Functions =======================
string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// stopword list
const string ignoreWordsArr[] = {
    "in","with","and","the","to","for","needed","required","experience",
    "skills","of","a","an","as","on","by","at","have","has","is","are",
    "job","role","responsibilities","looking","must","be","work"
};
const int ignoreWordCount = sizeof(ignoreWordsArr) / sizeof(ignoreWordsArr[0]);

bool isIgnoredWord(const string& w) {
    for (int i = 0; i < ignoreWordCount; ++i)
        if (ignoreWordsArr[i] == w) return true;
    return false;
}

// tokenize words
DynamicArray<string> tokenizeLower(const string& text) {
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

// calculate match score
double calculateMatchPercentage(const string& jobDesc, const string& resumeDesc) {
    DynamicArray<string> jobWords = tokenizeLower(jobDesc);
    DynamicArray<string> resumeWords = tokenizeLower(resumeDesc);

    DynamicArray<string> jobSkills;
    for (int i = 0; i < jobWords.size(); ++i)
        if (!isIgnoredWord(jobWords[i])) jobSkills.push_back(jobWords[i]);

    if (jobSkills.size() == 0) return 0.0;

    int matched = 0;
    for (int i = 0; i < jobSkills.size(); ++i) {
        for (int j = 0; j < resumeWords.size(); ++j) {
            if (jobSkills[i] == resumeWords[j]) {
                matched++;
                break;
            }
        }
    }
    return (matched * 100.0) / jobSkills.size();
}

// load lines from file
bool loadLines(const string& filename, DynamicArray<string>& out) {
    ifstream f(filename);
    if (!f.is_open()) return false;
    string line;
    while (getline(f, line)) {
        if (!line.empty())
            out.push_back(line);
    }
    f.close();
    return true;
}

// sort results by score (descending)
void sortResultsByScore(DynamicArray<Result>& arr) {
    for (int i = 0; i < arr.size() - 1; ++i) {
        for (int j = 0; j < arr.size() - i - 1; ++j) {
            if (arr[j].score < arr[j + 1].score) {
                Result temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// safely get integer input
int getIntInput(const string& prompt, int minVal, int maxVal) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val && val >= minVal && val <= maxVal) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        } else {
            cout << "❌ Invalid input. Please enter a number between "
                 << minVal << " and " << maxVal << ".\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// safely get double input
double getDoubleInput(const string& prompt, double minVal, double maxVal) {
    double val;
    while (true) {
        cout << prompt;
        if (cin >> val && val >= minVal && val <= maxVal) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        } else {
            cout << "❌ Invalid input. Please enter a number between "
                 << minVal << " and " << maxVal << ".\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// ======================= MAIN PROGRAM =======================
int main() {
    cout << "=== JOB MATCHING SYSTEM (ARRAY-BASED) ===\n\n";

    DynamicArray<string> jobs;
    DynamicArray<string> resumes;

    // Load files
    if (!loadLines("job_description.csv", jobs)) {
        cout << "❌ Error opening job_description.csv\n";
        return 1;
    }
    if (!loadLines("resume.csv", resumes)) {
        cout << "❌ Error opening resume.csv\n";
        return 1;
    }

    cout << "✅ Loaded " << jobs.size() << " job descriptions.\n";
    cout << "✅ Loaded " << resumes.size() << " resumes.\n\n";

    string keyword;
    cout << "Enter a skill keyword to search (example: sql): ";
    getline(cin, keyword);
    keyword = toLowerCase(keyword);

    DynamicArray<int> matchedJobIdx;
    for (int i = 0; i < jobs.size(); ++i) {
        string low = toLowerCase(jobs[i]);
        if (low.find(keyword) != string::npos)
            matchedJobIdx.push_back(i);
    }

    if (matchedJobIdx.size() == 0) {
        cout << "\n❌ No job descriptions found with that skill.\n";
        return 0;
    }

    cout << "\n=== JOBS FOUND WITH '" << keyword << "' ===\n";
    int showLimit = (matchedJobIdx.size() < 20) ? matchedJobIdx.size() : 20;
    for (int i = 0; i < showLimit; ++i)
        cout << setw(3) << i + 1 << ". " << jobs[matchedJobIdx[i]] << endl;

    if (matchedJobIdx.size() > 20) {
        cout << "\nMore than 20 jobs found. Showing first 20.\n";
    }

    // Get job choice safely
    int jobChoice = getIntInput("\nEnter the job number you want to analyze: ", 1, matchedJobIdx.size());
    int selectedJobIdx = matchedJobIdx[jobChoice - 1];

    // Get minimum percentage safely
    double minPercentage = getDoubleInput("Enter minimum match percentage (e.g., 50): ", 0.0, 100.0);

    cout << "\n=== SELECTED JOB DESCRIPTION ===\n"
         << jobs[selectedJobIdx] << "\n\n";

    cout << "--- Resume Match Results ---\n";

    auto start = high_resolution_clock::now();
    DynamicArray<Result> resumeResults;

    for (int i = 0; i < resumes.size(); ++i) {
        double score = calculateMatchPercentage(jobs[selectedJobIdx], resumes[i]);
        if (score >= minPercentage && score > 0.0) {
            Result r{resumes[i], score};
            resumeResults.push_back(r);
        }
    }

    sortResultsByScore(resumeResults); // sort highest match first

    int resumeLimit = (resumeResults.size() < 20) ? resumeResults.size() : 20;
    for (int i = 0; i < resumeLimit; ++i) {
        cout << setw(3) << i + 1 << ". " << resumeResults[i].text << endl;
        cout << "     Match Score: " << fixed << setprecision(2)
             << resumeResults[i].score << "%\n\n";
    }

    if (resumeResults.size() > 20) {
        cout << "More than 20 resumes matched. Showing top 20.\n";
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    cout << "=== SUMMARY ===\n";
    cout << "Total resumes matched: " << resumeResults.size() << endl;
    cout << "Matching completed in " << duration.count() << " ms.\n";
    cout << "Estimated memory used: "
         << ((jobs.size() + resumes.size()) * sizeof(string)) / 1024
         << " KB (approx.)\n";

    cout << "\nThank you for using the Array-Based Job Matching System!\n";
    return 0;
}
