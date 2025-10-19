#include "common.h"
#include <thread>

void runLinkedVersion() {
    cout << "\n=== Linked List (Job > Resume) ===\n";
    LinkedList<Item> jobs, resumes;

    if (!loadCSV_Linked("job_description.csv", jobs)) {
        cout << "Cannot open job_description.csv.\n";
        return;
    }
    if (!loadCSV_Linked("resume.csv", resumes)) {
        cout << "Cannot open resume.csv.\n";
        return;
    }

    cout << "Loaded " << jobs.size() << " jobs and " << resumes.size() << " resumes.\n";

    // ==========================
    // STAGE 1: FILTER JOBS BY SKILL
    // ==========================
    string skill;
    cout << "\nEnter skill to search (example: sql): " << flush;
    getline(cin, skill);
    skill = toLowerCase(skill);

    cout << "\n===============================\n";
    cout << "STAGE 1: FILTER JOBS BY SKILL (" << skill << ")\n";
    cout << "===============================\n";

    LinkedList<int> matchedJobsIdx;
    int idx = 0;
    for (Node<Item>* node = jobs.getHead(); node; node = node->next, ++idx) {
        if (node->data.text.find(skill) != string::npos)
            matchedJobsIdx.push_back(idx);
    }

    cout << "\nTotal jobs found with skill '" << skill << "': " << matchedJobsIdx.size() << endl;

    if (matchedJobsIdx.size() == 0) {
        cout << "No jobs found with that skill.\n";
        return;
    }

    // Print first 20
    cout << "\n--- Showing first " << min(20, matchedJobsIdx.size()) << " matching jobs ---\n";
    Node<int>* node = matchedJobsIdx.getHead();
    for (int i = 0; i < min(20, matchedJobsIdx.size()) && node; ++i, node = node->next) {
        int jobIdx = node->data;

        Node<Item>* jn = jobs.getHead();
        for (int c = 0; c < jobIdx && jn; ++c) jn = jn->next;

        if (jn) cout << "Job " << jobIdx + 1 << ": " << jn->data.originalText << "\n";
    }

    // Ask if want to print all
    char choice;
    cout << "\nDo you want to print all " << matchedJobsIdx.size() << " matching jobs? (y/n): " << flush;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 'y' || choice == 'Y') {
        cout << "\n--- All Matching Jobs ---\n";
        Node<int>* temp = matchedJobsIdx.getHead();
        while (temp) {
            int jobIdx = temp->data;
            Node<Item>* jn = jobs.getHead();
            for (int c = 0; c < jobIdx && jn; ++c) jn = jn->next;

            if (jn) cout << "Job " << jobIdx + 1 << ": " << jn->data.originalText << "\n";
            temp = temp->next;
        }
        cout << "-----------------------------\n";
    } else {
        cout << "Skipped printing full list.\n";
    }

    cout << "\n=========================================\n";
    cout << "STAGE 1 SUMMARY\n";
    cout << "=========================================\n";
    cout << "Skill searched: " << skill << endl;
    cout << "Matching jobs found: " << matchedJobsIdx.size() << endl;
    cout << "-----------------------------------------\n";

    // ==========================
    // STAGE 2: MATCH RESUMES FOR SELECTED JOB
    // ==========================
    int chosenIndex = 0;
    cout << "Enter job number to match with resumes (0 to exit): " << flush;
    if (!(cin >> chosenIndex)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a valid number.\n";
        return;
    }
    if (chosenIndex == 0) {
        cout << "Exiting program.\n";
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    double matchThreshold;
    cout << "Enter percentage for resume matching (example: 25): " << flush;
    cin >> matchThreshold;

    if (cin.fail() || matchThreshold < 1 || matchThreshold > 100) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid percentage. Please enter a number between 1 and 100.\n";
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n===============================\n";
    cout << "STAGE 2: RESUME MATCHING FOR JOB " << chosenIndex << "\n";
    cout << "===============================\n";

    // Find selected job using displayed number
    Node<int>* selNode = matchedJobsIdx.getHead();
    int selectedJobIndex = -1;

    while (selNode) {
        if (selNode->data + 1 == chosenIndex) {
            selectedJobIndex = selNode->data;
            break;
        }
        selNode = selNode->next;
    }

    if (selectedJobIndex == -1) {
        cout << "Invalid job selection. Please enter a valid job number from the list.\n";
        return;
    }

    Node<Item>* selectedJobNode = jobs.getHead();
    for (int i = 0; i < selectedJobIndex && selectedJobNode; ++i)
        selectedJobNode = selectedJobNode->next;

    if (!selectedJobNode) {
        cout << "Internal error: selected job not found.\n";
        return;
    }

    cout << "\nSelected Job (" << chosenIndex << "):\n"
        << selectedJobNode->data.originalText << "\n\n";

    // ==========================
    // MATCHING RESUMES — TIMED SECTION
    // ==========================
    auto start = chrono::high_resolution_clock::now();

    LinkedList<int> qualifiedResumeIdx;
    int rIdx = 0;
    for (Node<Item>* rNode = resumes.getHead(); rNode; rNode = rNode->next, ++rIdx) {
        DynamicArray<string> jobWords = tokenizeLower(selectedJobNode->data.text);
        DynamicArray<string> resumeWords = tokenizeLower(rNode->data.text);
        int matches = countMatches(jobWords, resumeWords);
        double percent = (jobWords.size() == 0) ? 0.0 : ((double)matches / jobWords.size()) * 100.0;

        if (percent >= matchThreshold)
            qualifiedResumeIdx.push_back(rIdx);
    }

    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // ==========================
    // DISPLAY RESULTS (NOT TIMED)
    // ==========================
    cout << "Total resumes matched with above " << matchThreshold << "%: " << qualifiedResumeIdx.size() << endl;

    if (qualifiedResumeIdx.size() == 0) {
        cout << "No resumes qualified for this job.\n";
    } else {
        cout << "\n--- Showing first " << min(20, qualifiedResumeIdx.size()) << " matching resumes ---\n";
        Node<int>* resNode = qualifiedResumeIdx.getHead();
        for (int i = 0; i < min(20, qualifiedResumeIdx.size()) && resNode; ++i, resNode = resNode->next) {
            int idx = resNode->data;

            Node<Item>* r = resumes.getHead();
            for (int c = 0; c < idx && r; ++c) r = r->next;

            if (r) {
                DynamicArray<string> jobWords = tokenizeLower(selectedJobNode->data.text);
                DynamicArray<string> resumeWords = tokenizeLower(r->data.text);
                int matches = countMatches(jobWords, resumeWords);
                double percent = (jobWords.size() == 0) ? 0.0 : ((double)matches / jobWords.size()) * 100.0;

                cout << "Resume " << idx + 1 << " (" << fixed << setprecision(2)
                    << percent << "%): " << r->data.originalText << "\n";
            }
        }

        char resChoice;
        cout << "\nDo you want to print all " << qualifiedResumeIdx.size()
            << " matching resumes? (y/n): " << flush;
        cin >> resChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (resChoice == 'y' || resChoice == 'Y') {
            cout << "\n--- All Matching Resumes ---\n";
            Node<int>* fullNode = qualifiedResumeIdx.getHead();
            while (fullNode) {
                int idx = fullNode->data;
                Node<Item>* r = resumes.getHead();
                for (int c = 0; c < idx && r; ++c) r = r->next;

                if (r) {
                    DynamicArray<string> jobWords = tokenizeLower(selectedJobNode->data.text);
                    DynamicArray<string> resumeWords = tokenizeLower(r->data.text);
                    int matches = countMatches(jobWords, resumeWords);
                    double percent = (jobWords.size() == 0) ? 0.0 : ((double)matches / jobWords.size()) * 100.0;

                    cout << "Resume " << idx + 1 << " (" << fixed << setprecision(2)
                        << percent << "%): " << r->data.originalText << "\n";
                }
                fullNode = fullNode->next;
            }
            cout << "-----------------------------\n";
        } else {
            cout << "Skipped printing full resume list.\n";
        }
    }

    cout << "\n=========================================\n";
    cout << "STAGE 2 SUMMARY (JOB " << chosenIndex << ")\n";
    cout << "=========================================\n";
    cout << "Total resumes checked: " << resumes.size() << endl;
    cout << "Resumes matched with above " << matchThreshold << "%: " << qualifiedResumeIdx.size() << endl;
    cout << "Time Taken (Matching Only): " << elapsed << " milliseconds\n";
    cout << "Memory Used: " << getMemoryUsageKB() << " KB\n";
}