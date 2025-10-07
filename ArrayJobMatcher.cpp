#include "ArrayJobMatcher.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

ArrayJobMatcher::ArrayJobMatcher(int maxJobSize, int maxResumeSize) 
    : jobCount(0), resumeCount(0), maxJobs(maxJobSize), maxResumes(maxResumeSize) {
    jobs = new Job[maxJobs];
    resumes = new Resume[maxResumes];
    calculateMemoryUsage();
}

ArrayJobMatcher::~ArrayJobMatcher() {
    delete[] jobs;
    delete[] resumes;
}

bool ArrayJobMatcher::loadJobsFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int jobId = 1;
    while (std::getline(file, line) && jobCount < maxJobs) {
        // Remove quotes and clean the line
        if (line.front() == '"' && line.back() == '"') {
            line = line.substr(1, line.length() - 2);
        }
        
        if (!line.empty()) {
            jobs[jobCount] = Job(jobId, line);
            jobCount++;
            jobId++;
        }
    }
    
    file.close();
    calculateMemoryUsage();
    return true;
}

bool ArrayJobMatcher::loadResumesFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int resumeId = 1;
    while (std::getline(file, line) && resumeCount < maxResumes) {
        // Remove quotes and clean the line
        if (line.front() == '"' && line.back() == '"') {
            line = line.substr(1, line.length() - 2);
        }
        
        if (!line.empty()) {
            resumes[resumeCount] = Resume(resumeId, line);
            resumeCount++;
            resumeId++;
        }
    }
    
    file.close();
    calculateMemoryUsage();
    return true;
}

void ArrayJobMatcher::addJob(const Job& job) {
    if (jobCount < maxJobs) {
        jobs[jobCount] = job;
        jobCount++;
        calculateMemoryUsage();
    } else {
        std::cerr << "Error: Job array is full" << std::endl;
    }
}

void ArrayJobMatcher::addResume(const Resume& resume) {
    if (resumeCount < maxResumes) {
        resumes[resumeCount] = resume;
        resumeCount++;
        calculateMemoryUsage();
    } else {
        std::cerr << "Error: Resume array is full" << std::endl;
    }
}

ArrayJobMatcher::Job* ArrayJobMatcher::getJob(int index) {
    if (index >= 0 && index < jobCount) {
        return &jobs[index];
    }
    return nullptr;
}

ArrayJobMatcher::Resume* ArrayJobMatcher::getResume(int index) {
    if (index >= 0 && index < resumeCount) {
        return &resumes[index];
    }
    return nullptr;
}

std::vector<ArrayJobMatcher::MatchResult> ArrayJobMatcher::findMatches(int resumeId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<MatchResult> allMatches;
    
    // Find the resume
    Resume* targetResume = nullptr;
    for (int i = 0; i < resumeCount; i++) {
        if (resumes[i].id == resumeId) {
            targetResume = &resumes[i];
            break;
        }
    }
    
    if (!targetResume) {
        std::cerr << "Resume with ID " << resumeId << " not found" << std::endl;
        return allMatches;
    }
    
    // Calculate match scores for all jobs
    for (int i = 0; i < jobCount; i++) {
        double score = calculateAdvancedMatchScore(jobs[i], *targetResume);
        MatchResult match(jobs[i].id, resumeId, score);
        match.commonSkills = findCommonSkills(jobs[i], *targetResume);
        allMatches.push_back(match);
    }
    
    // Sort matches by score (descending)
    std::sort(allMatches.begin(), allMatches.end(), 
              [](const MatchResult& a, const MatchResult& b) {
                  return a.overallScore > b.overallScore;
              });
    
    // Return top K matches
    if (topK > static_cast<int>(allMatches.size())) {
        topK = static_cast<int>(allMatches.size());
    }
    
    std::vector<MatchResult> topMatches(allMatches.begin(), allMatches.begin() + topK);
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return topMatches;
}

std::vector<ArrayJobMatcher::MatchResult> ArrayJobMatcher::findMatchesForJob(int jobId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<MatchResult> allMatches;
    
    // Find the job
    Job* targetJob = nullptr;
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].id == jobId) {
            targetJob = &jobs[i];
            break;
        }
    }
    
    if (!targetJob) {
        std::cerr << "Job with ID " << jobId << " not found" << std::endl;
        return allMatches;
    }
    
    // Calculate match scores for all resumes
    for (int i = 0; i < resumeCount; i++) {
        double score = calculateAdvancedMatchScore(*targetJob, resumes[i]);
        MatchResult match(jobId, resumes[i].id, score);
        match.commonSkills = findCommonSkills(*targetJob, resumes[i]);
        allMatches.push_back(match);
    }
    
    // Sort matches by score (descending)
    std::sort(allMatches.begin(), allMatches.end(), 
              [](const MatchResult& a, const MatchResult& b) {
                  return a.overallScore > b.overallScore;
              });
    
    // Return top K matches
    if (topK > static_cast<int>(allMatches.size())) {
        topK = static_cast<int>(allMatches.size());
    }
    
    std::vector<MatchResult> topMatches(allMatches.begin(), allMatches.begin() + topK);
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return topMatches;
}

double ArrayJobMatcher::calculateAdvancedMatchScore(const Job& job, const Resume& resume) {
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

std::vector<std::string> ArrayJobMatcher::findCommonSkills(const Job& job, const Resume& resume) {
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

void ArrayJobMatcher::sortJobsByScore() {
    auto start = std::chrono::high_resolution_clock::now();
    
    quickSortJobs(0, jobCount - 1);
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSortTime = std::chrono::duration<double, std::milli>(end - start).count();
}

void ArrayJobMatcher::sortResumesByScore() {
    auto start = std::chrono::high_resolution_clock::now();
    
    quickSortResumes(0, resumeCount - 1);
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSortTime = std::chrono::duration<double, std::milli>(end - start).count();
}

void ArrayJobMatcher::quickSortJobs(int low, int high) {
    if (low < high) {
        int pivotIndex = partitionJobs(low, high);
        quickSortJobs(low, pivotIndex - 1);
        quickSortJobs(pivotIndex + 1, high);
    }
}

void ArrayJobMatcher::quickSortResumes(int low, int high) {
    if (low < high) {
        int pivotIndex = partitionResumes(low, high);
        quickSortResumes(low, pivotIndex - 1);
        quickSortResumes(pivotIndex + 1, high);
    }
}

int ArrayJobMatcher::partitionJobs(int low, int high) {
    double pivot = jobs[high].matchScore;
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (jobs[j].matchScore >= pivot) {
            i++;
            std::swap(jobs[i], jobs[j]);
        }
    }
    
    std::swap(jobs[i + 1], jobs[high]);
    return i + 1;
}

int ArrayJobMatcher::partitionResumes(int low, int high) {
    double pivot = resumes[high].matchScore;
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        if (resumes[j].matchScore >= pivot) {
            i++;
            std::swap(resumes[i], resumes[j]);
        }
    }
    
    std::swap(resumes[i + 1], resumes[high]);
    return i + 1;
}

ArrayJobMatcher::Job* ArrayJobMatcher::binarySearchJob(int jobId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // First, sort by ID for binary search
    std::sort(jobs, jobs + jobCount, [](const Job& a, const Job& b) {
        return a.id < b.id;
    });
    
    int left = 0, right = jobCount - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (jobs[mid].id == jobId) {
            auto end = std::chrono::high_resolution_clock::now();
            metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return &jobs[mid];
        }
        
        if (jobs[mid].id < jobId) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

ArrayJobMatcher::Resume* ArrayJobMatcher::binarySearchResume(int resumeId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // First, sort by ID for binary search
    std::sort(resumes, resumes + resumeCount, [](const Resume& a, const Resume& b) {
        return a.id < b.id;
    });
    
    int left = 0, right = resumeCount - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (resumes[mid].id == resumeId) {
            auto end = std::chrono::high_resolution_clock::now();
            metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return &resumes[mid];
        }
        
        if (resumes[mid].id < resumeId) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

ArrayJobMatcher::Job* ArrayJobMatcher::linearSearchJob(int jobId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].id == jobId) {
            auto end = std::chrono::high_resolution_clock::now();
            metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return &jobs[i];
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

ArrayJobMatcher::Resume* ArrayJobMatcher::linearSearchResume(int resumeId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < resumeCount; i++) {
        if (resumes[i].id == resumeId) {
            auto end = std::chrono::high_resolution_clock::now();
            metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return &resumes[i];
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

void ArrayJobMatcher::calculateMemoryUsage() {
    metrics.memoryUsed = sizeof(ArrayJobMatcher) + 
                         (maxJobs * sizeof(Job)) + 
                         (maxResumes * sizeof(Resume));
}

void ArrayJobMatcher::resetPerformanceCounters() {
    metrics.reset();
}

void ArrayJobMatcher::displayTopMatches(const std::vector<MatchResult>& matches, int count) {
    std::cout << "\n=== Top " << count << " Matches ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    
    for (int i = 0; i < std::min(count, (int)matches.size()); i++) {
        const auto& match = matches[i];
        std::cout << "\nMatch " << (i + 1) << ":" << std::endl;
        std::cout << "  Job ID: " << match.jobId << std::endl;
        std::cout << "  Resume ID: " << match.resumeId << std::endl;
        std::cout << "  Score: " << match.overallScore << std::endl;
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

void ArrayJobMatcher::displayPerformanceStats() {
    std::cout << "\n=== Array Implementation Performance Stats ===" << std::endl;
    std::cout << "Memory Used: " << metrics.memoryUsed << " bytes" << std::endl;
    std::cout << "Last Search Time: " << metrics.lastSearchTime << " ms" << std::endl;
    std::cout << "Last Sort Time: " << metrics.lastSortTime << " ms" << std::endl;
    std::cout << "Jobs Loaded: " << jobCount << std::endl;
    std::cout << "Resumes Loaded: " << resumeCount << std::endl;
}

void ArrayJobMatcher::validateData() {
    std::cout << "\n=== Data Validation ===" << std::endl;
    
    int validJobs = 0, validResumes = 0;
    
    for (int i = 0; i < jobCount; i++) {
        if (!jobs[i].description.empty() && jobs[i].id > 0) {
            validJobs++;
        }
    }
    
    for (int i = 0; i < resumeCount; i++) {
        if (!resumes[i].description.empty() && resumes[i].id > 0) {
            validResumes++;
        }
    }
    
    std::cout << "Valid Jobs: " << validJobs << "/" << jobCount << std::endl;
    std::cout << "Valid Resumes: " << validResumes << "/" << resumeCount << std::endl;
    
    if (validJobs != jobCount || validResumes != resumeCount) {
        std::cout << "Warning: Some data entries are invalid!" << std::endl;
    } else {
        std::cout << "All data entries are valid." << std::endl;
    }
}
