#include "LinkedListJobMatcher.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

LinkedListJobMatcher::LinkedListJobMatcher() 
    : jobHead(nullptr), resumeHead(nullptr), jobCount(0), resumeCount(0),
      lastSearchTime(0.0), lastSortTime(0.0), memoryUsed(0) {
    calculateMemoryUsage();
}

LinkedListJobMatcher::~LinkedListJobMatcher() {
    clearJobList();
    clearResumeList();
}

bool LinkedListJobMatcher::loadJobsFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int jobId = 1;
    while (std::getline(file, line)) {
        // Remove quotes and clean the line
        if (line.front() == '"' && line.back() == '"') {
            line = line.substr(1, line.length() - 2);
        }
        
        if (!line.empty()) {
            JobNode* newNode = new JobNode(jobId, line);
            addJob(*newNode);
            delete newNode; // addJob creates a copy
            jobId++;
        }
    }
    
    file.close();
    calculateMemoryUsage();
    return true;
}

bool LinkedListJobMatcher::loadResumesFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int resumeId = 1;
    while (std::getline(file, line)) {
        // Remove quotes and clean the line
        if (line.front() == '"' && line.back() == '"') {
            line = line.substr(1, line.length() - 2);
        }
        
        if (!line.empty()) {
            ResumeNode* newNode = new ResumeNode(resumeId, line);
            addResume(*newNode);
            delete newNode; // addResume creates a copy
            resumeId++;
        }
    }
    
    file.close();
    calculateMemoryUsage();
    return true;
}

void LinkedListJobMatcher::addJob(const JobNode& job) {
    JobNode* newNode = new JobNode(job);
    newNode->next = jobHead;
    jobHead = newNode;
    jobCount++;
    calculateMemoryUsage();
}

void LinkedListJobMatcher::addResume(const ResumeNode& resume) {
    ResumeNode* newNode = new ResumeNode(resume);
    newNode->next = resumeHead;
    resumeHead = newNode;
    resumeCount++;
    calculateMemoryUsage();
}

std::vector<LinkedListJobMatcher::MatchResult> LinkedListJobMatcher::findMatches(int resumeId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<MatchResult> allMatches;
    
    // Find the resume
    ResumeNode* targetResume = linearSearchResume(resumeId);
    if (!targetResume) {
        std::cerr << "Resume with ID " << resumeId << " not found" << std::endl;
        return allMatches;
    }
    
    // Calculate match scores for all jobs
    JobNode* current = jobHead;
    while (current != nullptr) {
        double score = calculateMatchScore(*current, *targetResume);
        MatchResult match(current->id, resumeId, score);
        match.commonSkills = findCommonSkills(*current, *targetResume);
        allMatches.push_back(match);
        current = current->next;
    }
    
    // Sort matches by score (descending)
    std::sort(allMatches.begin(), allMatches.end(), 
              [](const MatchResult& a, const MatchResult& b) {
                  return a.score > b.score;
              });
    
    // Return top K matches
    if (topK > static_cast<int>(allMatches.size())) {
        topK = static_cast<int>(allMatches.size());
    }
    
    std::vector<MatchResult> topMatches(allMatches.begin(), allMatches.begin() + topK);
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return topMatches;
}

std::vector<LinkedListJobMatcher::MatchResult> LinkedListJobMatcher::findMatchesForJob(int jobId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<MatchResult> allMatches;
    
    // Find the job
    JobNode* targetJob = linearSearchJob(jobId);
    if (!targetJob) {
        std::cerr << "Job with ID " << jobId << " not found" << std::endl;
        return allMatches;
    }
    
    // Calculate match scores for all resumes
    ResumeNode* current = resumeHead;
    while (current != nullptr) {
        double score = calculateMatchScore(*targetJob, *current);
        MatchResult match(jobId, current->id, score);
        match.commonSkills = findCommonSkills(*targetJob, *current);
        allMatches.push_back(match);
        current = current->next;
    }
    
    // Sort matches by score (descending)
    std::sort(allMatches.begin(), allMatches.end(), 
              [](const MatchResult& a, const MatchResult& b) {
                  return a.score > b.score;
              });
    
    // Return top K matches
    if (topK > static_cast<int>(allMatches.size())) {
        topK = static_cast<int>(allMatches.size());
    }
    
    std::vector<MatchResult> topMatches(allMatches.begin(), allMatches.begin() + topK);
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return topMatches;
}

double LinkedListJobMatcher::calculateMatchScore(const JobNode& job, const ResumeNode& resume) {
    if (job.skills.empty() || resume.skills.empty()) {
        return 0.0;
    }
    
    // Find common skills
    std::vector<std::string> commonSkills = findCommonSkills(job, resume);
    
    // Calculate weighted score
    double skillMatchRatio = (double)commonSkills.size() / job.skills.size();
    double coverageRatio = (double)commonSkills.size() / resume.skills.size();
    
    // Weighted scoring: 70% skill match, 30% coverage
    double score = (skillMatchRatio * 0.7) + (coverageRatio * 0.3);
    
    // Bonus for exact skill matches
    double exactMatches = 0;
    for (const auto& skill : commonSkills) {
        if (std::find(job.skills.begin(), job.skills.end(), skill) != job.skills.end() &&
            std::find(resume.skills.begin(), resume.skills.end(), skill) != resume.skills.end()) {
            exactMatches++;
        }
    }
    
    if (exactMatches > 0) {
        score += (exactMatches / job.skills.size()) * 0.2; // 20% bonus
    }
    
    return std::min(score, 1.0); // Cap at 1.0
}

std::vector<std::string> LinkedListJobMatcher::findCommonSkills(const JobNode& job, const ResumeNode& resume) {
    std::vector<std::string> commonSkills;
    
    for (const auto& jobSkill : job.skills) {
        for (const auto& resumeSkill : resume.skills) {
            if (jobSkill == resumeSkill) {
                if (std::find(commonSkills.begin(), commonSkills.end(), jobSkill) == commonSkills.end()) {
                    commonSkills.push_back(jobSkill);
                }
            }
        }
    }
    
    return commonSkills;
}

void LinkedListJobMatcher::sortJobsByScore() {
    auto start = std::chrono::high_resolution_clock::now();
    
    jobHead = mergeSortJobs(jobHead);
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSortTime = std::chrono::duration<double, std::milli>(end - start).count();
}

void LinkedListJobMatcher::sortResumesByScore() {
    auto start = std::chrono::high_resolution_clock::now();
    
    resumeHead = mergeSortResumes(resumeHead);
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSortTime = std::chrono::duration<double, std::milli>(end - start).count();
}

LinkedListJobMatcher::JobNode* LinkedListJobMatcher::mergeSortJobs(JobNode* head) {
    if (!head || !head->next) {
        return head;
    }
    
    JobNode* middle = getMiddle(head);
    JobNode* nextToMiddle = middle->next;
    middle->next = nullptr;
    
    JobNode* left = mergeSortJobs(head);
    JobNode* right = mergeSortJobs(nextToMiddle);
    
    return mergeJobs(left, right);
}

LinkedListJobMatcher::ResumeNode* LinkedListJobMatcher::mergeSortResumes(ResumeNode* head) {
    if (!head || !head->next) {
        return head;
    }
    
    ResumeNode* middle = getMiddle(head);
    ResumeNode* nextToMiddle = middle->next;
    middle->next = nullptr;
    
    ResumeNode* left = mergeSortResumes(head);
    ResumeNode* right = mergeSortResumes(nextToMiddle);
    
    return mergeResumes(left, right);
}

LinkedListJobMatcher::JobNode* LinkedListJobMatcher::mergeJobs(JobNode* left, JobNode* right) {
    JobNode* result = nullptr;
    
    if (!left) return right;
    if (!right) return left;
    
    if (left->matchScore >= right->matchScore) {
        result = left;
        result->next = mergeJobs(left->next, right);
    } else {
        result = right;
        result->next = mergeJobs(left, right->next);
    }
    
    return result;
}

LinkedListJobMatcher::ResumeNode* LinkedListJobMatcher::mergeResumes(ResumeNode* left, ResumeNode* right) {
    ResumeNode* result = nullptr;
    
    if (!left) return right;
    if (!right) return left;
    
    if (left->matchScore >= right->matchScore) {
        result = left;
        result->next = mergeResumes(left->next, right);
    } else {
        result = right;
        result->next = mergeResumes(left, right->next);
    }
    
    return result;
}

LinkedListJobMatcher::JobNode* LinkedListJobMatcher::getMiddle(JobNode* head) {
    if (!head) return head;
    
    JobNode* slow = head;
    JobNode* fast = head->next;
    
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    
    return slow;
}

LinkedListJobMatcher::ResumeNode* LinkedListJobMatcher::getMiddle(ResumeNode* head) {
    if (!head) return head;
    
    ResumeNode* slow = head;
    ResumeNode* fast = head->next;
    
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    
    return slow;
}

LinkedListJobMatcher::JobNode* LinkedListJobMatcher::linearSearchJob(int jobId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        if (current->id == jobId) {
            auto end = std::chrono::high_resolution_clock::now();
            lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return current;
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

LinkedListJobMatcher::ResumeNode* LinkedListJobMatcher::linearSearchResume(int resumeId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    ResumeNode* current = resumeHead;
    while (current != nullptr) {
        if (current->id == resumeId) {
            auto end = std::chrono::high_resolution_clock::now();
            lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return current;
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

LinkedListJobMatcher::JobNode* LinkedListJobMatcher::searchJobByScore(double minScore) {
    auto start = std::chrono::high_resolution_clock::now();
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        if (current->matchScore >= minScore) {
            auto end = std::chrono::high_resolution_clock::now();
            lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return current;
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

LinkedListJobMatcher::ResumeNode* LinkedListJobMatcher::searchResumeByScore(double minScore) {
    auto start = std::chrono::high_resolution_clock::now();
    
    ResumeNode* current = resumeHead;
    while (current != nullptr) {
        if (current->matchScore >= minScore) {
            auto end = std::chrono::high_resolution_clock::now();
            lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return current;
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

void LinkedListJobMatcher::calculateMemoryUsage() {
    memoryUsed = sizeof(LinkedListJobMatcher);
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        memoryUsed += sizeof(JobNode);
        current = current->next;
    }
    
    ResumeNode* resumeCurrent = resumeHead;
    while (resumeCurrent != nullptr) {
        memoryUsed += sizeof(ResumeNode);
        resumeCurrent = resumeCurrent->next;
    }
}

void LinkedListJobMatcher::resetPerformanceCounters() {
    lastSearchTime = 0.0;
    lastSortTime = 0.0;
}

void LinkedListJobMatcher::clearJobList() {
    while (jobHead != nullptr) {
        JobNode* temp = jobHead;
        jobHead = jobHead->next;
        delete temp;
    }
    jobCount = 0;
}

void LinkedListJobMatcher::clearResumeList() {
    while (resumeHead != nullptr) {
        ResumeNode* temp = resumeHead;
        resumeHead = resumeHead->next;
        delete temp;
    }
    resumeCount = 0;
}

LinkedListJobMatcher::JobNode* LinkedListJobMatcher::copyJobNode(const JobNode* node) {
    if (!node) return nullptr;
    
    JobNode* newNode = new JobNode();
    newNode->id = node->id;
    newNode->description = node->description;
    newNode->skills = node->skills;
    newNode->matchScore = node->matchScore;
    newNode->next = nullptr;
    
    return newNode;
}

LinkedListJobMatcher::ResumeNode* LinkedListJobMatcher::copyResumeNode(const ResumeNode* node) {
    if (!node) return nullptr;
    
    ResumeNode* newNode = new ResumeNode();
    newNode->id = node->id;
    newNode->description = node->description;
    newNode->skills = node->skills;
    newNode->matchScore = node->matchScore;
    newNode->next = nullptr;
    
    return newNode;
}

void LinkedListJobMatcher::displayTopMatches(const std::vector<MatchResult>& matches, int count) {
    std::cout << "\n=== Top " << count << " Matches (Linked List) ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    
    for (int i = 0; i < std::min(count, (int)matches.size()); i++) {
        const auto& match = matches[i];
        std::cout << "\nMatch " << (i + 1) << ":" << std::endl;
        std::cout << "  Job ID: " << match.jobId << std::endl;
        std::cout << "  Resume ID: " << match.resumeId << std::endl;
        std::cout << "  Score: " << match.score << std::endl;
        std::cout << "  Common Skills: ";
        
        for (size_t j = 0; j < match.commonSkills.size(); j++) {
            std::cout << match.commonSkills[j];
            if (j < match.commonSkills.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
}

void LinkedListJobMatcher::displayPerformanceStats() {
    std::cout << "\n=== Linked List Implementation Performance Stats ===" << std::endl;
    std::cout << "Memory Used: " << memoryUsed << " bytes" << std::endl;
    std::cout << "Last Search Time: " << lastSearchTime << " ms" << std::endl;
    std::cout << "Last Sort Time: " << lastSortTime << " ms" << std::endl;
    std::cout << "Jobs Loaded: " << jobCount << std::endl;
    std::cout << "Resumes Loaded: " << resumeCount << std::endl;
}

void LinkedListJobMatcher::validateData() {
    std::cout << "\n=== Data Validation (Linked List) ===" << std::endl;
    
    int validJobs = 0, validResumes = 0;
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        if (!current->description.empty() && current->id > 0) {
            validJobs++;
        }
        current = current->next;
    }
    
    ResumeNode* resumeCurrent = resumeHead;
    while (resumeCurrent != nullptr) {
        if (!resumeCurrent->description.empty() && resumeCurrent->id > 0) {
            validResumes++;
        }
        resumeCurrent = resumeCurrent->next;
    }
    
    std::cout << "Valid Jobs: " << validJobs << "/" << jobCount << std::endl;
    std::cout << "Valid Resumes: " << validResumes << "/" << resumeCount << std::endl;
    
    if (validJobs != jobCount || validResumes != resumeCount) {
        std::cout << "Warning: Some data entries are invalid!" << std::endl;
    } else {
        std::cout << "All data entries are valid." << std::endl;
    }
}

void LinkedListJobMatcher::displayJobList() {
    std::cout << "\n=== Job List (First 10) ===" << std::endl;
    JobNode* current = jobHead;
    int count = 0;
    while (current != nullptr && count < 10) {
        std::cout << "Job " << current->id << ": " << current->description.substr(0, 50) << "..." << std::endl;
        current = current->next;
        count++;
    }
}

void LinkedListJobMatcher::displayResumeList() {
    std::cout << "\n=== Resume List (First 10) ===" << std::endl;
    ResumeNode* current = resumeHead;
    int count = 0;
    while (current != nullptr && count < 10) {
        std::cout << "Resume " << current->id << ": " << current->description.substr(0, 50) << "..." << std::endl;
        current = current->next;
        count++;
    }
}
