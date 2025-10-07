#ifndef LINKED_LIST_JOB_MATCHER_HPP
#define LINKED_LIST_JOB_MATCHER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cctype>

class LinkedListJobMatcher {
private:
    // Linked list node structures
    struct JobNode {
        int id;
        std::string description;
        std::vector<std::string> skills;
        double matchScore;
        JobNode* next;
        
        JobNode() : id(0), matchScore(0.0), next(nullptr) {}
        JobNode(int jobId, const std::string& desc) : id(jobId), description(desc), matchScore(0.0), next(nullptr) {
            extractSkills();
        }
        
        void extractSkills() {
            // Extract skills from job description
            std::string lowerDesc = toLowerCase(description);
            std::vector<std::string> commonSkills = {
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
        std::vector<std::string> skills;
        double matchScore;
        ResumeNode* next;
        
        ResumeNode() : id(0), matchScore(0.0), next(nullptr) {}
        ResumeNode(int resumeId, const std::string& desc) : id(resumeId), description(desc), matchScore(0.0), next(nullptr) {
            extractSkills();
        }
        
        void extractSkills() {
            // Extract skills from resume description
            std::string lowerDesc = toLowerCase(description);
            std::vector<std::string> commonSkills = {
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
        std::vector<std::string> commonSkills;
        
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
    std::vector<MatchResult> findMatches(int resumeId, int topK = 10);
    std::vector<MatchResult> findMatchesForJob(int jobId, int topK = 10);
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
    
    // Performance analysis
    double getLastSearchTime() const { return lastSearchTime; }
    double getLastSortTime() const { return lastSortTime; }
    size_t getMemoryUsed() const { return memoryUsed; }
    void resetPerformanceCounters();
    
    // Utility methods
    void displayTopMatches(const std::vector<MatchResult>& matches, int count = 5);
    void displayPerformanceStats();
    void validateData();
    void displayJobList();
    void displayResumeList();
    
private:
    void calculateMemoryUsage();
    std::vector<std::string> findCommonSkills(const JobNode& job, const ResumeNode& resume);
    void clearJobList();
    void clearResumeList();
    JobNode* copyJobNode(const JobNode* node);
    ResumeNode* copyResumeNode(const ResumeNode* node);
};

#endif // LINKED_LIST_JOB_MATCHER_HPP

