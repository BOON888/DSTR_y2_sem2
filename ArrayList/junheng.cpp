#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <limits>
using namespace std;
using namespace std::chrono;

// Simple DynamicArray (no STL containers)
template<typename T>
class DynamicArray {
private:
	T* data;
	int capacity;
	int length;

	void grow() {
		int newCap = capacity * 2;
		T* nd = new T[newCap];
		for (int i = 0; i < length; ++i) nd[i] = data[i];
		delete[] data;
		data = nd;
		capacity = newCap;
	}

public:
	DynamicArray(int cap = 16) : capacity(cap), length(0) {
		data = new T[capacity];
	}
	~DynamicArray() { delete[] data; }
	void push_back(const T& v) {
		if (length == capacity) grow();
		data[length++] = v;
	}
	T& operator[](int i) { return data[i]; }
	const T& operator[](int i) const { return data[i]; }
	int size() const { return length; }
	void clear() { delete[] data; data = new T[capacity]; length = 0; }
};

// Helper data structures
struct Result { std::string text; double score; };

// Utility helpers
string toLowerCase(string s) {
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

DynamicArray<string> splitWords(const string& text) {
	DynamicArray<string> words;
	string w;
	for (size_t i = 0; i < text.size(); ++i) {
		char c = text[i];
		if (isalnum((unsigned char)c)) {
			w.push_back(c);
		} else {
			if (!w.empty()) { words.push_back(w); w.clear(); }
		}
	}
	if (!w.empty()) words.push_back(w);
	return words;
}

// ignore words
const string ignoreWordsArr[] = {
	"in","with","and","the","to","for","needed","required","experience",
	"skills","of","a","an","as","on","by","at","have","has","is","are",
	"job","role","responsibilities","looking","must","be","work"
};
const int ignoreWordCount = sizeof(ignoreWordsArr) / sizeof(ignoreWordsArr[0]);

bool isIgnoredWord(const string& w) {
	for (int i = 0; i < ignoreWordCount; ++i) if (ignoreWordsArr[i] == w) return true;
	return false;
}

// Tokenize: build alphanumeric tokens in lower-case
DynamicArray<string> tokenizeLower(const string& text) {
	DynamicArray<string> toks;
	string cur;
	for (char ch : text) {
		if (isalnum((unsigned char)ch)) cur.push_back(ch);
		else {
			if (!cur.empty()) { transform(cur.begin(), cur.end(), cur.begin(), ::tolower); toks.push_back(cur); cur.clear(); }
		}
	}
	if (!cur.empty()) { transform(cur.begin(), cur.end(), cur.begin(), ::tolower); toks.push_back(cur); }
	return toks;
}

double calculateMatchPercentage(const string& jobDesc, const string& resumeDesc) {
	DynamicArray<string> jobWords = tokenizeLower(jobDesc);
	DynamicArray<string> resumeWords = tokenizeLower(resumeDesc);

	DynamicArray<string> jobSkills;
	for (int i = 0; i < jobWords.size(); ++i) {
		if (!isIgnoredWord(jobWords[i])) jobSkills.push_back(jobWords[i]);
	}

	if (jobSkills.size() == 0) return 0.0;

	int matched = 0;
	for (int i = 0; i < jobSkills.size(); ++i) {
		for (int j = 0; j < resumeWords.size(); ++j) {
			if (jobSkills[i] == resumeWords[j]) { matched++; break; }
		}
	}
	return (matched * 100.0) / jobSkills.size();
}

// Load CSV into DynamicArray<string>
bool loadLines(const string& filename, DynamicArray<string>& out) {
	ifstream f(filename);
	if (!f.is_open()) return false;
	string line;
	while (getline(f, line)) {
		if (line.empty()) continue;
		out.push_back(line);
	}
	f.close();
	return true;
}

int main() {
	cout << "=== Job Matching System (Array-Based) ===\n";

	DynamicArray<string> jobs;
	DynamicArray<string> resumes;

	if (!loadLines("job_description.csv", jobs)) {
		cout << "Error opening job_description.csv" << endl; return 1;
	}
	if (!loadLines("resume.csv", resumes)) {
		cout << "Error opening resume.csv" << endl; return 1;
	}

	cout << "Loaded " << resumes.size() << " resumes." << endl;
	cout << "Loaded " << jobs.size() << " jobs." << endl;

	string keyword;
	cout << "\nEnter a skill keyword to search (example: sql): ";
	if (!getline(cin, keyword)) return 0;
	keyword = toLowerCase(keyword);

	DynamicArray<int> matchedJobIdx;
	for (int i = 0; i < jobs.size(); ++i) {
		string low = toLowerCase(jobs[i]);
		if (low.find(keyword) != string::npos) matchedJobIdx.push_back(i);
	}

	if (matchedJobIdx.size() == 0) { cout << "\nNo job descriptions found with that skill." << endl; return 0; }

	cout << "\nJobs found with '" << keyword << "':\n" << endl;
	int showLimit = (matchedJobIdx.size() < 20) ? matchedJobIdx.size() : 20;
	for (int i = 0; i < showLimit; ++i) {
		cout << i+1 << ". " << jobs[ matchedJobIdx[i] ] << endl;
	}
	if (matchedJobIdx.size() > 20) {
		cout << "\nMore than 20 jobs found. Do you want to see all? (y/n): ";
		string resp; getline(cin, resp);
		if (!resp.empty() && (resp[0]=='y' || resp[0]=='Y')) {
			for (int i = 20; i < matchedJobIdx.size(); ++i) cout << i+1 << ". " << jobs[ matchedJobIdx[i] ] << endl;
		}
	}

	cout << "\nTotal jobs found: " << matchedJobIdx.size() << endl;
	cout << "\nEnter the job number you want to analyze: ";
	string selLine; if (!getline(cin, selLine)) return 0;
	int jobChoice = stoi(selLine);
	if (jobChoice < 1 || jobChoice > matchedJobIdx.size()) { cout << "Invalid job number!" << endl; return 0; }

	cout << "Enter the minimum percentage to display resumes (example: 50): ";
	string percLine; if (!getline(cin, percLine)) return 0;
	double minPercentage = stod(percLine);

	int selectedJobIdx = matchedJobIdx[ jobChoice - 1 ];
	cout << "\nSelected Job Description:\n" << jobs[selectedJobIdx] << endl;

	cout << "\n--- Resume Match Results ---" << endl;
	auto start = high_resolution_clock::now();

	DynamicArray<Result> resumeResults;
	for (int i = 0; i < resumes.size(); ++i) {
		double score = calculateMatchPercentage(jobs[selectedJobIdx], resumes[i]);
		if (score >= minPercentage && score > 0.0) {
			Result r; r.text = resumes[i]; r.score = score; resumeResults.push_back(r);
		}
	}

	int resumeLimit = (resumeResults.size() < 20) ? resumeResults.size() : 20;
	for (int i = 0; i < resumeLimit; ++i) {
		cout << i+1 << ". " << resumeResults[i].text << endl;
		cout << "   Score: " << fixed << setprecision(2) << resumeResults[i].score << "%" << endl << endl;
	}
	if (resumeResults.size() > 20) {
		cout << "More than 20 resumes found. Do you want to see more resumes? (y/n): ";
		string r; getline(cin, r);
		if (!r.empty() && (r[0]=='y' || r[0]=='Y')) {
			for (int i = 20; i < resumeResults.size(); ++i) {
				cout << i+1 << ". " << resumeResults[i].text << endl;
				cout << "   Score: " << fixed << setprecision(2) << resumeResults[i].score << "%" << endl << endl;
			}
		}
	}

	auto end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(end - start);

	cout << "Total resumes matched: " << resumeResults.size() << endl;
	cout << "Matching complete." << endl;
	cout << "Total time used for parsing resumes: " << duration.count() << " ms" << endl;

	return 0;
}

