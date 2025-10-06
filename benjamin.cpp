#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
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

// ---------- Custom Container Templates ----------
template<typename T>
struct MyNode {
    T data;
    MyNode* next;
};

template<typename T>
class MyList {
private:
    MyNode<T>* head;
    MyNode<T>* tail;
    int count;

public:
    MyList() : head(nullptr), tail(nullptr), count(0) {}

    void push_back(const T& value) {
        MyNode<T>* newNode = new MyNode<T>{value, nullptr};
        if (!head)
            head = tail = newNode;
        else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }

    T& at(int index) {
        MyNode<T>* temp = head;
        for (int i = 0; i < index && temp; i++) temp = temp->next;
        return temp->data;
    }

    int size() const { return count; }

    void clear() {
        while (head) {
            MyNode<T>* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        count = 0;
    }

    ~MyList() { clear(); }

    // Iterator-like traversal
    MyNode<T>* getHead() const { return head; }
};

// ---------- Utility Functions ----------
string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

MyList<string> splitWords(const string& text) {
    MyList<string> words;
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

// ---------- Ignore Word List ----------
const string ignoreWordsArr[] = {
    "in","with","and","the","to","for","needed","required","experience",
    "skills","of","a","an","as","on","by","at","have","has","is","are",
    "job","role","responsibilities","looking","must","be","work"
};
const int ignoreWordCount = sizeof(ignoreWordsArr) / sizeof(ignoreWordsArr[0]);

bool isIgnoredWord(const string& word) {
    for (int i = 0; i < ignoreWordCount; i++) {
        if (ignoreWordsArr[i] == word)
            return true;
    }
    return false;
}

// ---------- Calculate Match Percentage ----------
double calculateMatchPercentage(const string& jobDesc, const string& resumeDesc) {
    MyList<string> jobWords = splitWords(toLowerCase(jobDesc));
    MyList<string> resumeWords = splitWords(toLowerCase(resumeDesc));

    MyList<string> jobSkills;
    MyNode<string>* jwNode = jobWords.getHead();
    while (jwNode) {
        if (!isIgnoredWord(jwNode->data))
            jobSkills.push_back(jwNode->data);
        jwNode = jwNode->next;
    }

    if (jobSkills.size() == 0) return 0.0;

    int matched = 0;
    MyNode<string>* jobSkillNode = jobSkills.getHead();
    while (jobSkillNode) {
        MyNode<string>* rwNode = resumeWords.getHead();
        while (rwNode) {
            if (jobSkillNode->data == rwNode->data) {
                matched++;
                break;
            }
            rwNode = rwNode->next;
        }
        jobSkillNode = jobSkillNode->next;
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
    cout << "=== Job Matching System (Linked List Based, No STL Containers) ===" << endl;

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

    MyList<JobNode*> matchedJobs;
    JobNode* tempJob = jobHead;

    // Search for matching jobs
    while (tempJob) {
        if (containsKeyword(toLowerCase(tempJob->description), keyword)) {
            matchedJobs.push_back(tempJob);
        }
        tempJob = tempJob->next;
    }

    if (matchedJobs.size() == 0) {
        cout << "\nNo job descriptions found with that skill." << endl;
        return 0;
    }

    cout << "\nJobs found with \"" << keyword << "\":\n" << endl;

    int showLimit = (matchedJobs.size() < 20) ? matchedJobs.size() : 20;
    for (int i = 0; i < showLimit; i++) {
        cout << i + 1 << ". " << matchedJobs.at(i)->description << endl;
    }

    if (matchedJobs.size() > 20) {
        char choice;
        cout << "\nMore than 20 jobs found. Do you want to see all? (y/n): ";
        cin >> choice;

        if (tolower(choice) == 'y') {
            for (int i = 20; i < matchedJobs.size(); i++) {
                cout << i + 1 << ". " << matchedJobs.at(i)->description << endl;
            }
        }
    }

    cout << "\nTotal jobs found: " << matchedJobs.size() << endl;

    int jobChoice;
    cout << "\nEnter the job number you want to analyze: ";
    cin >> jobChoice;

    if (jobChoice < 1 || jobChoice > matchedJobs.size()) {
        cout << "Invalid job number!" << endl;
        return 0;
    }

    double minPercentage;
    cout << "Enter the minimum percentage to display resumes (example: 50): ";
    cin >> minPercentage;

    JobNode* selectedJob = matchedJobs.at(jobChoice - 1);
    cout << "\nSelected Job Description:\n" << selectedJob->description << endl;

    cout << "\n--- Resume Match Results ---" << endl;

    // Start timing
    auto startTime = high_resolution_clock::now();

    ResumeNode* tempResume = resumeHead;
    MyList<pair<string, double>> resumeResults;

    while (tempResume) {
        double score = calculateMatchPercentage(selectedJob->description, tempResume->description);
        if (score >= minPercentage && score > 0.0) {
            resumeResults.push_back({tempResume->description, score});
        }
        tempResume = tempResume->next;
    }

    // Show only first 20 resumes
    int resumeLimit = (resumeResults.size() < 20) ? resumeResults.size() : 20;
    for (int i = 0; i < resumeLimit; i++) {
        auto result = resumeResults.at(i);
        cout << i + 1 << ". " << result.first << endl;
        cout << "   Score: " << result.second << "%" << endl << endl;
    }

    if (resumeResults.size() > 20) {
        char choice;
        cout << "More than 20 resumes found. Do you want to see more resumes? (y/n): ";
        cin >> choice;

        if (tolower(choice) == 'y') {
            for (int i = 20; i < resumeResults.size(); i++) {
                auto result = resumeResults.at(i);
                cout << i + 1 << ". " << result.first << endl;
                cout << "   Score: " << result.second << "%" << endl << endl;
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
