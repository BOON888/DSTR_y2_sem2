#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <set>
using namespace std;

// ==========================
// Utility: Clean text (remove punctuation, lowercase)
// ==========================
string cleanText(string text) {
    string result;
    for (char c : text) {
        if (isalpha(c) || isdigit(c) || isspace(c)) {
            result += tolower(c);
        }
    }
    return result;
}

// ==========================
// Structures for Job & Resume
// ==========================
struct Job {
    string jobId;
    string description;
};

struct Resume {
    string resumeId;
    string text;
};

// ==========================
// Generic Linked List
// ==========================
template <typename T>
struct Node {
    T data;
    Node* next;
    Node(T value) : data(value), next(nullptr) {}
};

template <typename T>
class LinkedList {
private:
    Node<T>* head;
    int count;
public:
    LinkedList() : head(nullptr), count(0) {}

    ~LinkedList() {
        Node<T>* current = head;
        while (current) {
            Node<T>* nextNode = current->next;
            delete current;
            current = nextNode;
        }
    }

    void append(T value) {
        Node<T>* newNode = new Node<T>(value);
        if (!head) {
            head = newNode;
        } else {
            Node<T>* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
        count++;
    }

    Node<T>* getHead() const { return head; }
    int size() const { return count; }

    void display(); // specialized below
};

// Specialization: Job display
template<>
void LinkedList<Job>::display() {
    cout << "--- Displaying Job Postings ---" << endl;
    Node<Job>* temp = getHead();
    int displayCount = 0;
    while (temp && displayCount < 5) {
        cout << "Job ID: " << temp->data.jobId 
             << "\nDescription: " << temp->data.description.substr(0, 70) << "..." << endl;
        cout << "--------------------------------" << endl;
        temp = temp->next;
        displayCount++;
    }
    cout << "Total jobs loaded: " << size() << endl;
}

// Specialization: Resume display
template<>
void LinkedList<Resume>::display() {
    cout << "\n--- Displaying Resumes ---" << endl;
    Node<Resume>* temp = getHead();
    int displayCount = 0;
    while (temp && displayCount < 5) {
        cout << "Resume ID: " << temp->data.resumeId 
             << "\nText: " << temp->data.text.substr(0, 70) << "..." << endl;
        cout << "--------------------------------" << endl;
        temp = temp->next;
        displayCount++;
    }
    cout << "Total resumes loaded: " << size() << endl;
}

// ==========================
// CSV Loaders
// ==========================
void loadJobsFromCSV(const string& filename, LinkedList<Job>& jobList) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    string line;
    getline(file, line); // skip header if exists

    int jobCounter = 1;
    while (getline(file, line)) {
        if (!line.empty()) {
            string jobId = "J" + to_string(jobCounter++);
            Job newJob = {jobId, line};
            jobList.append(newJob);
        }
    }
    file.close();
    cout << "Successfully loaded data from " << filename << endl;
}

void loadResumesFromCSV(const string& filename, LinkedList<Resume>& resumeList) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    string line;
    getline(file, line); // skip header if exists

    int resumeCounter = 1;
    while (getline(file, line)) {
        if (!line.empty()) {
            string resumeId = "R" + to_string(resumeCounter++);
            Resume newResume = {resumeId, line};
            resumeList.append(newResume);
        }
    }
    file.close();
    cout << "Successfully loaded data from " << filename << endl;
}

// ==========================
// Matching Algorithm
// ==========================
int matchScore(string job, string resume) {
    job = cleanText(job);
    resume = cleanText(resume);

    istringstream j(job), r(resume);
    set<string> jobWords, resumeWords;
    string word;

    while (j >> word) jobWords.insert(word);
    while (r >> word) resumeWords.insert(word);

    int score = 0;
    for (auto &w : jobWords) {
        if (resumeWords.find(w) != resumeWords.end()) score++;
    }
    return score;
}

void matchJobs(LinkedList<Job>& jobs, LinkedList<Resume>& resumes) {
    Node<Job>* jobNode = jobs.getHead();
    while (jobNode) {
        Node<Resume>* resNode = resumes.getHead();
        int bestScore = -1;
        Resume bestMatch;

        while (resNode) {
            int score = matchScore(jobNode->data.description, resNode->data.text);
            if (score > bestScore) {
                bestScore = score;
                bestMatch = resNode->data;
            }
            resNode = resNode->next;
        }

        cout << "\nJob ID: " << jobNode->data.jobId
             << "\nDescription: " << jobNode->data.description.substr(0, 60) << "..."
             << "\nBest Match Resume: " << bestMatch.resumeId
             << "\nScore: " << bestScore << endl;
        cout << "--------------------------------" << endl;

        jobNode = jobNode->next;
    }
}

// ==========================
// Main Program
// ==========================
int main() {
    LinkedList<Job> jobList;
    LinkedList<Resume> resumeList;

    loadJobsFromCSV("job_description.csv", jobList);
    loadResumesFromCSV("resume.csv", resumeList);

    cout << "\nData loading complete. Verifying contents..." << endl;
    cout << "============================================" << endl;

    jobList.display();
    resumeList.display();

    cout << "\n=== Job Matching Results ===" << endl;
    matchJobs(jobList, resumeList);

    return 0;
}
