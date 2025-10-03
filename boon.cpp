#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <ctime>
using namespace std;

// ====== SETTINGS ======
const int RESUME_LIMIT = 10000; // number of resumes to process
const int JOB_LIMIT = 10000;    // number of jobs to process
// ======================

// Node for linked list
struct Node
{
    string text;
    Node *next;
    Node(string t) : text(t), next(nullptr) {}
};

class LinkedList
{
private:
    Node *head;

public:
    LinkedList() : head(nullptr) {}

    void insert(string value)
    {
        Node *newNode = new Node(value);
        if (!head)
        {
            head = newNode;
            return;
        }
        Node *temp = head;
        while (temp->next)
            temp = temp->next;
        temp->next = newNode;
    }

    vector<string> toVector()
    {
        vector<string> vec;
        Node *temp = head;
        while (temp)
        {
            vec.push_back(temp->text);
            temp = temp->next;
        }
        return vec;
    }
};

void loadCSV(string filename, LinkedList &list)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    string line;
    getline(file, line); // skip header
    while (getline(file, line))
    {
        if (!line.empty())
        {
            // Remove wrapping quotes if present
            if (line.front() == '"' && line.back() == '"')
            {
                line = line.substr(1, line.size() - 2);
            }
            list.insert(line);
        }
    }
    file.close();
}

// Tokenize into lowercase words
vector<string> tokenize(string text)
{
    vector<string> tokens;
    stringstream ss(text);
    string word;
    while (ss >> word)
    {
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        tokens.push_back(word);
    }
    return tokens;
}

// Count overlap between resume and job description
int matchScore(string resume, string job)
{
    vector<string> rwords = tokenize(resume);
    vector<string> jwords = tokenize(job);
    int score = 0;
    for (string &rw : rwords)
    {
        if (find(jwords.begin(), jwords.end(), rw) != jwords.end())
            score++;
    }
    return score;
}

int main()
{
    LinkedList resumes, jobs;

    // Load datasets
    loadCSV("resume.csv", resumes);
    loadCSV("job_description.csv", jobs);

    // Convert to vectors
    vector<string> resumeList = resumes.toVector();
    vector<string> jobList = jobs.toVector();

    cout << "Loaded " << resumeList.size() << " resumes." << endl;
    cout << "Loaded " << jobList.size() << " jobs." << endl;

    int totalResumes = min(RESUME_LIMIT, (int)resumeList.size());
    int totalJobs = min(JOB_LIMIT, (int)jobList.size());

    cout << "\nMatching " << totalResumes
         << " resumes to " << totalJobs << " jobs...\n";

    // Track execution time
    clock_t start = clock();

    // Match resumes
    for (int i = 0; i < totalResumes; i++)
    {
        int bestScore = 0;
        int bestJob = -1;

        for (int j = 0; j < totalJobs; j++)
        {
            int score = matchScore(resumeList[i], jobList[j]);
            if (score > bestScore)
            {
                bestScore = score;
                bestJob = j;
            }
        }

        // Print only the best match
        cout << "\nResume " << i + 1 << ": " << resumeList[i] << endl;
        if (bestJob != -1)
        {
            cout << "Best Match: Job " << bestJob + 1
                 << " with score " << bestScore << endl;
            cout << "   Job Description: " << jobList[bestJob] << endl;
        }
        else
        {
            cout << "No suitable job found.\n";
        }
    }

    clock_t end = clock();
    double elapsed = double(end - start) / CLOCKS_PER_SEC;

    cout << "\nMatching completed in " << elapsed << " seconds." << endl;

    return 0;
}
