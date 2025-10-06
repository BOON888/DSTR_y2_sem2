#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <chrono>
#include <algorithm>
#include <vector>
#include <direct.h>
using namespace std;

// =============================
// 1. Dynamic Array Template
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

    int size() const {
        return length;
    }
};

// =============================
// 2. Structs for Job & Resume
// =============================
struct Job {
    string id;
    string description;
};

struct Resume {
    string id;
    string description;
};

// =============================
// 3. Helper: Clean and Tokenize
// =============================
DynamicArray<string> tokenize(const string& text) {
    DynamicArray<string> tokens;
    string word = "";
    for (char c : text) {
        if (isalnum((unsigned char)c)) {
            word += tolower((unsigned char)c);
        } else if (!word.empty()) {
            tokens.push_back(word);
            word = "";
        }
    }
    if (!word.empty()) tokens.push_back(word);
    return tokens;
}

// Helper: trim whitespace from both ends of a string
static void trimString(string &s) {
    const string whitespace = " \t\n\r";
    size_t start = s.find_first_not_of(whitespace);
    size_t end = s.find_last_not_of(whitespace);
    if (start == string::npos) { s.clear(); return; }
    s = s.substr(start, end - start + 1);
}

// Helper: remove surrounding double quotes if present (after trimming)
static void stripQuotesString(string &s) {
    trimString(s);
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        s = s.substr(1, s.size() - 2);
    }
}

// =============================
// 4. Keyword Matching Algorithm
// =============================
int calculateScore(const string& jobDesc, const string& resumeDesc) {
    DynamicArray<string> jobWords = tokenize(jobDesc);
    DynamicArray<string> resumeWords = tokenize(resumeDesc);

    int score = 0;
    for (int i = 0; i < jobWords.size(); ++i) {
        for (int j = 0; j < resumeWords.size(); ++j) {
            if (jobWords[i] == resumeWords[j]) {
                score++;
                break; // avoid double counting
            }
        }
    }
    return score;
}

// =============================
// 5. Read CSV Files
// =============================
void readCSV(const string& filename, DynamicArray<Job>& jobs, DynamicArray<Resume>& resumes) {
    // Try opening the file from several likely locations (cwd, parent, ArrayList, etc.)
    vector<string> candidates;
    candidates.push_back(filename);
    candidates.push_back(string("..\\") + filename);
    candidates.push_back(string("../") + filename);
    candidates.push_back(string("ArrayList\\") + filename);
    candidates.push_back(string(".\\") + filename);

    ifstream file;
    string openedPath;
    for (const auto &p : candidates) {
        file.open(p);
        if (file.is_open()) { openedPath = p; break; }
    }
    if (!file.is_open()) {
        // Print helpful diagnostics
        // getcwd is available via _getcwd on Windows (direct.h)
        char cwdBuf[FILENAME_MAX];
        if (_getcwd(cwdBuf, FILENAME_MAX) == nullptr) cwdBuf[0] = '\0';
        cerr << "Error opening file: " << filename << endl;
        cerr << "Current working directory: " << cwdBuf << endl;
        cerr << "Attempted paths:" << endl;
        for (const auto &p : candidates) cerr << "  " << p << endl;
        return;
    }
    if (openedPath != filename) {
        cout << "[INFO] Opened '" << filename << "' via fallback path: " << openedPath << endl;
    }
    // use helper functions defined above

    string line;
    int lineNo = 0;
    while (getline(file, line)) {
        lineNo++;
        if (line.empty()) continue;

        string id, desc;
    string tmp = line;
    trimString(tmp);

        // Skip simple header lines like: job_description  or resume
        string lowered = tmp;
        transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);
        if (lowered == "job_description" || lowered == "resume") continue;

        // If the whole line is quoted (single-column CSV where description contains commas)
        if (tmp.front() == '"' && tmp.back() == '"') {
            desc = tmp.substr(1, tmp.size() - 2);
            // generate an id
            if (filename.find("job") != string::npos)
                id = string("job") + to_string(lineNo);
            else
                id = string("res") + to_string(lineNo);
        } else {
            // Attempt to split at first comma (id,desc). If no comma, treat whole line as description.
            size_t pos = tmp.find(',');
            if (pos == string::npos) {
                desc = tmp;
                if (filename.find("job") != string::npos)
                    id = string("job") + to_string(lineNo);
                else
                    id = string("res") + to_string(lineNo);
            } else {
                id = tmp.substr(0, pos);
                desc = tmp.substr(pos + 1);
                stripQuotesString(id);
                stripQuotesString(desc);
            }
        }

        stripQuotesString(id);
        stripQuotesString(desc);

        if (filename.find("job") != string::npos)
            jobs.push_back({id, desc});
        else
            resumes.push_back({id, desc});
    }
    file.close();
}

// =============================
// 6. Find Best Match
// =============================
void findBestMatches(DynamicArray<Job>& jobs, DynamicArray<Resume>& resumes) {
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < jobs.size(); ++i) {
        int bestScore = -1;
        string bestResumeId = "";

        for (int j = 0; j < resumes.size(); ++j) {
            int score = calculateScore(jobs[i].description, resumes[j].description);
            if (score > bestScore) {
                bestScore = score;
                bestResumeId = resumes[j].id;
            }
        }

        cout << "Job ID: " << jobs[i].id << endl;
        cout << "Description: \"" << jobs[i].description.substr(0, 60)
             << (jobs[i].description.size() > 60 ? "..." : "") << "\"" << endl;
        cout << "Best Match Resume: " << bestResumeId << endl;
        cout << "Score: " << bestScore << endl;
        cout << "--------------------------------------" << endl;
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "\n[INFO] Processing time: " << duration << " ms" << endl;
}

// =============================
// 7. Main Function
// =============================
int main() {
    DynamicArray<Job> jobs;
    DynamicArray<Resume> resumes;

    // Allow optional command-line args in future; default filenames used here
    string jobFile = "job_description.csv";
    string resFile = "resume.csv";

    readCSV(jobFile, jobs, resumes);
    readCSV(resFile, jobs, resumes); // resumes go into same arrays based on filename check

    cout << "=== ARRAY-BASED JOB MATCHING SYSTEM ===\n" << endl;
    findBestMatches(jobs, resumes);

    return 0;
}
