#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>   // for milliseconds timing
using namespace std;

// ====== SETTINGS ======
const int RESUME_LIMIT = 10000;
const int JOB_LIMIT    = 10000;
// ======================

// Linked List Node
struct Node {
    string text;
    string originalText;
    Node* next;
    Node(string t, string o) : text(t), originalText(o), next(nullptr) {}
};

// Linked List
class LinkedList {
private:
    Node* head;
public:
    LinkedList() : head(nullptr) {}

    void insert(string value) {
        string lower = value;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        Node* newNode = new Node(lower, value);
        if (!head) { head = newNode; return; }
        Node* temp = head;
        while (temp->next) temp = temp->next;
        temp->next = newNode;
    }

    vector<string> toVectorLower() {
        vector<string> vec;
        Node* temp = head;
        while (temp) {
            vec.push_back(temp->text);
            temp = temp->next;
        }
        return vec;
    }

    vector<string> toVectorOriginal() {
        vector<string> vec;
        Node* temp = head;
        while (temp) {
            vec.push_back(temp->originalText);
            temp = temp->next;
        }
        return vec;
    }
};

// Load CSV safely
bool loadCSV(string filename, LinkedList &list) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return false;
    }

    string line;
    getline(file, line); // skip header
    while (getline(file, line)) {
        if (!line.empty()) {
            if (line.front() == '"' && line.back() == '"')
                line = line.substr(1, line.size() - 2);
            list.insert(line);
        }
    }
    file.close();
    return true;
}

// Tokenize text
vector<string> tokenize(string text) {
    vector<string> tokens;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        if (!word.empty())
            tokens.push_back(word);
    }
    return tokens;
}

// Count overlapping words
int countMatches(const vector<string> &rwords, const vector<string> &jwords) {
    int count = 0;
    for (const string &rw : rwords) {
        if (find(jwords.begin(), jwords.end(), rw) != jwords.end())
            count++;
    }
    return count;
}

// MAIN PROGRAM
int main() {
    LinkedList resumes, jobs;
    string resumePath = "resume.csv";
    string jobPath = "job_description.csv";

    // Load datasets
    if (!loadCSV(resumePath, resumes)) {
        cout << "Cannot open resume.csv. Please check the file path.\n";
        return 0;
    }
    if (!loadCSV(jobPath, jobs)) {
        cout << "Cannot open job_description.csv. Please check the file path.\n";
        return 0;
    }

    vector<string> resumeListLower = resumes.toVectorLower();
    vector<string> resumeListOriginal = resumes.toVectorOriginal();
    vector<string> jobListLower = jobs.toVectorLower();
    vector<string> jobListOriginal = jobs.toVectorOriginal();

    cout << "Loaded " << resumeListLower.size() << " resumes." << endl;
    cout << "Loaded " << jobListLower.size() << " jobs." << endl;

    string skill;
    cout << "\nEnter skill to search: ";
    getline(cin, skill);
    transform(skill.begin(), skill.end(), skill.begin(), ::tolower);

    cout << "\n===============================" << endl;
    cout << "STAGE 1: FILTER BY SKILL (" << skill << ")" << endl;
    cout << "===============================" << endl;

    vector<int> skillResumes;
    for (int i = 0; i < (int)resumeListLower.size(); i++) {
        if (resumeListLower[i].find(skill) != string::npos) {
            skillResumes.push_back(i);
            cout << "Resume " << i + 1 << ": " << resumeListOriginal[i] << endl;
        }
    }

    cout << "\nTotal resumes found with skill '" << skill << "': " << skillResumes.size() << endl;

    if (skillResumes.empty()) {
        cout << "No resumes contain this skill. Exiting program.\n";
        return 0;
    }

    cout << "\n=========================================\n";
    cout << "STAGE 1 SUMMARY\n";
    cout << "=========================================\n";
    cout << "Skill searched: " << skill << endl;
    cout << "Matching resumes found: " << skillResumes.size() << endl;
    cout << "-----------------------------------------\n";

    int chosenIndex;
    cout << "Enter resume number to match with jobs (0 to exit): ";
    cin >> chosenIndex;
    if (chosenIndex == 0) {
        cout << "Exiting program.\n";
        return 0;
    }

    if (find(skillResumes.begin(), skillResumes.end(), chosenIndex - 1) == skillResumes.end()) {
        cout << "Invalid choice. Please select one of the resumes listed above.\n";
        return 0;
    }

    double matchThreshold;
    cout << "Enter percentage threshold for job matching: ";
    cin >> matchThreshold;

    cout << "\n===============================" << endl;
    cout << "STAGE 2: JOB MATCHING FOR RESUME " << chosenIndex << endl;
    cout << "===============================" << endl;

    auto start = chrono::high_resolution_clock::now();

    int idx = chosenIndex - 1;
    cout << "Resume " << chosenIndex << ": " << resumeListOriginal[idx] << endl;

    vector<string> rwords = tokenize(resumeListLower[idx]);
    bool foundQualified = false;
    int qualifiedCount = 0;

    for (int j = 0; j < (int)jobListLower.size(); j++) {
        vector<string> jwords = tokenize(jobListLower[j]);
        int matches = countMatches(rwords, jwords);
        double percent = ((double)matches / rwords.size()) * 100.0;

        if (percent >= matchThreshold) {
            if (!foundQualified)
                cout << "\nJobs matching ≥ " << matchThreshold << "%:\n";
            foundQualified = true;
            qualifiedCount++;
            cout << "   - Job " << j + 1 << " (" << fixed << setprecision(2)
                 << percent << "%)\n";
            cout << "     " << jobListOriginal[j] << endl;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "\n=========================================\n";
    cout << "STAGE 2 SUMMARY (RESUME " << chosenIndex << ")\n";
    cout << "=========================================\n";
    cout << "Total jobs checked: " << jobListLower.size() << endl;
    cout << "Jobs " << matchThreshold << "% and above match: " << qualifiedCount << endl;
    if (qualifiedCount == 0)
        cout << "This resume did not qualify for any job.\n";
    cout << "Time Taken: " << elapsed << " milliseconds\n";

    return 0;
}
