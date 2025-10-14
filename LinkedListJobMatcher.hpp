#ifndef LINKED_LIST_JOB_MATCHER_HPP
#define LINKED_LIST_JOB_MATCHER_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cctype>
#include "CustomContainers.hpp"

class LinkedListJobMatcher {
private:
    // Linked list node structures
    struct JobNode {
        int id;
        std::string description;
        StringArray skills;
        double matchScore;
        JobNode* next;
        
        JobNode() : id(0), matchScore(0.0), next(nullptr) {}
        JobNode(int jobId, const std::string& desc) : id(jobId), description(desc), matchScore(0.0), next(nullptr) {
            extractSkills();
        }
        
        void extractSkills() {
            // Extract skills from job description
            std::string lowerDesc = toLowerCase(description);
            StringArray commonSkills = {
                "sql", "excel", "power bi", "tableau", "python", "java", "javascript",
                "machine learning", "deep learning", "nlp", "pandas", "tensorflow",
                "keras", "mlops", "docker", "git", "agile", "system design",
                "rest apis", "spring boot", "data cleaning", "reporting", "statistics",
                "computer vision", "stakeholder management", "user stories", "product roadmap"
            };
            
            for (const auto& skill : commonSkills) {
                if (lowerDesc.find(skill) != std::string::npos) {
                    skills.push_back(skill);
                }
            }
        }
        
        std::string toLowerCase(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return result;
        }
    };
    
    struct ResumeNode {
        int id;
        std::string description;
        StringArray skills;
        double matchScore;
        ResumeNode* next;
        
        ResumeNode() : id(0), matchScore(0.0), next(nullptr) {}
        ResumeNode(int resumeId, const std::string& desc) : id(resumeId), description(desc), matchScore(0.0), next(nullptr) {
            extractSkills();
        }
        
        void extractSkills() {
            // Extract skills from resume description
            std::string lowerDesc = toLowerCase(description);
            StringArray commonSkills = {
                "sql", "excel", "power bi", "tableau", "python", "java", "javascript",
                "machine learning", "deep learning", "nlp", "pandas", "tensorflow",
                "keras", "mlops", "docker", "git", "agile", "system design",
                "rest apis", "spring boot", "data cleaning", "reporting", "statistics",
                "computer vision", "stakeholder management", "user stories", "product roadmap"
            };
            
            for (const auto& skill : commonSkills) {
                if (lowerDesc.find(skill) != std::string::npos) {
                    skills.push_back(skill);
                }
            }
        }
        
        std::string toLowerCase(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return result;
        }
    };
    
    struct MatchResult {
        int jobId;
        int resumeId;
        double score;
        StringArray commonSkills;
        
        // Default constructor
        MatchResult() : jobId(0), resumeId(0), score(0.0) {}
        
        MatchResult(int jId, int rId, double s) : jobId(jId), resumeId(rId), score(s) {}
    };
    
    // Linked list heads
    JobNode* jobHead;
    ResumeNode* resumeHead;
    int jobCount;
    int resumeCount;
    
    // Performance tracking
    mutable double lastSearchTime;
    mutable double lastSortTime;
    mutable size_t memoryUsed;
    
    // Comprehensive error handling and validation
    struct ErrorHandler {
        enum ErrorType {
            FILE_ERROR,
            MEMORY_ERROR,
            DATA_VALIDATION_ERROR,
            INPUT_VALIDATION_ERROR,
            BOUNDS_ERROR,
            FORMAT_ERROR,
            SYSTEM_ERROR
        };
        
        struct ErrorInfo {
            ErrorType type;
            std::string message;
            std::string context;
            int errorCode;
            bool isRecoverable;
            
            // Default constructor
            ErrorInfo() : type(FILE_ERROR), message(""), context(""), errorCode(0), isRecoverable(true) {}
            
            ErrorInfo(ErrorType t, const std::string& msg, const std::string& ctx = "", 
                     int code = 0, bool recoverable = true)
                : type(t), message(msg), context(ctx), errorCode(code), isRecoverable(recoverable) {}
        };
        
        DynamicArray<ErrorInfo> errorLog;
        bool hasErrors = false;
        bool hasWarnings = false;
        
        void logError(ErrorType type, const std::string& message, const std::string& context = "", int code = 0) {
            errorLog.emplace_back(type, message, context, code);
            hasErrors = true;
        }
        
        void logWarning(const std::string& message, const std::string& context = "") {
            errorLog.emplace_back(DATA_VALIDATION_ERROR, "WARNING: " + message, context, 0, true);
            hasWarnings = true;
        }
        
        void clearErrors() {
            errorLog.clear();
            hasErrors = false;
            hasWarnings = false;
        }
        
        std::string getErrorSummary() const {
            if (errorLog.empty()) return "No errors detected";
            
            std::string summary = "Error Summary:\n";
            for (const auto& error : errorLog) {
                summary += "- " + error.message;
                if (!error.context.empty()) {
                    summary += " (Context: " + error.context + ")";
                }
                summary += "\n";
            }
            return summary;
        }
    };
    
    mutable ErrorHandler errorHandler;
    
public:
    LinkedListJobMatcher();
    ~LinkedListJobMatcher();
    
    // Data loading methods
    bool loadJobsFromCSV(const std::string& filename);
    bool loadResumesFromCSV(const std::string& filename);
    
    // Linked list operations
    void addJob(const JobNode& job);
    void addResume(const ResumeNode& resume);
    JobNode* getJobHead() const { return jobHead; }
    ResumeNode* getResumeHead() const { return resumeHead; }
    int getJobCount() const { return jobCount; }
    int getResumeCount() const { return resumeCount; }
    
    // Job matching algorithm (Weighted Scoring)
    DynamicArray<MatchResult> findMatches(int resumeId, int topK = 10);
    DynamicArray<MatchResult> findMatchesForJob(int jobId, int topK = 10);
    double calculateMatchScore(const JobNode& job, const ResumeNode& resume);
    
    // Sorting algorithms (Merge Sort for linked lists)
    void sortJobsByScore();
    void sortResumesByScore();
    JobNode* mergeSortJobs(JobNode* head);
    ResumeNode* mergeSortResumes(ResumeNode* head);
    JobNode* mergeJobs(JobNode* left, JobNode* right);
    ResumeNode* mergeResumes(ResumeNode* left, ResumeNode* right);
    JobNode* getMiddle(JobNode* head);
    ResumeNode* getMiddle(ResumeNode* head);
    
    // Searching algorithms
    JobNode* linearSearchJob(int jobId);
    ResumeNode* linearSearchResume(int resumeId);
    JobNode* searchJobByScore(double minScore);
    ResumeNode* searchResumeByScore(double minScore);
    
    // Advanced filtering operations
    DynamicArray<JobNode*> searchJobsBySkill(const std::string& skill);
    DynamicArray<ResumeNode*> searchResumesBySkill(const std::string& skill);
    DynamicArray<JobNode*> searchJobsByScoreRange(double minScore, double maxScore);
    DynamicArray<ResumeNode*> searchResumesByScoreRange(double minScore, double maxScore);
    DynamicArray<JobNode*> filterJobsBySkillCount(int minSkills, int maxSkills);
    DynamicArray<ResumeNode*> filterResumesBySkillCount(int minSkills, int maxSkills);
    DynamicArray<JobNode*> filterJobsByDescription(const std::string& keyword);
    DynamicArray<ResumeNode*> filterResumesByDescription(const std::string& keyword);
    
    // Performance analysis
    double getLastSearchTime() const { return lastSearchTime; }
    double getLastSortTime() const { return lastSortTime; }
    size_t getMemoryUsed() const { return memoryUsed; }
    void resetPerformanceCounters();
    
    // Comprehensive error handling and validation
    bool hasErrors() const { return errorHandler.hasErrors; }
    bool hasWarnings() const { return errorHandler.hasWarnings; }
    const ErrorHandler& getErrorHandler() const { return errorHandler; }
    std::string getErrorSummary() const { return errorHandler.getErrorSummary(); }
    void clearErrors() { errorHandler.clearErrors(); }
    bool validateDataIntegrity();
    bool validateInputParameters(int jobId, int resumeId);
    bool validateFileFormat(const std::string& filename);
    bool validateMemoryAllocation();
    void handleError(const std::string& operation, const std::string& error);
    
    // Utility methods
    void displayTopMatches(const DynamicArray<MatchResult>& matches, int count = 5);
    void displayPerformanceStats();
    void validateData();
    void displayJobList();
    void displayResumeList();
    
private:
    void calculateMemoryUsage();
    StringArray findCommonSkills(const JobNode& job, const ResumeNode& resume);
    void clearJobList();
    void clearResumeList();
    JobNode* copyJobNode(const JobNode* node);
    ResumeNode* copyResumeNode(const ResumeNode* node);
    std::string toLowerCase(const std::string& str);
};

#endif // LINKED_LIST_JOB_MATCHER_HPP

