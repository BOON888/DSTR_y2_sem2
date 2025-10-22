#include "common.h"

void runArrayVersion() {
    cout << "\n=== Array (Resume > Job) ===\n";

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

    cout << "Loaded " << resumes.size() << " resumes.\n";
    cout << "Loaded " << jobs.size() << " jobs.\n";

    string skill;
    cout << "\nEnter skill to search (example: sql): " << flush;
    getline(cin, skill);
    transform(skill.begin(), skill.end(), skill.begin(), ::tolower);

    cout << "\n===============================\n";
    cout << "STAGE 1: FILTER BY SKILL (" << skill << ")\n";
    cout << "===============================\n";

    DynamicArray<int> skillResumesIndices;
    for (int i = 0; i < resumes.size(); ++i) {
        if (resumes[i].text.find(skill) != string::npos) {
            skillResumesIndices.push_back(i);
        }
    }

    cout << "\nTotal resumes found with skill '" << skill << "': "
         << skillResumesIndices.size() << endl;

    if (skillResumesIndices.size() == 0) {
        cout << "No resumes contain this skill. Exiting program.\n";
        return;
    }

    // Print first 20 results only
    cout << "\n--- Showing first " 
         << min(20, skillResumesIndices.size()) << " matching resumes ---\n";
    for (int i = 0; i < min(20, skillResumesIndices.size()); ++i) {
        int idx = skillResumesIndices[i];
        cout << "Resume " << (idx + 1) << ": " << resumes[idx].originalText << "\n";
    }

    // Ask if want to print all
    char choice;
    cout << "\nDo you want to print all " << skillResumesIndices.size()
         << " matching resumes? (y/n): " << flush;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 'y' || choice == 'Y') {
        cout << "\n--- All Matching Resumes ---\n";
        for (int i = 0; i < skillResumesIndices.size(); ++i) {
            int idx = skillResumesIndices[i];
            cout << "Resume " << (idx + 1) << ": " << resumes[idx].originalText << "\n";
        }
        cout << "-----------------------------\n";
    } else {
        cout << "Skipped printing full list.\n";
    }

    cout << "\n=========================================\n";
    cout << "STAGE 1 SUMMARY\n";
    cout << "=========================================\n";
    cout << "Skill searched: " << skill << endl;
    cout << "Matching resumes found: " << skillResumesIndices.size() << endl;
    cout << "-----------------------------------------\n";

    int chosenIndex = 0;
    cout << "Enter resume number to match with jobs (0 to exit): " << flush;
    if (!(cin >> chosenIndex)) return;
    if (chosenIndex == 0) { cout << "Exiting program.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    double matchThreshold;
    cout << "Enter percentage for job matching (example: 25): " << flush;
    if (!(cin >> matchThreshold)) return;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\n===============================\n";
    cout << "STAGE 2: JOB MATCHING FOR RESUME " << chosenIndex << "\n";
    cout << "===============================\n";

    int idx = chosenIndex - 1;
    cout << "Resume " << chosenIndex << ": " 
         << resumes[idx].originalText << "\n";

    DynamicArray<string> rwords = tokenizeLower(resumes[idx].text);

    // Structure to hold job index + percent match
    struct JobMatch {
        int index;
        double percent;
    };

    DynamicArray<JobMatch> matchedJobs;

    // ✅ Start timing
    auto start = chrono::high_resolution_clock::now();

    for (int j = 0; j < jobs.size(); ++j) {
        DynamicArray<string> jwords = tokenizeLower(jobs[j].text);
        int matches = countMatches(rwords, jwords);
        double percent = (rwords.size() == 0)
            ? 0.0
            : ((double)matches / (double)rwords.size()) * 100.0;

        if (percent >= matchThreshold) {
            matchedJobs.push_back({j, percent});
        }
    }

    // ✅ Sort by descending percentage
    for (int i = 0; i < matchedJobs.size() - 1; ++i) {
        for (int k = i + 1; k < matchedJobs.size(); ++k) {
            if (matchedJobs[k].percent > matchedJobs[i].percent) {
                JobMatch temp = matchedJobs[i];
                matchedJobs[i] = matchedJobs[k];
                matchedJobs[k] = temp;
            }
        }
    }

    // ✅ End timing
    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "\nTotal jobs matched with above " << matchThreshold << "%: "
        << matchedJobs.size() << endl;

    if (matchedJobs.size() == 0) {
        cout << "This resume did not qualify for any jobs.\n";
    } else {
        // Print first 20 results only
        cout << "\n--- Showing first "
            << min(20, matchedJobs.size()) << " matching jobs (sorted high → low) ---\n";
        for (int i = 0; i < min(20, matchedJobs.size()); ++i) {
            int j = matchedJobs[i].index;
            double p = matchedJobs[i].percent;
            cout << "Job " << (j + 1) << " (" << fixed << setprecision(2)
                << p << "%): " << jobs[j].originalText << "\n";
        }

        // Ask if want to print all
        char jobChoice;
        cout << "\nDo you want to print all " << matchedJobs.size()
            << " matching jobs? (y/n): " << flush;
        cin >> jobChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (jobChoice == 'y' || jobChoice == 'Y') {
            cout << "\n--- All Matching Jobs (Sorted High → Low) ---\n";
            for (int i = 0; i < matchedJobs.size(); ++i) {
                int j = matchedJobs[i].index;
                double p = matchedJobs[i].percent;
                cout << "Job " << (j + 1) << " (" << fixed << setprecision(2)
                    << p << "%): " << jobs[j].originalText << "\n";
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
    cout << "Jobs matched with above " << matchThreshold << "%: " 
         << matchedJobs.size() << endl;
    cout << "Time Taken: " << elapsed << " milliseconds\n";
    cout << "Memory Used: " << getMemoryUsageKB() << " KB\n";
}
