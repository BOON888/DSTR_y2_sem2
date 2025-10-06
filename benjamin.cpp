#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
using namespace std;
using namespace std::chrono;

// ---------- Linked List Node Structures ----------
struct JobNode {
    string description;
    JobNode* next;
};

struct ResumeNode {
    string description;
    ResumeNode* next;
};

// ---------- Utility Functions ----------
string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

vector<string> splitWords(const string& text) {
    vector<string> words;
    string word;
    for (char c : text) {
        if (isalnum(c))
            word += c;
        else if (!word.empty()) {
            words.push_back(word);
            word.clear();
        }
    }
    if (!word.empty()) words.push_back(word);
    return words;
}

bool containsKeyword(const string& text, const string& keyword) {
    return text.find(keyword) != string::npos;
}

// Calculate match percentage between job and resume
double calculateMatchPercentage(const string& jobDesc, const string& resumeDesc) {
    vector<string> jobWords = splitWords(toLowerCase(jobDesc));
    vector<string> resumeWords = splitWords(toLowerCase(resumeDesc));

    // Ignore filler words
    vector<string> ignoreList = {"in","with","and","the","to","for","needed","required","experience",
                                 "skills","of","a","an","as","on","by","at","have","has","is","are",
                                 "job","role","responsibilities","looking","must","be","work"};

    auto isIgnored = [&](const string& word) {
        return find(ignoreList.begin(), ignoreList.end(), word) != ignoreList.end();
    };

    vector<string> jobSkills;
    for (auto& w : jobWords)
        if (!isIgnored(w)) jobSkills.push_back(w);

    if (jobSkills.empty()) return 0.0;

    int matched = 0;
    for (const string& jw : jobSkills) {
        for (const string& rw : resumeWords) {
            if (jw == rw) {
                matched++;
                break;
            }
        }
    }

    return (matched * 100.0) / jobSkills.size();
}

// ---------- Load CSV Data ----------
JobNode* loadJobs(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening job file: " << filename << endl;
        return nullptr;
    }

    JobNode* head = nullptr;
    JobNode* tail = nullptr;
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;
        JobNode* newNode = new JobNode{line, nullptr};
        if (!head)
            head = tail = newNode;
        else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    file.close();
    return head;
}

ResumeNode* loadResumes(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening resume file: " << filename << endl;
        return nullptr;
    }

    ResumeNode* head = nullptr;
    ResumeNode* tail = nullptr;
    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;
        ResumeNode* newNode = new ResumeNode{line, nullptr};
        if (!head)
            head = tail = newNode;
        else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    file.close();
    return head;
}

// ---------- Main Program ----------
int main() {
    cout << "=== Job Matching System (Linked List Based) ===" << endl;

    JobNode* jobHead = loadJobs("job_description.csv");
    ResumeNode* resumeHead = loadResumes("resume.csv");

    if (!jobHead || !resumeHead) {
        cout << "Error loading CSV files." << endl;
        return 1;
    }

    string keyword;
    cout << "\nEnter a skill keyword to search (example: sql): ";
    cin >> keyword;
    keyword = toLowerCase(keyword);

    vector<JobNode*> matchedJobs;
    JobNode* tempJob = jobHead;

    // Search for matching jobs
    while (tempJob) {
        if (containsKeyword(toLowerCase(tempJob->description), keyword)) {
            matchedJobs.push_back(tempJob);
        }
        tempJob = tempJob->next;
    }

    if (matchedJobs.empty()) {
        cout << "\nNo job descriptions found with that skill." << endl;
        return 0;
    }

    cout << "\nJobs found with \"" << keyword << "\":\n" << endl;

    int showLimit = min(20, (int)matchedJobs.size());
    for (int i = 0; i < showLimit; i++) {
        cout << i + 1 << ". " << matchedJobs[i]->description << endl;
    }

    if ((int)matchedJobs.size() > 20) {
        char choice;
        cout << "\nMore than 20 jobs found. Do you want to see all? (y/n): ";
        cin >> choice;

        if (tolower(choice) == 'y') {
            for (int i = 20; i < (int)matchedJobs.size(); i++) {
                cout << i + 1 << ". " << matchedJobs[i]->description << endl;
            }
        }
    }

    cout << "\nTotal jobs found: " << matchedJobs.size() << endl;

    int jobChoice;
    cout << "\nEnter the job number you want to analyze: ";
    cin >> jobChoice;

    if (jobChoice < 1 || jobChoice > (int)matchedJobs.size()) {
        cout << "Invalid job number!" << endl;
        return 0;
    }

    double minPercentage;
    cout << "Enter the minimum percentage to display resumes (example: 50): ";
    cin >> minPercentage;

    JobNode* selectedJob = matchedJobs[jobChoice - 1];
    cout << "\nSelected Job Description:\n" << selectedJob->description << endl;

    cout << "\n--- Resume Match Results ---" << endl;

    // Start timing
    auto startTime = high_resolution_clock::now();

    ResumeNode* tempResume = resumeHead;
    vector<pair<string, double>> resumeResults;
    int resumeIndex = 1;

    while (tempResume) {
        double score = calculateMatchPercentage(selectedJob->description, tempResume->description);
        if (score >= minPercentage && score > 0.0) {
            resumeResults.push_back({tempResume->description, score});
        }
        tempResume = tempResume->next;
        resumeIndex++;
    }

    // Show only first 20 resumes
    int resumeLimit = min(20, (int)resumeResults.size());
    for (int i = 0; i < resumeLimit; i++) {
        cout << i + 1 << ". " << resumeResults[i].first << endl;
        cout << "   Score: " << resumeResults[i].second << "%" << endl << endl;
    }

    if ((int)resumeResults.size() > 20) {
        char choice;
        cout << "More than 20 resumes found. Do you want to see more resumes? (y/n): ";
        cin >> choice;

        if (tolower(choice) == 'y') {
            for (int i = 20; i < (int)resumeResults.size(); i++) {
                cout << i + 1 << ". " << resumeResults[i].first << endl;
                cout << "   Score: " << resumeResults[i].second << "%" << endl << endl;
            }
        }
    }

    // Stop timing
    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(endTime - startTime);

    cout << "Total resumes matched: " << resumeResults.size() << endl;
    cout << "Matching complete." << endl;
    cout << "Total time used for parsing resumes: " << duration.count() << " ms" << endl;

    // Free memory
    while (jobHead) {
        JobNode* temp = jobHead;
        jobHead = jobHead->next;
        delete temp;
    }
    while (resumeHead) {
        ResumeNode* temp = resumeHead;
        resumeHead = resumeHead->next;
        delete temp;
    }

    return 0;
}
