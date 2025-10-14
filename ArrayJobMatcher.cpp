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
    // Clear previous errors
    errorHandler.clearErrors();
    
    // Validate file format
    if (!validateFileFormat(filename)) {
        return false;
    }
    
    // Validate memory allocation
    if (!validateMemoryAllocation()) {
        return false;
    }
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorHandler.logError(ErrorHandler::FILE_ERROR, 
            "Could not open file: " + filename, "loadJobsFromCSV");
        std::cerr << "❌ Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Check if file is empty
    if (file.peek() == std::ifstream::traits_type::eof()) {
        errorHandler.logError(ErrorHandler::FILE_ERROR, 
            "File is empty: " + filename, "loadJobsFromCSV");
        std::cerr << "❌ Error: File " << filename << " is empty" << std::endl;
        file.close();
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    int validJobs = 0;
    int invalidJobs = 0;
    
    // Skip header and validate
    if (std::getline(file, line)) {
        lineNumber++;
        if (line.empty()) {
            errorHandler.logWarning("Empty header line detected", "Line " + std::to_string(lineNumber));
        }
    } else {
        errorHandler.logError(ErrorHandler::FILE_ERROR, 
            "Could not read header from file: " + filename, "loadJobsFromCSV");
        file.close();
        return false;
    }
    
    int jobId = 1;
    while (std::getline(file, line) && jobCount < maxJobs) {
        lineNumber++;
        
        // Validate line length
        if (line.length() > 10000) { // Reasonable limit
            errorHandler.logWarning("Line " + std::to_string(lineNumber) + " exceeds maximum length", 
                "Skipping oversized line");
            invalidJobs++;
            continue;
        }
        
        // Remove quotes and clean the line
        std::string cleanedLine = line;
        if (line.length() >= 2 && line.front() == '"' && line.back() == '"') {
            cleanedLine = line.substr(1, line.length() - 2);
        }
        
        // Validate cleaned line
        if (cleanedLine.empty()) {
            errorHandler.logWarning("Empty job description at line " + std::to_string(lineNumber), 
                "Skipping empty entry");
            invalidJobs++;
            continue;
        }
        
        // Check for suspicious content
        if (cleanedLine.length() < 10) {
            errorHandler.logWarning("Very short job description at line " + std::to_string(lineNumber), 
                "Description may be incomplete");
        }
        
        try {
            jobs[jobCount] = Job(jobId, cleanedLine);
            jobCount++;
            jobId++;
            validJobs++;
        } catch (const std::exception& e) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Failed to create job from line " + std::to_string(lineNumber) + ": " + e.what(), 
                "Job ID: " + std::to_string(jobId));
            invalidJobs++;
        }
    }
    
    file.close();
    
    // Validate data integrity after loading
    if (!validateDataIntegrity()) {
        return false;
    }
    
    // Log summary
    if (invalidJobs > 0) {
        errorHandler.logWarning("Loaded " + std::to_string(validJobs) + " valid jobs, " + 
            std::to_string(invalidJobs) + " invalid entries skipped", "Data loading summary");
    }
    
    if (jobCount >= maxJobs) {
        errorHandler.logWarning("Reached maximum job capacity (" + std::to_string(maxJobs) + ")", 
            "Some jobs may not have been loaded");
    }
    
    calculateMemoryUsage();
    
    std::cout << "✅ Successfully loaded " << jobCount << " jobs from " << filename << std::endl;
    if (errorHandler.hasWarnings) {
        std::cout << "⚠️  " << errorHandler.errorLog.size() << " warnings detected" << std::endl;
    }
    
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

ArrayJobMatcher::Job ArrayJobMatcher::getJobById(int id) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].id == id) {
            return jobs[i];
        }
    }
    return Job{}; // Return empty job if not found
}

ArrayJobMatcher::Resume ArrayJobMatcher::getResumeById(int id) {
    for (int i = 0; i < resumeCount; i++) {
        if (resumes[i].id == id) {
            return resumes[i];
        }
    }
    return Resume{}; // Return empty resume if not found
}

DynamicArray<ArrayJobMatcher::MatchResult> ArrayJobMatcher::findMatches(int resumeId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<MatchResult> allMatches;
    
    // Validate input parameters
    if (!validateInputParameters(1, resumeId)) { // Use dummy jobId for validation
        return allMatches;
    }
    
    // Validate topK parameter
    if (topK <= 0) {
        errorHandler.logError(ErrorHandler::INPUT_VALIDATION_ERROR, 
            "Invalid topK parameter: " + std::to_string(topK) + " (must be positive)", "findMatches");
        return allMatches;
    }
    
    if (topK > 10000) { // Reasonable limit to prevent excessive memory usage
        errorHandler.logWarning("Large topK value: " + std::to_string(topK), 
            "Performance may be affected");
    }
    
    // Find the resume
    Resume* targetResume = nullptr;
    for (int i = 0; i < resumeCount; i++) {
        if (resumes[i].id == resumeId) {
            targetResume = &resumes[i];
            break;
        }
    }
    
    if (!targetResume) {
        errorHandler.logError(ErrorHandler::BOUNDS_ERROR, 
            "Resume with ID " + std::to_string(resumeId) + " not found", "findMatches");
        std::cerr << "❌ Resume with ID " << resumeId << " not found" << std::endl;
        return allMatches;
    }
    
    // Validate resume data
    if (targetResume->skills.empty()) {
        errorHandler.logWarning("Resume " + std::to_string(resumeId) + " has no skills", 
            "Matching may not be accurate");
    }
    
    // Calculate match scores for all jobs
    int processedJobs = 0;
    int skippedJobs = 0;
    
    for (int i = 0; i < jobCount; i++) {
        try {
            // Validate job data
            if (jobs[i].skills.empty()) {
                skippedJobs++;
                continue;
            }
            
            double score = calculateAdvancedMatchScore(jobs[i], *targetResume);
            MatchResult match(jobs[i].id, resumeId, score);
            match.commonSkills = findCommonSkills(jobs[i], *targetResume);
            allMatches.push_back(match);
            processedJobs++;
            
        } catch (const std::exception& e) {
            errorHandler.logError(ErrorHandler::SYSTEM_ERROR, 
                "Error processing job " + std::to_string(jobs[i].id) + ": " + e.what(), "findMatches");
            skippedJobs++;
        }
    }
    
    // Log processing summary
    if (skippedJobs > 0) {
        errorHandler.logWarning("Skipped " + std::to_string(skippedJobs) + " jobs during matching", 
            "Processing summary");
    }
    
    if (allMatches.empty()) {
        errorHandler.logWarning("No matches found for resume " + std::to_string(resumeId), 
            "Matching results");
        return allMatches;
    }
    
    // Sort matches by score (descending)
    try {
        std::sort(allMatches.begin(), allMatches.end(), 
                  [](const MatchResult& a, const MatchResult& b) {
                      return a.overallScore > b.overallScore;
                  });
    } catch (const std::exception& e) {
        errorHandler.logError(ErrorHandler::SYSTEM_ERROR, 
            "Error sorting matches: " + std::string(e.what()), "findMatches");
        return allMatches;
    }
    
    // Return top K matches
    if (topK > static_cast<int>(allMatches.size())) {
        topK = static_cast<int>(allMatches.size());
    }
    
    DynamicArray<MatchResult> topMatches;
    for (size_t i = 0; i < static_cast<size_t>(topK) && i < allMatches.size(); ++i) {
        topMatches.push_back(allMatches[i]);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    std::cout << "✅ Found " << topMatches.size() << " matches for resume " << resumeId 
              << " (processed " << processedJobs << " jobs)" << std::endl;
    
    return topMatches;
}

DynamicArray<ArrayJobMatcher::MatchResult> ArrayJobMatcher::findMatchesForJob(int jobId, int topK) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<MatchResult> allMatches;
    
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
    
    DynamicArray<MatchResult> topMatches;
    for (size_t i = 0; i < static_cast<size_t>(topK) && i < allMatches.size(); ++i) {
        topMatches.push_back(allMatches[i]);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return topMatches;
}

double ArrayJobMatcher::calculateAdvancedMatchScore(const Job& job, const Resume& resume) {
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

StringArray ArrayJobMatcher::findCommonSkills(const Job& job, const Resume& resume) {
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

void ArrayJobMatcher::displayTopMatches(const DynamicArray<MatchResult>& matches, int count) {
    std::cout << "\n=== Top " << count << " Matches ===" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    
    for (int i = 0; i < std::min(count, static_cast<int>(matches.size())); i++) {
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

// Advanced filtering operations
DynamicArray<ArrayJobMatcher::Job*> ArrayJobMatcher::searchJobsBySkill(const std::string& skill) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Job*> result;
    result.reserve(jobCount / 10); // Reserve space for estimated matches
    std::string lowerSkill = toLowerCase(skill);
    
    for (int i = 0; i < jobCount; i++) {
        for (const auto& jobSkill : jobs[i].skills) {
            if (toLowerCase(jobSkill) == lowerSkill) {
                result.push_back(&jobs[i]);
                break;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<ArrayJobMatcher::Resume*> ArrayJobMatcher::searchResumesBySkill(const std::string& skill) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Resume*> result;
    result.reserve(resumeCount / 10); // Reserve space for estimated matches
    std::string lowerSkill = toLowerCase(skill);
    
    for (int i = 0; i < resumeCount; i++) {
        for (const auto& resumeSkill : resumes[i].skills) {
            if (toLowerCase(resumeSkill) == lowerSkill) {
                result.push_back(&resumes[i]);
                break;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<ArrayJobMatcher::Job*> ArrayJobMatcher::searchJobsByScoreRange(double minScore, double maxScore) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Job*> result;
    
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].matchScore >= minScore && jobs[i].matchScore <= maxScore) {
            result.push_back(&jobs[i]);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<ArrayJobMatcher::Resume*> ArrayJobMatcher::searchResumesByScoreRange(double minScore, double maxScore) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Resume*> result;
    
    for (int i = 0; i < resumeCount; i++) {
        if (resumes[i].matchScore >= minScore && resumes[i].matchScore <= maxScore) {
            result.push_back(&resumes[i]);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<ArrayJobMatcher::Job*> ArrayJobMatcher::filterJobsBySkillCount(int minSkills, int maxSkills) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Job*> result;
    
    for (int i = 0; i < jobCount; i++) {
        int skillCount = static_cast<int>(jobs[i].skills.size());
        if (skillCount >= minSkills && skillCount <= maxSkills) {
            result.push_back(&jobs[i]);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<ArrayJobMatcher::Resume*> ArrayJobMatcher::filterResumesBySkillCount(int minSkills, int maxSkills) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Resume*> result;
    
    for (int i = 0; i < resumeCount; i++) {
        int skillCount = static_cast<int>(resumes[i].skills.size());
        if (skillCount >= minSkills && skillCount <= maxSkills) {
            result.push_back(&resumes[i]);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<ArrayJobMatcher::Job*> ArrayJobMatcher::filterJobsByDescription(const std::string& keyword) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Job*> result;
    std::string lowerKeyword = toLowerCase(keyword);
    
    for (int i = 0; i < jobCount; i++) {
        if (toLowerCase(jobs[i].description).find(lowerKeyword) != std::string::npos) {
            result.push_back(&jobs[i]);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

DynamicArray<ArrayJobMatcher::Resume*> ArrayJobMatcher::filterResumesByDescription(const std::string& keyword) {
    auto start = std::chrono::high_resolution_clock::now();
    
    DynamicArray<Resume*> result;
    std::string lowerKeyword = toLowerCase(keyword);
    
    for (int i = 0; i < resumeCount; i++) {
        if (toLowerCase(resumes[i].description).find(lowerKeyword) != std::string::npos) {
            result.push_back(&resumes[i]);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    return result;
}

std::string ArrayJobMatcher::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Comprehensive error handling and validation methods
bool ArrayJobMatcher::validateFileFormat(const std::string& filename) {
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

bool ArrayJobMatcher::validateMemoryAllocation() {
    if (jobs == nullptr || resumes == nullptr) {
        errorHandler.logError(ErrorHandler::MEMORY_ERROR, 
            "Memory allocation failed for jobs or resumes arrays", "validateMemoryAllocation");
        return false;
    }
    
    if (maxJobs <= 0 || maxResumes <= 0) {
        errorHandler.logError(ErrorHandler::MEMORY_ERROR, 
            "Invalid maximum size parameters: maxJobs=" + std::to_string(maxJobs) + 
            ", maxResumes=" + std::to_string(maxResumes), "validateMemoryAllocation");
        return false;
    }
    
    return true;
}

bool ArrayJobMatcher::validateDataIntegrity() {
    bool isValid = true;
    
    // Validate job data
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].id <= 0) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Invalid job ID at index " + std::to_string(i), "validateDataIntegrity");
            isValid = false;
        }
        
        if (jobs[i].description.empty()) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Empty job description at index " + std::to_string(i), "validateDataIntegrity");
            isValid = false;
        }
        
        if (jobs[i].skills.empty()) {
            errorHandler.logWarning("Job " + std::to_string(jobs[i].id) + " has no skills extracted", 
                "Data validation");
        }
    }
    
    // Validate resume data
    for (int i = 0; i < resumeCount; i++) {
        if (resumes[i].id <= 0) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Invalid resume ID at index " + std::to_string(i), "validateDataIntegrity");
            isValid = false;
        }
        
        if (resumes[i].description.empty()) {
            errorHandler.logError(ErrorHandler::DATA_VALIDATION_ERROR, 
                "Empty resume description at index " + std::to_string(i), "validateDataIntegrity");
            isValid = false;
        }
        
        if (resumes[i].skills.empty()) {
            errorHandler.logWarning("Resume " + std::to_string(resumes[i].id) + " has no skills extracted", 
                "Data validation");
        }
    }
    
    return isValid;
}

bool ArrayJobMatcher::validateInputParameters(int jobId, int resumeId) {
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
    
    if (jobId > jobCount) {
        errorHandler.logError(ErrorHandler::BOUNDS_ERROR, 
            "Job ID " + std::to_string(jobId) + " exceeds available jobs (" + std::to_string(jobCount) + ")", 
            "validateInputParameters");
        isValid = false;
    }
    
    if (resumeId > resumeCount) {
        errorHandler.logError(ErrorHandler::BOUNDS_ERROR, 
            "Resume ID " + std::to_string(resumeId) + " exceeds available resumes (" + std::to_string(resumeCount) + ")", 
            "validateInputParameters");
        isValid = false;
    }
    
    return isValid;
}

void ArrayJobMatcher::handleError(const std::string& operation, const std::string& error) {
    errorHandler.logError(ErrorHandler::SYSTEM_ERROR, 
        "Error in " + operation + ": " + error, "handleError");
    std::cerr << "❌ Error in " << operation << ": " << error << std::endl;
}

// Additional missing method implementations
ArrayJobMatcher::Job* ArrayJobMatcher::interpolationSearchJob(int jobId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // First, sort by ID for interpolation search
    std::sort(jobs, jobs + jobCount, [](const Job& a, const Job& b) {
        return a.id < b.id;
    });
    
    int left = 0;
    int right = jobCount - 1;
    
    while (left <= right && jobId >= jobs[left].id && jobId <= jobs[right].id) {
        if (left == right) {
            if (jobs[left].id == jobId) {
                auto end = std::chrono::high_resolution_clock::now();
                metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
                return &jobs[left];
            }
            break;
        }
        
        // Interpolation formula
        int pos = left + ((jobId - jobs[left].id) * (right - left)) / (jobs[right].id - jobs[left].id);
        
        if (jobs[pos].id == jobId) {
            auto end = std::chrono::high_resolution_clock::now();
            metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return &jobs[pos];
        }
        
        if (jobs[pos].id < jobId) {
            left = pos + 1;
        } else {
            right = pos - 1;
        }
        
        metrics.comparisons++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

ArrayJobMatcher::Resume* ArrayJobMatcher::interpolationSearchResume(int resumeId) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // First, sort by ID for interpolation search
    std::sort(resumes, resumes + resumeCount, [](const Resume& a, const Resume& b) {
        return a.id < b.id;
    });
    
    int left = 0;
    int right = resumeCount - 1;
    
    while (left <= right && resumeId >= resumes[left].id && resumeId <= resumes[right].id) {
        if (left == right) {
            if (resumes[left].id == resumeId) {
                auto end = std::chrono::high_resolution_clock::now();
                metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
                return &resumes[left];
            }
            break;
        }
        
        // Interpolation formula
        int pos = left + ((resumeId - resumes[left].id) * (right - left)) / (resumes[right].id - resumes[left].id);
        
        if (resumes[pos].id == resumeId) {
            auto end = std::chrono::high_resolution_clock::now();
            metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
            return &resumes[pos];
        }
        
        if (resumes[pos].id < resumeId) {
            left = pos + 1;
        } else {
            right = pos - 1;
        }
        
        metrics.comparisons++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    metrics.lastSearchTime = std::chrono::duration<double, std::milli>(end - start).count();
    return nullptr;
}

int ArrayJobMatcher::partitionJobs3Way(int low, int high) {
    if (low >= high) return low;
    
    double pivot = jobs[low].matchScore;
    int lt = low;      // jobs[low..lt-1] < pivot
    int gt = high;     // jobs[gt+1..high] > pivot
    int i = low + 1;   // jobs[lt..i-1] == pivot
    
    while (i <= gt) {
        if (jobs[i].matchScore < pivot) {
            std::swap(jobs[lt++], jobs[i++]);
            metrics.swaps++;
        } else if (jobs[i].matchScore > pivot) {
            std::swap(jobs[i], jobs[gt--]);
            metrics.swaps++;
        } else {
            i++;
        }
        metrics.comparisons++;
    }
    
    return lt;
}

int ArrayJobMatcher::partitionResumes3Way(int low, int high) {
    if (low >= high) return low;
    
    double pivot = resumes[low].matchScore;
    int lt = low;      // resumes[low..lt-1] < pivot
    int gt = high;     // resumes[gt+1..high] > pivot
    int i = low + 1;   // resumes[lt..i-1] == pivot
    
    while (i <= gt) {
        if (resumes[i].matchScore < pivot) {
            std::swap(resumes[lt++], resumes[i++]);
            metrics.swaps++;
        } else if (resumes[i].matchScore > pivot) {
            std::swap(resumes[i], resumes[gt--]);
            metrics.swaps++;
        } else {
            i++;
        }
        metrics.comparisons++;
    }
    
    return lt;
}

void ArrayJobMatcher::introsortJobs(int low, int high, int depthLimit) {
    if (low < high) {
        if (high - low < 16) {
            // Use insertion sort for small arrays
            insertionSortJobs(low, high);
        } else if (depthLimit == 0) {
            // Use heap sort as fallback
            std::make_heap(jobs + low, jobs + high + 1);
            std::sort_heap(jobs + low, jobs + high + 1);
        } else {
            // Use quicksort with 3-way partitioning
            int pivotIndex = partitionJobs3Way(low, high);
            introsortJobs(low, pivotIndex - 1, depthLimit - 1);
            introsortJobs(pivotIndex + 1, high, depthLimit - 1);
        }
    }
}

void ArrayJobMatcher::introsortResumes(int low, int high, int depthLimit) {
    if (low < high) {
        if (high - low < 16) {
            // Use insertion sort for small arrays
            insertionSortResumes(low, high);
        } else if (depthLimit == 0) {
            // Use heap sort as fallback
            std::make_heap(resumes + low, resumes + high + 1);
            std::sort_heap(resumes + low, resumes + high + 1);
        } else {
            // Use quicksort with 3-way partitioning
            int pivotIndex = partitionResumes3Way(low, high);
            introsortResumes(low, pivotIndex - 1, depthLimit - 1);
            introsortResumes(pivotIndex + 1, high, depthLimit - 1);
        }
    }
}

void ArrayJobMatcher::insertionSortJobs(int low, int high) {
    for (int i = low + 1; i <= high; i++) {
        Job key = jobs[i];
        int j = i - 1;
        
        while (j >= low && jobs[j].matchScore < key.matchScore) {
            jobs[j + 1] = jobs[j];
            j--;
            metrics.swaps++;
        }
        jobs[j + 1] = key;
        metrics.comparisons++;
    }
}

void ArrayJobMatcher::insertionSortResumes(int low, int high) {
    for (int i = low + 1; i <= high; i++) {
        Resume key = resumes[i];
        int j = i - 1;
        
        while (j >= low && resumes[j].matchScore < key.matchScore) {
            resumes[j + 1] = resumes[j];
            j--;
            metrics.swaps++;
        }
        resumes[j + 1] = key;
        metrics.comparisons++;
    }
}

int ArrayJobMatcher::medianOfThreeJobs(int low, int mid, int high) {
    if (jobs[low].matchScore > jobs[mid].matchScore) {
        if (jobs[mid].matchScore > jobs[high].matchScore) return mid;
        else if (jobs[low].matchScore > jobs[high].matchScore) return high;
        else return low;
    } else {
        if (jobs[low].matchScore > jobs[high].matchScore) return low;
        else if (jobs[mid].matchScore > jobs[high].matchScore) return high;
        else return mid;
    }
}

int ArrayJobMatcher::medianOfThreeResumes(int low, int mid, int high) {
    if (resumes[low].matchScore > resumes[mid].matchScore) {
        if (resumes[mid].matchScore > resumes[high].matchScore) return mid;
        else if (resumes[low].matchScore > resumes[high].matchScore) return high;
        else return low;
    } else {
        if (resumes[low].matchScore > resumes[high].matchScore) return low;
        else if (resumes[mid].matchScore > resumes[high].matchScore) return high;
        else return mid;
    }
}
