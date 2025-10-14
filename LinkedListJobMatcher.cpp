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

DynamicArray<LinkedListJobMatcher::MatchResult> LinkedListJobMatcher::findMatches(int resumeId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<MatchResult> allMatches;
    
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
    
    DynamicArray<MatchResult> topMatches;
    for (size_t i = 0; i < static_cast<size_t>(topK) && i < allMatches.size(); ++i) {
        topMatches.push_back(allMatches[i]);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return topMatches;
}

DynamicArray<LinkedListJobMatcher::MatchResult> LinkedListJobMatcher::findMatchesForJob(int jobId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<MatchResult> allMatches;
    
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
    
    DynamicArray<MatchResult> topMatches;
    for (size_t i = 0; i < static_cast<size_t>(topK) && i < allMatches.size(); ++i) {
        topMatches.push_back(allMatches[i]);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return topMatches;
}

double LinkedListJobMatcher::calculateMatchScore(const JobNode& job, const ResumeNode& resume) {
    if (job.skills.empty() || resume.skills.empty()) {
        return 0.0;
    }
    
    // Find common skills
    StringArray commonSkills = findCommonSkills(job, resume);
    
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

StringArray LinkedListJobMatcher::findCommonSkills(const JobNode& job, const ResumeNode& resume) {
    StringArray commonSkills;
    
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

void LinkedListJobMatcher::displayTopMatches(const DynamicArray<MatchResult>& matches, int count) {
    std::cout << "\n=== Top " << count << " Matches (Linked List) ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    
    for (int i = 0; i < std::min(count, static_cast<int>(matches.size())); i++) {
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

// Advanced filtering operations for Linked List
DynamicArray<LinkedListJobMatcher::JobNode*> LinkedListJobMatcher::searchJobsBySkill(const std::string& skill) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<JobNode*> result;
    result.reserve(jobCount / 10); // Reserve space for estimated matches
    std::string lowerSkill = toLowerCase(skill);
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        for (const auto& jobSkill : current->skills) {
            if (toLowerCase(jobSkill) == lowerSkill) {
                result.push_back(current);
                break;
            }
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<LinkedListJobMatcher::ResumeNode*> LinkedListJobMatcher::searchResumesBySkill(const std::string& skill) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<ResumeNode*> result;
    std::string lowerSkill = toLowerCase(skill);
    
    ResumeNode* current = resumeHead;
    while (current != nullptr) {
        for (const auto& resumeSkill : current->skills) {
            if (toLowerCase(resumeSkill) == lowerSkill) {
                result.push_back(current);
                break;
            }
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<LinkedListJobMatcher::JobNode*> LinkedListJobMatcher::searchJobsByScoreRange(double minScore, double maxScore) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<JobNode*> result;
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        if (current->matchScore >= minScore && current->matchScore <= maxScore) {
            result.push_back(current);
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<LinkedListJobMatcher::ResumeNode*> LinkedListJobMatcher::searchResumesByScoreRange(double minScore, double maxScore) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<ResumeNode*> result;
    
    ResumeNode* current = resumeHead;
    while (current != nullptr) {
        if (current->matchScore >= minScore && current->matchScore <= maxScore) {
            result.push_back(current);
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<LinkedListJobMatcher::JobNode*> LinkedListJobMatcher::filterJobsBySkillCount(int minSkills, int maxSkills) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<JobNode*> result;
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        int skillCount = static_cast<int>(current->skills.size());
        if (skillCount >= minSkills && skillCount <= maxSkills) {
            result.push_back(current);
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<LinkedListJobMatcher::ResumeNode*> LinkedListJobMatcher::filterResumesBySkillCount(int minSkills, int maxSkills) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<ResumeNode*> result;
    
    ResumeNode* current = resumeHead;
    while (current != nullptr) {
        int skillCount = static_cast<int>(current->skills.size());
        if (skillCount >= minSkills && skillCount <= maxSkills) {
            result.push_back(current);
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<LinkedListJobMatcher::JobNode*> LinkedListJobMatcher::filterJobsByDescription(const std::string& keyword) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<JobNode*> result;
    std::string lowerKeyword = toLowerCase(keyword);
    
    JobNode* current = jobHead;
    while (current != nullptr) {
        if (toLowerCase(current->description).find(lowerKeyword) != std::string::npos) {
            result.push_back(current);
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<LinkedListJobMatcher::ResumeNode*> LinkedListJobMatcher::filterResumesByDescription(const std::string& keyword) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<ResumeNode*> result;
    std::string lowerKeyword = toLowerCase(keyword);
    
    ResumeNode* current = resumeHead;
    while (current != nullptr) {
        if (toLowerCase(current->description).find(lowerKeyword) != std::string::npos) {
            result.push_back(current);
        }
        current = current->next;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

std::string LinkedListJobMatcher::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Comprehensive error handling and validation methods
bool LinkedListJobMatcher::validateFileFormat(const std::string& filename) {
    // Check file extension
    if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".csv") {
        errorHandler.logError(ErrorHandler::FORMAT_ERROR, 
            "Invalid file format: " + filename + " (expected .csv)", "validateFileFormat");
        return false;
    }
    
    // Check if file exists
    std::ifstream testFile(filename);
    if (!testFile.good()) {
        errorHandler.logError(ErrorHandler::FILE_ERROR, 
            "File does not exist or is not accessible: " + filename, "validateFileFormat");
        return false;
    }
    testFile.close();
    
    return true;
}

bool LinkedListJobMatcher::validateMemoryAllocation() {
    // For linked lists, we don't pre-allocate memory, so this is always valid
    return true;
}

bool LinkedListJobMatcher::validateDataIntegrity() {
    bool isValid = true;
    
    // Validate job data
    JobNode* current = jobHead;
    int jobIndex = 0;
    while (current != nullptr) {
        if (current->id <= 0) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Invalid job ID at index " + std::to_string(jobIndex), "validateDataIntegrity");
            isValid = false;
        }
        
        if (current->description.empty()) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Empty job description at index " + std::to_string(jobIndex), "validateDataIntegrity");
            isValid = false;
        }
        
        if (current->skills.empty()) {
            errorHandler.logWarning("Job " + std::to_string(current->id) + " has no skills extracted", 
                "Data validation");
        }
        
        current = current->next;
        jobIndex++;
    }
    
    // Validate resume data
    ResumeNode* resumeCurrent = resumeHead;
    int resumeIndex = 0;
    while (resumeCurrent != nullptr) {
        if (resumeCurrent->id <= 0) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Invalid resume ID at index " + std::to_string(resumeIndex), "validateDataIntegrity");
            isValid = false;
        }
        
        if (resumeCurrent->description.empty()) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Empty resume description at index " + std::to_string(resumeIndex), "validateDataIntegrity");
            isValid = false;
        }
        
        if (resumeCurrent->skills.empty()) {
            errorHandler.logWarning("Resume " + std::to_string(resumeCurrent->id) + " has no skills extracted", 
                "Data validation");
        }
        
        resumeCurrent = resumeCurrent->next;
        resumeIndex++;
    }
    
    return isValid;
}

bool LinkedListJobMatcher::validateInputParameters(int jobId, int resumeId) {
    bool isValid = true;
    
    if (jobId <= 0) {
        errorHandler.logError(ErrorHandler::INPUT_VALIDATION_ERROR, 
            "Invalid job ID: " + std::to_string(jobId) + " (must be positive)", "validateInputParameters");
        isValid = false;
    }
    
    if (resumeId <= 0) {
        errorHandler.logError(ErrorHandler::INPUT_VALIDATION_ERROR, 
            "Invalid resume ID: " + std::to_string(resumeId) + " (must be positive)", "validateInputParameters");
        isValid = false;
    }
    
    // Check if job exists
    bool jobExists = false;
    JobNode* current = jobHead;
    while (current != nullptr) {
        if (current->id == jobId) {
            jobExists = true;
            break;
        }
        current = current->next;
    }
    
    if (!jobExists && jobId > 0) {
        errorHandler.logError(ErrorHandler::BOUNDS_ERROR, 
            "Job ID " + std::to_string(jobId) + " not found in linked list", "validateInputParameters");
        isValid = false;
    }
    
    // Check if resume exists
    bool resumeExists = false;
    ResumeNode* resumeCurrent = resumeHead;
    while (resumeCurrent != nullptr) {
        if (resumeCurrent->id == resumeId) {
            resumeExists = true;
            break;
        }
        resumeCurrent = resumeCurrent->next;
    }
    
    if (!resumeExists && resumeId > 0) {
        errorHandler.logError(ErrorHandler::BOUNDS_ERROR, 
            "Resume ID " + std::to_string(resumeId) + " not found in linked list", "validateInputParameters");
        isValid = false;
    }
    
    return isValid;
}

void LinkedListJobMatcher::handleError(const std::string& operation, const std::string& error) {
    errorHandler.logError(ErrorHandler::SYSTEM_ERROR, 
        "Error in " + operation + ": " + error, "handleError");
    std::cerr << "❌ Error in " << operation << ": " << error << std::endl;
}
