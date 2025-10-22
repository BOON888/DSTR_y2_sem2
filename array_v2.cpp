#include "common.h"

void runArrayVersion2() {
    cout << "\n=== Array (Job > Resume) ===\n";

    DynamicArray<Item> resumes;
    DynamicArray<Item> jobs;

    string resumePath = "resume.csv";
    string jobPath = "job_description.csv";

    if (!loadCSV_Array(resumePath, resumes)) {
        cout << "Cannot open resume.csv. Please check the file path.\n";
        return;
    }
    if (!loadCSV_Array(jobPath, jobs)) {
        cout << "Cannot open job_description.csv. Please check the file path.\n";
        return;
    }

    cout << "Loaded " << jobs.size() << " jobs.\n";
    cout << "Loaded " << resumes.size() << " resumes.\n";

    string skill;
    cout << "\nEnter skill to search (example: sql): " << flush;
    getline(cin, skill);
    transform(skill.begin(), skill.end(), skill.begin(), ::tolower);

    cout << "\n===============================\n";
    cout << "STAGE 1: FILTER JOBS BY SKILL (" << skill << ")\n";
    cout << "===============================\n";

    DynamicArray<int> skillJobsIndices;
    for (int i = 0; i < jobs.size(); ++i) {
        if (jobs[i].text.find(skill) != string::npos) {
            skillJobsIndices.push_back(i);
        }
    }

    cout << "\nTotal jobs found with skill '" << skill << "': "
         << skillJobsIndices.size() << endl;

    if (skillJobsIndices.size() == 0) {
        cout << "No jobs contain this skill. Exiting program.\n";
        return;
    }

    // Print first 20 results only
    cout << "\n--- Showing first "
         << min(20, skillJobsIndices.size()) << " matching jobs ---\n";
    for (int i = 0; i < min(20, skillJobsIndices.size()); ++i) {
        int idx = skillJobsIndices[i];
        cout << "Job " << (idx + 1) << ": " << jobs[idx].originalText << "\n";
    }

    // Ask if want to print all
    char choice;
    cout << "\nDo you want to print all " << skillJobsIndices.size()
         << " matching jobs? (y/n): " << flush;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 'y' || choice == 'Y') {
        cout << "\n--- All Matching Jobs ---\n";
        for (int i = 0; i < skillJobsIndices.size(); ++i) {
            int idx = skillJobsIndices[i];
            cout << "Job " << (idx + 1) << ": " << jobs[idx].originalText << "\n";
        }
        cout << "-----------------------------\n";
    } else {
        cout << "Skipped printing full list.\n";
    }

    cout << "\n=========================================\n";
    cout << "STAGE 1 SUMMARY\n";
    cout << "=========================================\n";
    cout << "Skill searched: " << skill << endl;
    cout << "Matching jobs found: " << skillJobsIndices.size() << endl;
    cout << "-----------------------------------------\n";

    int chosenIndex = 0;
    cout << "Enter job number to match with resumes (0 to exit): " << flush;
    if (!(cin >> chosenIndex)) return;
    if (chosenIndex == 0) { cout << "Exiting program.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    double matchThreshold;
    cout << "Enter percentage for resume matching (example: 25): " << flush;
    if (!(cin >> matchThreshold)) return;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n===============================\n";
    cout << "STAGE 2: RESUME MATCHING FOR JOB " << chosenIndex << "\n";
    cout << "===============================\n";

    int idx = chosenIndex - 1;
    cout << "Job " << chosenIndex << ": "
         << jobs[idx].originalText << "\n";

    DynamicArray<string> jwords = tokenizeLower(jobs[idx].text);

    // Store resume index and percent
    struct ResumeMatch {
        int index;
        double percent;
    };

    DynamicArray<ResumeMatch> matchedResumes;

    // ✅ Start timing
    auto start = chrono::high_resolution_clock::now();

    for (int r = 0; r < resumes.size(); ++r) {
        DynamicArray<string> rwords = tokenizeLower(resumes[r].text);
        int matches = countMatches(jwords, rwords);
        double percent = (jwords.size() == 0)
            ? 0.0
            : ((double)matches / (double)jwords.size()) * 100.0;

        if (percent >= matchThreshold) {
            matchedResumes.push_back({r, percent});
        }
    }

    // ✅ Sort matched resumes by percentage (descending)
    for (int i = 0; i < matchedResumes.size() - 1; ++i) {
        for (int j = i + 1; j < matchedResumes.size(); ++j) {
            if (matchedResumes[j].percent > matchedResumes[i].percent) {
                ResumeMatch temp = matchedResumes[i];
                matchedResumes[i] = matchedResumes[j];
                matchedResumes[j] = temp;
            }
        }
    }

    // ✅ End timing
    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "\nTotal resumes matched with above " << matchThreshold << "%: "
        << matchedResumes.size() << endl;

    if (matchedResumes.size() == 0) {
        cout << "This job did not qualify for any resumes.\n";
    } else {
        cout << "\n--- Showing first "
            << min(20, matchedResumes.size()) << " matching resumes ---\n";
        for (int i = 0; i < min(20, matchedResumes.size()); ++i) {
            int r = matchedResumes[i].index;
            double p = matchedResumes[i].percent;
            cout << "Resume " << (r + 1) << " (" << fixed << setprecision(2)
                << p << "%): " << resumes[r].originalText << "\n";
        }

        // Ask if want to print all
        char resumeChoice;
        cout << "\nDo you want to print all " << matchedResumes.size()
            << " matching resumes? (y/n): " << flush;
        cin >> resumeChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (resumeChoice == 'y' || resumeChoice == 'Y') {
            cout << "\n--- All Matching Resumes (Sorted High → Low) ---\n";
            for (int i = 0; i < matchedResumes.size(); ++i) {
                int r = matchedResumes[i].index;
                double p = matchedResumes[i].percent;
                cout << "Resume " << (r + 1) << " (" << fixed << setprecision(2)
                    << p << "%): " << resumes[r].originalText << "\n";
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
    cout << "Resumes matched with above " << matchThreshold << "%: "
         << matchedResumes.size() << endl;
    cout << "Time Taken: " << elapsed << " milliseconds\n";
    cout << "Memory Used: " << getMemoryUsageKB() << " KB\n";
}
