#include "common.h"
#include <thread>

void runLinkedVersion2() {
    cout << "\n=== Linked List (Resume > Job) ===\n";
    LinkedList<Item> resumes, jobs;
    if (!loadCSV_Linked("resume.csv", resumes)) {
        cout << "Cannot open resume.csv.\n";
        return;
    }
    if (!loadCSV_Linked("job_description.csv", jobs)) {
        cout << "Cannot open job_description.csv.\n";
        return;
    }

    cout << "Loaded " << resumes.size() << " resumes and " << jobs.size() << " jobs.\n";

    // ==========================
    // STAGE 1: FILTER RESUMES BY SKILL
    // ==========================
    string skill;
    cout << "\nEnter skill to search (example: sql): " << flush;
    getline(cin, skill);
    skill = toLowerCase(skill);

    cout << "\n===============================\n";
    cout << "STAGE 1: FILTER RESUMES BY SKILL (" << skill << ")\n";
    cout << "===============================\n";

    LinkedList<int> matchedResumesIdx;
    int idx = 0;
    Node<Item>* rnode = resumes.getHead();
    while (rnode) {
        if (rnode->data.text.find(skill) != string::npos)
            matchedResumesIdx.push_back(idx);
        rnode = rnode->next;
        idx++;
    }

    cout << "\nTotal resumes found with skill '" << skill << "': " << matchedResumesIdx.size() << endl;

    if (matchedResumesIdx.size() == 0) {
        cout << "No resumes found with that skill.\n";
        return;
    }

    // Print first 20
    cout << "\n--- Showing first " << min(20, matchedResumesIdx.size()) << " matching resumes ---\n";
    Node<int>* node = matchedResumesIdx.getHead();
    for (int i = 0; i < min(20, matchedResumesIdx.size()) && node; ++i, node = node->next) {
        int resumeIdx = node->data;

        Node<Item>* rn = resumes.getHead();
        for (int c = 0; c < resumeIdx && rn; ++c) rn = rn->next;

        if (rn) cout << "Resume " << resumeIdx + 1 << ": " << rn->data.originalText << "\n";
    }

    // Ask if want to print all
    char choice;
    cout << "\nDo you want to print all " << matchedResumesIdx.size() << " matching resumes? (y/n): " << flush;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 'y' || choice == 'Y') {
        cout << "\n--- All Matching Resumes ---\n";
        Node<int>* temp = matchedResumesIdx.getHead();
        while (temp) {
            int resumeIdx = temp->data;
            Node<Item>* rn = resumes.getHead();
            for (int c = 0; c < resumeIdx && rn; ++c) rn = rn->next;

            if (rn) cout << "Resume " << resumeIdx + 1 << ": " << rn->data.originalText << "\n";
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
    cout << "Matching resumes found: " << matchedResumesIdx.size() << endl;
    cout << "-----------------------------------------\n";

    // ==========================
    // STAGE 2: MATCH JOBS FOR SELECTED RESUME
    // ==========================
    int chosenIndex = 0;
    cout << "Enter resume number to match with jobs (0 to exit): " << flush;
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
    cout << "Enter percentage for job matching (example: 25): " << flush;
    cin >> matchThreshold;

    if (cin.fail() || matchThreshold < 1 || matchThreshold > 100) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid percentage. Please enter a number between 1 and 100.\n";
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n===============================\n";
    cout << "STAGE 2: JOB MATCHING FOR RESUME " << chosenIndex << "\n";
    cout << "===============================\n";

    // Find selected resume
    Node<int>* selNode = matchedResumesIdx.getHead();
    int selectedResumeIndex = -1;

    while (selNode) {
        if (selNode->data + 1 == chosenIndex) {
            selectedResumeIndex = selNode->data;
            break;
        }
        selNode = selNode->next;
    }

    if (selectedResumeIndex == -1) {
        cout << "Invalid resume selection. Please enter a valid resume number from the list.\n";
        return;
    }

    Node<Item>* selectedResumeNode = resumes.getHead();
    for (int i = 0; i < selectedResumeIndex && selectedResumeNode; ++i)
        selectedResumeNode = selectedResumeNode->next;

    if (!selectedResumeNode) {
        cout << "Internal error: selected resume not found.\n";
        return;
    }

    cout << "\nSelected Resume (" << chosenIndex << "):\n"
        << selectedResumeNode->data.originalText << "\n\n";

    // ==========================
    // MATCHING JOBS — TIMED SECTION
    // ==========================
    auto start = chrono::high_resolution_clock::now();

    LinkedList<pair<int, double>> qualifiedJobs;
    int jIdx = 0;
    for (Node<Item>* jNode = jobs.getHead(); jNode; jNode = jNode->next, ++jIdx) {
        DynamicArray<string> resumeWords = tokenizeLower(selectedResumeNode->data.text);
        DynamicArray<string> jobWords = tokenizeLower(jNode->data.text);
        int matches = countMatches(resumeWords, jobWords);
        double percent = (resumeWords.size() == 0) ? 0.0 : ((double)matches / resumeWords.size()) * 100.0;

        if (percent >= matchThreshold)
            qualifiedJobs.push_back({jIdx, percent});
    }

    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // ==========================
    // DISPLAY RESULTS (NOT TIMED)
    // ==========================
    cout << "Total jobs matched with above " << matchThreshold << "%: " << qualifiedJobs.size() << endl;

    if (qualifiedJobs.size() == 0) {
        cout << "No jobs qualified for this resume.\n";
    } else {
        cout << "\n--- Showing first " << min(20, qualifiedJobs.size()) << " matching jobs ---\n";
        Node<pair<int, double>>* jobNode = qualifiedJobs.getHead();
        for (int i = 0; i < min(20, qualifiedJobs.size()) && jobNode; ++i, jobNode = jobNode->next) {
            int idx = jobNode->data.first;
            double percent = jobNode->data.second;

            Node<Item>* j = jobs.getHead();
            for (int c = 0; c < idx && j; ++c) j = j->next;

            if (j) {
                cout << "Job " << idx + 1 << " (" << fixed << setprecision(2)
                    << percent << "%): " << j->data.originalText << "\n";
            }
        }

        char jobChoice;
        cout << "\nDo you want to print all " << qualifiedJobs.size()
            << " matching jobs? (y/n): " << flush;
        cin >> jobChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (jobChoice == 'y' || jobChoice == 'Y') {
            cout << "\n--- All Matching Jobs ---\n";
            Node<pair<int, double>>* fullNode = qualifiedJobs.getHead();
            while (fullNode) {
                int idx = fullNode->data.first;
                double percent = fullNode->data.second;

                Node<Item>* j = jobs.getHead();
                for (int c = 0; c < idx && j; ++c) j = j->next;

                if (j) {
                    cout << "Job " << idx + 1 << " (" << fixed << setprecision(2)
                        << percent << "%): " << j->data.originalText << "\n";
                }
                fullNode = fullNode->next;
            }
            cout << "-----------------------------\n";
        } else {
            cout << "Skipped printing full job list.\n";
        }
    }

    cout << "\n=========================================\n";
    cout << "STAGE 2 SUMMARY (RESUME " << chosenIndex << ")\n";
    cout << "=========================================\n";
    cout << "Total jobs checked: " << jobs.size() << endl;
    cout << "Jobs matched with above " << matchThreshold << "%: " << qualifiedJobs.size() << endl;
    cout << "Time Taken (Matching Only): " << elapsed << " milliseconds\n";
    cout << "Memory Used: " << getMemoryUsageKB() << " KB\n";
}