#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm> // Still needed for transform, remove_if, ispunct
#include <iomanip>
#include <chrono>    // for milliseconds timing
// NOTE: <vector> and <list> are now excluded
using namespace std;

// ====== SETTINGS ======
const int RESUME_LIMIT = 10000; // Not strictly used after removing vector sizing logic, but kept for context
const int JOB_LIMIT    = 10000; // Same as above
// ======================

// Linked List Node
struct Node {
    string text;
    string originalText;
    int index; // Added to store index for filtering (replaces skillResumes vector)
    Node* next;

    // Constructor for text/originalText storage (for CSV data)
    Node(string t, string o) : text(t), originalText(o), index(-1), next(nullptr) {}
    
    // Constructor for index storage (for skillResumes indices)
    Node(int i) : text(""), originalText(""), index(i), next(nullptr) {} 

    // Constructor for tokens (text only)
    Node(string t) : text(t), originalText(""), index(-1), next(nullptr) {}
};

// Linked List Class - Used for CSV data, tokens, and filtered indices
class LinkedList {
private:
    Node* head;
    int count; // Keep track of size without a vector
public:
    LinkedList() : head(nullptr), count(0) {}
    ~LinkedList() { // Added destructor for proper memory cleanup
        Node* current = head;
        Node* next;
        while (current != nullptr) {
            next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
    }

    // Insert for CSV data
    void insert(string value, int idx = -1) {
        string lower = value;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        Node* newNode = new Node(lower, value);
        newNode->index = idx; // Store index if provided (for job/resume lists)

        if (!head) { head = newNode; }
        else {
            Node* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
        count++;
    }

    // Insert for index (to replace skillResumes vector)
    void insertIndex(int idx) {
        Node* newNode = new Node(idx);
        if (!head) { head = newNode; }
        else {
            Node* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
        count++;
    }

    // Insert for token (lower case only)
    void insertToken(string lowerToken) {
        Node* newNode = new Node(lowerToken);
        if (!head) { head = newNode; }
        else {
            Node* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
        count++;
    }

    int size() const { return count; }
    Node* getHead() const { return head; }

    // Helper function to get data at a specific index (Replaces vector[i] access)
    // NOTE: This O(N) access is a major performance change from O(1) vector access.
    Node* get(int idx) {
        if (idx < 0 || idx >= count) return nullptr;
        Node* temp = head;
        for (int i = 0; i < idx; ++i) {
            temp = temp->next;
        }
        return temp;
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
    int index = 0;
    while (getline(file, line)) {
        if (!line.empty()) {
            if (line.front() == '"' && line.back() == '"')
                line = line.substr(1, line.size() - 2);
            list.insert(line, index++);
        }
    }
    file.close();
    return true;
}

// Tokenize text - now returns a LinkedList instead of a vector
LinkedList tokenize(string text) {
    LinkedList tokens;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        if (!word.empty())
            tokens.insertToken(word); // Store token in the linked list
    }
    return tokens;
}

// Helper: Check if a string exists in a LinkedList of tokens
bool findInList(const LinkedList &list, const string &target) {
    Node* temp = list.getHead();
    while (temp) {
        if (temp->text == target) return true;
        temp = temp->next;
    }
    return false;
}

// Count overlapping words - takes LinkedLists
int countMatches(const LinkedList &rwords, const LinkedList &jwords) {
    int count = 0;
    Node* rTemp = rwords.getHead();
    while (rTemp) {
        // Use the new helper function to replace std::find
        if (findInList(jwords, rTemp->text))
            count++;
        rTemp = rTemp->next;
    }
    return count;
}

// MAIN PROGRAM
int main() {
    // LinkedLists replace the vectors for storing all CSV data
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

    cout << "Loaded " << resumes.size() << " resumes." << endl;
    cout << "Loaded " << jobs.size() << " jobs." << endl;

    string skill;
    cout << "\nEnter skill to search: ";
    getline(cin, skill);
    transform(skill.begin(), skill.end(), skill.begin(), ::tolower);

    cout << "\n===============================" << endl;
    cout << "STAGE 1: FILTER BY SKILL (" << skill << ")" << endl;
    cout << "===============================" << endl;

    // LinkedList replaces the skillResumes vector
    LinkedList skillResumesIndices; 
    
    // Iterate over the LinkedList for filtering
    Node* currentResume = resumes.getHead();
    int i = 0;
    while (currentResume) {
        if (currentResume->text.find(skill) != string::npos) {
            skillResumesIndices.insertIndex(i); // Store the index
            cout << "Resume " << i + 1 << ": " << currentResume->originalText << endl;
        }
        currentResume = currentResume->next;
        i++;
    }

    cout << "\nTotal resumes found with skill '" << skill << "': " << skillResumesIndices.size() << endl;

    if (skillResumesIndices.size() == 0) {
        cout << "No resumes contain this skill. Exiting program.\n";
        return 0;
    }

    cout << "\n=========================================\n";
    cout << "STAGE 1 SUMMARY\n";
    cout << "=========================================\n";
    cout << "Skill searched: " << skill << endl;
    cout << "Matching resumes found: " << skillResumesIndices.size() << endl;
    cout << "-----------------------------------------\n";

    int chosenIndex;
    cout << "Enter resume number to match with jobs (0 to exit): ";
    cin >> chosenIndex;
    if (chosenIndex == 0) {
        cout << "Exiting program.\n";
        return 0;
    }

    // Check if chosenIndex is valid by iterating the skillResumesIndices list
    bool validChoice = false;
    Node* currentIdxNode = skillResumesIndices.getHead();
    while(currentIdxNode) {
        if (currentIdxNode->index == chosenIndex - 1) {
            validChoice = true;
            break;
        }
        currentIdxNode = currentIdxNode->next;
    }

    if (!validChoice) {
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
    // Get the chosen resume data using the O(N) get() helper
    Node* chosenResumeNode = resumes.get(idx);
    if (!chosenResumeNode) {
        cout << "Internal error: Resume not found.\n";
        return 0;
    }

    cout << "Resume " << chosenIndex << ": " << chosenResumeNode->originalText << endl;

    // Tokenize the chosen resume text into a LinkedList
    LinkedList rwords = tokenize(chosenResumeNode->text); 
    bool foundQualified = false;
    int qualifiedCount = 0;

    // Iterate through the jobs LinkedList
    Node* currentJob = jobs.getHead();
    int j = 0;
    while (currentJob) {
        // Tokenize the current job description text into a LinkedList
        LinkedList jwords = tokenize(currentJob->text); 
        int matches = countMatches(rwords, jwords);
        
        // Size is now retrieved from the LinkedList's count field
        double percent = ((double)matches / rwords.size()) * 100.0; 

        if (percent >= matchThreshold) {
            if (!foundQualified)
                cout << "\nJobs matching ≥ " << matchThreshold << "%:\n";
            foundQualified = true;
            qualifiedCount++;
            cout << "   - Job " << j + 1 << " (" << fixed << setprecision(2)
                 << percent << "%)\n";
            cout << "     " << currentJob->originalText << endl;
        }

        currentJob = currentJob->next;
        j++;
    }

    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "\n=========================================\n";
    cout << "STAGE 2 SUMMARY (RESUME " << chosenIndex << ")\n";
    cout << "=========================================\n";
    cout << "Total jobs checked: " << jobs.size() << endl;
    cout << "Jobs " << matchThreshold << "% and above match: " << qualifiedCount << endl;
    if (qualifiedCount == 0)
        cout << "This resume did not qualify for any job.\n";
    cout << "Time Taken: " << elapsed << " milliseconds\n";

    return 0;
}