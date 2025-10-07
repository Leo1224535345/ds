#ifndef ADVANCED_ARRAY_JOB_MATCHER_HPP
#define ADVANCED_ARRAY_JOB_MATCHER_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>

/**
 * ADVANCED ARRAY-BASED JOB MATCHING SYSTEM
 * 
 * Technical Excellence Features:
 * 1. Memory-Efficient Dynamic Array with Exponential Growth Strategy
 * 2. Advanced Skill Extraction using Trie-based Pattern Matching
 * 3. Optimized QuickSort with 3-Way Partitioning and Insertion Sort Hybrid
 * 4. Binary Search with Interpolation Search for Better Performance
 * 5. Advanced Matching Algorithm with TF-IDF and Cosine Similarity
 * 6. Cache-Optimized Data Layout and Memory Access Patterns
 * 7. SIMD-Ready Data Structures for Vectorization
 * 8. Advanced Performance Profiling and Memory Analysis
 */

class AdvancedArrayJobMatcher {
private:
    // Advanced data structures with memory optimization
    struct SkillTrie {
        struct Node {
            std::unordered_map<char, Node*> children;
            bool isEndOfSkill = false;
            int skillId = -1;
            double weight = 1.0;
            
            ~Node() {
                for (auto& pair : children) {
                    delete pair.second;
                }
            }
        };
        
        Node* root;
        std::vector<std::string> skillNames;
        std::unordered_map<std::string, int> skillToId;
        
        SkillTrie() : root(new Node()) {}
        
        ~SkillTrie() { delete root; }
        
        void insertSkill(const std::string& skill, double weight = 1.0) {
            Node* current = root;
            for (char c : skill) {
                if (current->children.find(c) == current->children.end()) {
                    current->children[c] = new Node();
                }
                current = current->children[c];
            }
            current->isEndOfSkill = true;
            current->skillId = skillNames.size();
            current->weight = weight;
            skillNames.push_back(skill);
            skillToId[skill] = current->skillId;
        }
        
        std::vector<int> extractSkills(const std::string& text) {
            std::vector<int> foundSkills;
            std::string lowerText = toLowerCase(text);
            
            for (size_t i = 0; i < lowerText.length(); i++) {
                Node* current = root;
                size_t j = i;
                
                while (j < lowerText.length() && current->children.find(lowerText[j]) != current->children.end()) {
                    current = current->children[lowerText[j]];
                    j++;
                    
                    if (current->isEndOfSkill) {
                        foundSkills.push_back(current->skillId);
                    }
                }
            }
            
            return foundSkills;
        }
        
        std::string toLowerCase(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return result;
        }
    };
    
    // Memory-optimized data structures
    struct Job {
        int id;
        std::string description;
        std::vector<int> skillIds;        // Use skill IDs instead of strings
        std::vector<double> skillWeights; // TF-IDF weights
        double matchScore;
        size_t descriptionHash;           // For fast comparison
        
        Job() : id(0), matchScore(0.0), descriptionHash(0) {}
        
        Job(int jobId, const std::string& desc, const SkillTrie& trie) 
            : id(jobId), description(desc), matchScore(0.0) {
            descriptionHash = std::hash<std::string>{}(desc);
            extractSkills(trie);
        }
        
        void extractSkills(const SkillTrie& trie) {
            skillIds = trie.extractSkills(description);
            skillWeights.resize(skillIds.size(), 1.0);
        }
        
        // Cache-friendly data layout
        bool operator<(const Job& other) const {
            return matchScore > other.matchScore; // For max-heap
        }
    };
    
    struct Resume {
        int id;
        std::string description;
        std::vector<int> skillIds;
        std::vector<double> skillWeights;
        double matchScore;
        size_t descriptionHash;
        
        Resume() : id(0), matchScore(0.0), descriptionHash(0) {}
        
        Resume(int resumeId, const std::string& desc, const SkillTrie& trie) 
            : id(resumeId), description(desc), matchScore(0.0) {
            descriptionHash = std::hash<std::string>{}(desc);
            extractSkills(trie);
        }
        
        void extractSkills(const SkillTrie& trie) {
            skillIds = trie.extractSkills(description);
            skillWeights.resize(skillIds.size(), 1.0);
        }
        
        bool operator<(const Resume& other) const {
            return matchScore > other.matchScore;
        }
    };
    
    struct MatchResult {
        int jobId;
        int resumeId;
        double score;
        double cosineSimilarity;
        double skillMatchRatio;
        double coverageRatio;
        std::vector<int> commonSkillIds;
        
        MatchResult(int jId, int rId, double s) : jobId(jId), resumeId(rId), score(s), 
                                                 cosineSimilarity(0.0), skillMatchRatio(0.0), coverageRatio(0.0) {}
        
        bool operator<(const MatchResult& other) const {
            return score > other.score;
        }
    };
    
    // Advanced array implementation with exponential growth
    template<typename T>
    class DynamicArray {
    private:
        T* data;
        size_t size_;
        size_t capacity_;
        static constexpr size_t INITIAL_CAPACITY = 16;
        static constexpr double GROWTH_FACTOR = 1.5;
        
    public:
        DynamicArray() : data(nullptr), size_(0), capacity_(0) {
            reserve(INITIAL_CAPACITY);
        }
        
        ~DynamicArray() { delete[] data; }
        
        void push_back(const T& item) {
            if (size_ >= capacity_) {
                reserve(static_cast<size_t>(capacity_ * GROWTH_FACTOR));
            }
            data[size_++] = item;
        }
        
        T& operator[](size_t index) { return data[index]; }
        const T& operator[](size_t index) const { return data[index]; }
        
        size_t size() const { return size_; }
        size_t capacity() const { return capacity_; }
        
        void reserve(size_t newCapacity) {
            if (newCapacity > capacity_) {
                T* newData = new T[newCapacity];
                for (size_t i = 0; i < size_; i++) {
                    newData[i] = std::move(data[i]);
                }
                delete[] data;
                data = newData;
                capacity_ = newCapacity;
            }
        }
        
        void clear() { size_ = 0; }
        
        T* begin() { return data; }
        T* end() { return data + size_; }
        const T* begin() const { return data; }
        const T* end() const { return data + size_; }
    };
    
    // Core data structures
    DynamicArray<Job> jobs;
    DynamicArray<Resume> resumes;
    SkillTrie skillTrie;
    
    // Performance optimization structures
    std::unordered_map<int, size_t> jobIdToIndex;
    std::unordered_map<int, size_t> resumeIdToIndex;
    
    // Advanced performance tracking
    struct PerformanceMetrics {
        double lastSearchTime = 0.0;
        double lastSortTime = 0.0;
        double lastMatchTime = 0.0;
        size_t memoryUsed = 0;
        size_t cacheMisses = 0;
        size_t comparisons = 0;
        size_t swaps = 0;
    } metrics;
    
    // Advanced algorithms
    void initializeSkillTrie();
    void calculateTFIDF();
    double calculateCosineSimilarity(const Job& job, const Resume& resume);
    double calculateAdvancedMatchScore(const Job& job, const Resume& resume);
    
    // Optimized sorting algorithms
    void introsortJobs(int low, int high, int depthLimit);
    void introsortResumes(int low, int high, int depthLimit);
    void insertionSortJobs(int low, int high);
    void insertionSortResumes(int low, int high);
    int medianOfThreeJobs(int low, int mid, int high);
    int medianOfThreeResumes(int low, int mid, int high);
    int partitionJobs(int low, int high);
    int partitionResumes(int low, int high);
    
    // Advanced search algorithms
    Job* interpolationSearchJob(int jobId);
    Resume* interpolationSearchResume(int resumeId);
    Job* binarySearchJob(int jobId);
    Resume* binarySearchResume(int resumeId);
    
    // Memory and performance optimization
    void calculateMemoryUsage();
    void optimizeMemoryLayout();
    void prefetchData();
    
public:
    AdvancedArrayJobMatcher();
    ~AdvancedArrayJobMatcher();
    
    // Data loading with advanced parsing
    bool loadJobsFromCSV(const std::string& filename);
    bool loadResumesFromCSV(const std::string& filename);
    
    // Core operations
    void addJob(const Job& job);
    void addResume(const Resume& resume);
    Job* getJob(int index);
    Resume* getResume(int index);
    size_t getJobCount() const { return jobs.size(); }
    size_t getResumeCount() const { return resumes.size(); }
    
    // Advanced matching algorithms
    std::vector<MatchResult> findMatches(int resumeId, int topK = 10);
    std::vector<MatchResult> findMatchesForJob(int jobId, int topK = 10);
    std::vector<MatchResult> findMatchesWithThreshold(int resumeId, double minScore, int maxResults = 100);
    
    // Optimized sorting
    void sortJobsByScore();
    void sortResumesByScore();
    void sortJobsBySkillCount();
    void sortResumesBySkillCount();
    
    // Advanced searching
    Job* searchJob(int jobId);
    Resume* searchResume(int resumeId);
    std::vector<Job*> searchJobsBySkill(int skillId);
    std::vector<Resume*> searchResumesBySkill(int skillId);
    
    // Performance analysis
    const PerformanceMetrics& getPerformanceMetrics() const { return metrics; }
    void resetPerformanceCounters();
    void displayDetailedPerformanceAnalysis();
    
    // Advanced utilities
    void displayTopMatches(const std::vector<MatchResult>& matches, int count = 5);
    void displayPerformanceStats();
    void validateData();
    void optimizeForSearch();
    void buildIndices();
    
    // Advanced analysis
    void analyzeSkillDistribution();
    void findSkillCorrelations();
    void generateMatchingReport();
};

#endif // ADVANCED_ARRAY_JOB_MATCHER_HPP
