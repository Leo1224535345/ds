#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <map>
#include <unordered_map>
#include <set>

#include "ArrayJobMatcher.hpp"
#include "LinkedListJobMatcher.hpp"

class JobPortalCLI {
private:
    ArrayJobMatcher arrayMatcher;
    LinkedListJobMatcher listMatcher;
    bool useArrayImpl;
    int pageSize;
    
    // Performance tracking
    std::chrono::steady_clock::time_point operationStart;
    size_t peakMemoryUsage;
    
    // Color codes for terminal output
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string BOLD = "\033[1m";
    
public:
    JobPortalCLI(bool useArray = true, int pageSize = 20) 
        : useArrayImpl(useArray), pageSize(pageSize), peakMemoryUsage(0) {
        loadData();
    }
    
    void loadData() {
        std::cout << BLUE << "Loading job and resume data..." << RESET << std::endl;
        auto start = std::chrono::steady_clock::now();
        
        if (useArrayImpl) {
            arrayMatcher.loadJobsFromCSV("job_description.csv");
            arrayMatcher.loadResumesFromCSV("resume.csv");
        } else {
            listMatcher.loadJobsFromCSV("job_description.csv");
            listMatcher.loadResumesFromCSV("resume.csv");
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << GREEN << "✅ Data loaded in " << duration.count() << " ms" << RESET << std::endl;
    }
    
    void clearScreen() {
        std::cout << "\033[2J\033[1;1H"; // Clear screen and move cursor to top
    }
    
    void printHeader(const std::string& title) {
        std::string impl = useArrayImpl ? "Array" : "Linked List";
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << " " << title << " [Impl: " << impl << "] " << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
    
    void printRule() {
        std::cout << std::string(60, '-') << std::endl;
    }
    
    void startOperation() {
        operationStart = std::chrono::steady_clock::now();
    }
    
    void endOperation() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - operationStart);
        
        std::cout << "\n" << std::string(60, '-') << std::endl;
        std::cout << "Execution Time: " << duration.count() << " ms";
        
        // Memory tracking (simplified)
        size_t currentMemory = getCurrentMemoryUsage();
        if (currentMemory > peakMemoryUsage) {
            peakMemoryUsage = currentMemory;
        }
        
        if (currentMemory > 0) {
            std::cout << "  Memory: " << (currentMemory / 1024 / 1024) << " MB";
            if (peakMemoryUsage > currentMemory) {
                std::cout << " / " << (peakMemoryUsage / 1024 / 1024) << " MB";
            }
        } else {
            std::cout << "  Memory: N/A";
        }
        std::cout << std::endl;
    }
    
    void showPerformanceInsights() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << " PERFORMANCE INSIGHTS [Impl: " << (useArrayImpl ? "Array" : "Linked List") << "] " << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        if (useArrayImpl) {
            std::cout << "🔵 Array Implementation Advantages:" << std::endl;
            std::cout << "   ✓ Cache Performance: Superior spatial locality" << std::endl;
            std::cout << "   ✓ Memory Efficiency: No pointer overhead" << std::endl;
            std::cout << "   ✓ Random Access: O(1) access to any element" << std::endl;
            std::cout << "   ✓ Search Performance: O(1) Hash Map indexing" << std::endl;
            std::cout << "   ✓ Sort Performance: O(n log n) QuickSort with cache optimization" << std::endl;
        } else {
            std::cout << "🟢 Linked List Implementation Advantages:" << std::endl;
            std::cout << "   ✓ Dynamic Size: No reallocation needed" << std::endl;
            std::cout << "   ✓ Insertion Efficiency: O(1) for known position" << std::endl;
            std::cout << "   ✓ Memory Flexibility: No contiguous memory requirement" << std::endl;
            std::cout << "   ✓ Sort Performance: O(n log n) MergeSort" << std::endl;
        }
        
        std::cout << "\n📊 Algorithm Complexity:" << std::endl;
        std::cout << "   • Matching Algorithm: TF-IDF + Cosine Similarity" << std::endl;
        std::cout << "   • Search: " << (useArrayImpl ? "O(1) Hash Map" : "O(n) Linear Search") << std::endl;
        std::cout << "   • Sort: " << (useArrayImpl ? "O(n log n) QuickSort" : "O(n log n) MergeSort") << std::endl;
        std::cout << "   • Memory Access: " << (useArrayImpl ? "O(1) Random" : "O(n) Sequential") << std::endl;
    }
    
    void showSystemStats() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << " SYSTEM STATISTICS [Impl: " << (useArrayImpl ? "Array" : "Linked List") << "] " << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        int jobCount = useArrayImpl ? arrayMatcher.getJobCount() : listMatcher.getJobCount();
        int resumeCount = useArrayImpl ? arrayMatcher.getResumeCount() : listMatcher.getResumeCount();
        
        std::cout << "📊 Data Loaded:" << std::endl;
        std::cout << "   • Jobs: " << jobCount << " records" << std::endl;
        std::cout << "   • Resumes: " << resumeCount << " records" << std::endl;
        std::cout << "   • Total Records: " << (jobCount + resumeCount) << std::endl;
        
        std::cout << "\n💾 Memory Usage:" << std::endl;
        size_t memory = getCurrentMemoryUsage();
        std::cout << "   • Current: " << (memory / 1024 / 1024) << " MB" << std::endl;
        std::cout << "   • Peak: " << (peakMemoryUsage / 1024 / 1024) << " MB" << std::endl;
        
        std::cout << "\n⚡ Performance:" << std::endl;
        std::cout << "   • Implementation: " << (useArrayImpl ? "Array-based" : "Linked List-based") << std::endl;
        std::cout << "   • Page Size: " << pageSize << " items per page" << std::endl;
        std::cout << "   • Error Handling: Comprehensive validation enabled" << std::endl;
    }
    
    void showResumeIdInfo() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << " RESUME ID INFORMATION [Impl: " << (useArrayImpl ? "Array" : "Linked List") << "] " << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        int resumeCount = useArrayImpl ? arrayMatcher.getResumeCount() : listMatcher.getResumeCount();
        
        std::cout << "📋 AVAILABLE RESUME IDs:" << std::endl;
        std::cout << "   • Valid Range: 1 to " << resumeCount << std::endl;
        std::cout << "   • Total Resumes: " << resumeCount << " records" << std::endl;
        
        std::cout << "\n🎯 SAMPLE RESUME IDs:" << std::endl;
        
        if (useArrayImpl) {
            // Show first 5 resume IDs with their skills
            for (int i = 1; i <= std::min(5, resumeCount); i++) {
                const auto& resume = arrayMatcher.getResumeById(i);
                if (resume.id > 0) {
                    std::cout << "   ✅ Resume ID " << resume.id << ":" << std::endl;
                    std::cout << "      Skills: ";
                    for (size_t j = 0; j < resume.skills.size(); j++) {
                        std::cout << resume.skills[j];
                        if (j < resume.skills.size() - 1) std::cout << ", ";
                    }
                    std::cout << std::endl;
                    std::cout << "      Description: " << resume.description.substr(0, 60) << "..." << std::endl;
                    std::cout << std::endl;
                }
            }
        }
        
        std::cout << "💡 USAGE TIPS:" << std::endl;
        std::cout << "   • Use Resume ID 1 for SQL/Power BI skills" << std::endl;
        std::cout << "   • Use Resume ID 2 for Python/ML skills" << std::endl;
        std::cout << "   • Use Resume ID 3 for Java/System Design skills" << std::endl;
        std::cout << "   • Try different IDs to see various skill combinations" << std::endl;
        
        std::cout << "\n🚀 QUICK COMMANDS:" << std::endl;
        std::cout << "   • Search Resume: Choose [2] Search Resume by ID" << std::endl;
        std::cout << "   • Find Matching Jobs: Choose [3] Find Matching Jobs for My Resume" << std::endl;
        std::cout << "   • Browse All Resumes: Choose [1] Browse All Resumes" << std::endl;
    }
    
    void showJobIdInfo() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << " JOB ID INFORMATION [Impl: " << (useArrayImpl ? "Array" : "Linked List") << "] " << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        int jobCount = useArrayImpl ? arrayMatcher.getJobCount() : listMatcher.getJobCount();
        
        std::cout << "📋 AVAILABLE JOB IDs:" << std::endl;
        std::cout << "   • Valid Range: 1 to " << jobCount << std::endl;
        std::cout << "   • Total Jobs: " << jobCount << " records" << std::endl;
        
        std::cout << "\n🎯 SAMPLE JOB IDs:" << std::endl;
        
        if (useArrayImpl) {
            // Show first 5 job IDs with their skills
            for (int i = 1; i <= std::min(5, jobCount); i++) {
                const auto& job = arrayMatcher.getJobById(i);
                if (job.id > 0) {
                    std::cout << "   ✅ Job ID " << job.id << ":" << std::endl;
                    std::cout << "      Required Skills: ";
                    for (size_t j = 0; j < job.skills.size(); j++) {
                        std::cout << job.skills[j];
                        if (j < job.skills.size() - 1) std::cout << ", ";
                    }
                    std::cout << std::endl;
                    std::cout << "      Description: " << job.description.substr(0, 60) << "..." << std::endl;
                    std::cout << std::endl;
                }
            }
        }
        
        std::cout << "💡 USAGE TIPS:" << std::endl;
        std::cout << "   • Use Job ID 1 for general skills" << std::endl;
        std::cout << "   • Use Job ID 2 for Data Science/Python skills" << std::endl;
        std::cout << "   • Use Job ID 3 for Java/System Design skills" << std::endl;
        std::cout << "   • Try different IDs to see various job requirements" << std::endl;
        
        std::cout << "\n🚀 QUICK COMMANDS:" << std::endl;
        std::cout << "   • Search Job: Choose [2] Search Job by ID" << std::endl;
        std::cout << "   • Find Matching Jobs: Choose [3] Find Matching Jobs for My Resume" << std::endl;
        std::cout << "   • Browse All Jobs: Choose [1] Browse All Jobs" << std::endl;
    }
    
    void showSkillFrequencyAnalysis() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << " SKILL FREQUENCY ANALYSIS [Impl: " << (useArrayImpl ? "Array" : "Linked List") << "] " << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        std::unordered_map<std::string, int> jobSkills, resumeSkills;
        
        // Count skills in jobs
        if (useArrayImpl) {
            int jobCount = arrayMatcher.getJobCount();
            for (int i = 0; i < jobCount; i++) {
                const auto& job = *arrayMatcher.getJob(i);
                for (const auto& skill : job.skills) {
                    jobSkills[skill]++;
                }
            }
        } else {
            // For linked list, traverse the list
            auto* current = listMatcher.getJobHead();
            while (current != nullptr) {
                for (const auto& skill : current->skills) {
                    jobSkills[skill]++;
                }
                current = current->next;
            }
        }
        
        // Count skills in resumes
        if (useArrayImpl) {
            int resumeCount = arrayMatcher.getResumeCount();
            for (int i = 0; i < resumeCount; i++) {
                const auto& resume = *arrayMatcher.getResume(i);
                for (const auto& skill : resume.skills) {
                    resumeSkills[skill]++;
                }
            }
        } else {
            // For linked list, traverse the list
            auto* current = listMatcher.getResumeHead();
            while (current != nullptr) {
                for (const auto& skill : current->skills) {
                    resumeSkills[skill]++;
                }
                current = current->next;
            }
        }
        
        // Sort by frequency
        std::vector<std::pair<std::string, int>> topJobSkills(jobSkills.begin(), jobSkills.end());
        std::vector<std::pair<std::string, int>> topResumeSkills(resumeSkills.begin(), resumeSkills.end());
        
        std::sort(topJobSkills.begin(), topJobSkills.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        std::sort(topResumeSkills.begin(), topResumeSkills.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::cout << CYAN << "📊 TOP 10 MOST DEMANDED SKILLS (Jobs):" << RESET << std::endl;
        for (int i = 0; i < std::min(10, (int)topJobSkills.size()); i++) {
            std::cout << "   " << (i+1) << ". " << GREEN << topJobSkills[i].first << RESET 
                      << " (" << topJobSkills[i].second << " jobs)" << std::endl;
        }
        
        std::cout << "\n" << CYAN << "📊 TOP 10 MOST AVAILABLE SKILLS (Resumes):" << RESET << std::endl;
        for (int i = 0; i < std::min(10, (int)topResumeSkills.size()); i++) {
            std::cout << "   " << (i+1) << ". " << BLUE << topResumeSkills[i].first << RESET 
                      << " (" << topResumeSkills[i].second << " resumes)" << std::endl;
        }
        
        // Skill gap analysis
        std::cout << "\n" << YELLOW << "⚠️  SKILL GAP ANALYSIS:" << RESET << std::endl;
        std::set<std::string> highDemandSkills, highSupplySkills;
        
        for (int i = 0; i < std::min(5, (int)topJobSkills.size()); i++) {
            highDemandSkills.insert(topJobSkills[i].first);
        }
        for (int i = 0; i < std::min(5, (int)topResumeSkills.size()); i++) {
            highSupplySkills.insert(topResumeSkills[i].first);
        }
        
        std::cout << "   High Demand, Low Supply: ";
        bool first = true;
        for (const auto& skill : highDemandSkills) {
            if (highSupplySkills.find(skill) == highSupplySkills.end()) {
                if (!first) std::cout << ", ";
                std::cout << RED << skill << RESET;
                first = false;
            }
        }
        if (first) std::cout << "None";
        std::cout << std::endl;
        
        std::cout << "   High Supply, Low Demand: ";
        first = true;
        for (const auto& skill : highSupplySkills) {
            if (highDemandSkills.find(skill) == highDemandSkills.end()) {
                if (!first) std::cout << ", ";
                std::cout << MAGENTA << skill << RESET;
                first = false;
            }
        }
        if (first) std::cout << "None";
        std::cout << std::endl;
    }
    
    size_t getCurrentMemoryUsage() {
        // Simplified memory tracking - approximate based on counts
        return useArrayImpl ? arrayMatcher.getJobCount() * 1024 + 
                              arrayMatcher.getResumeCount() * 1024 : 0;
    }
    
    int getValidId(const std::string& prompt, int maxId) {
        int id;
        while (true) {
            std::cout << prompt;
            if (!(std::cin >> id) || std::cin.fail()) {
                if (std::cin.eof()) {
                    return -1; // EOF detected
                }
                std::cout << "Invalid input. Please enter a valid number." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            
            if (id <= 0 || id > maxId) {
                std::cout << "Invalid ID. Please enter a number between 1 and " << maxId << "." << std::endl;
                continue;
            }
            
            return id;
        }
    }
    
    int getValidTopK() {
        int topK;
        while (true) {
            std::cout << "Enter number of results (1-1000, default 20): ";
            std::string input;
            std::getline(std::cin, input);
            
            if (input.empty()) {
                return 20; // Default
            }
            
            try {
                topK = std::stoi(input);
                if (topK < 1 || topK > 1000) {
                    std::cout << "Invalid number. Please enter between 1 and 1000." << std::endl;
                    continue;
                }
                return topK;
            } catch (const std::exception&) {
                std::cout << "Invalid input. Please enter a valid number." << std::endl;
                continue;
            }
        }
    }
    
    void showMainMenu() {
        clearScreen();
        printHeader("Job Portal - Main Menu");
        std::cout << "\nWho are you?\n" << std::endl;
        std::cout << "[1] Employer" << std::endl;
        std::cout << "[2] Job Seeker" << std::endl;
        std::cout << "[0] Exit" << std::endl;
        std::cout << "\nEnter your choice: ";
    }
    
    void showEmployerMenu() {
        clearScreen();
        printHeader("Recruiter Mode");
        std::cout << "\n[1] Browse All Resumes" << std::endl;
        std::cout << "[2] Search Resume by ID" << std::endl;
        std::cout << "[3] Job Matching" << std::endl;
        std::cout << "[4] System Statistics" << std::endl;
        std::cout << "[5] Show Resume ID Information" << std::endl;
        std::cout << "[6] " << CYAN << "Skill Frequency Analysis" << RESET << std::endl;
        std::cout << "[0] Back to Main Menu" << std::endl;
        std::cout << "\nEnter your choice: ";
    }
    
    void showSeekerMenu() {
        clearScreen();
        printHeader("Job Seeker Mode");
        std::cout << "\n[1] Browse All Jobs" << std::endl;
        std::cout << "[2] Search Job by ID" << std::endl;
        std::cout << "[3] Find Matching Jobs for My Resume" << std::endl;
        std::cout << "[4] System Statistics" << std::endl;
        std::cout << "[5] Show Job ID Information" << std::endl;
        std::cout << "[6] " << CYAN << "Skill Frequency Analysis" << RESET << std::endl;
        std::cout << "[0] Back to Main Menu" << std::endl;
        std::cout << "\nEnter your choice: ";
    }
    
    void browseResumes(int page = 1) {
        startOperation();
        
        int totalResumes = useArrayImpl ? arrayMatcher.getResumeCount() : listMatcher.getResumeCount();
        int totalPages = (totalResumes + pageSize - 1) / pageSize;
        int startIdx = (page - 1) * pageSize;
        int endIdx = std::min(startIdx + pageSize, totalResumes);
        
        printHeader("Browse All Resumes");
        std::cout << "Pages " << page << "/" << totalPages << " (showing " << (startIdx + 1) << "–" << endIdx << " of " << totalResumes << ")" << std::endl;
        printRule();
        
        std::cout << std::left << std::setw(8) << "ID" 
                  << std::setw(30) << "Name" 
                  << std::setw(15) << "Skills Count" << std::endl;
        printRule();
        
        if (useArrayImpl) {
            for (int i = startIdx; i < endIdx; i++) {
                const auto* resume = arrayMatcher.getResume(i);
                if (resume) {
                    std::cout << std::left << std::setw(8) << resume->id
                              << std::setw(30) << resume->description.substr(0, 29)
                              << std::setw(15) << resume->skills.size() << std::endl;
                }
            }
        } else {
            // For linked list, we'd need to traverse to the specific index
            // This is a simplified version
            std::cout << "Linked list browsing not fully implemented for pagination" << std::endl;
        }
        
        endOperation();
        showPaginationControls(page, totalPages);
    }
    
    void browseJobs(int page = 1) {
        startOperation();
        
        int totalJobs = useArrayImpl ? arrayMatcher.getJobCount() : listMatcher.getJobCount();
        int totalPages = (totalJobs + pageSize - 1) / pageSize;
        int startIdx = (page - 1) * pageSize;
        int endIdx = std::min(startIdx + pageSize, totalJobs);
        
        printHeader("Browse All Jobs");
        std::cout << "Pages " << page << "/" << totalPages << " (showing " << (startIdx + 1) << "–" << endIdx << " of " << totalJobs << ")" << std::endl;
        printRule();
        
        std::cout << std::left << std::setw(8) << "ID" 
                  << std::setw(40) << "Title" 
                  << std::setw(15) << "Skills Count" << std::endl;
        printRule();
        
        if (useArrayImpl) {
            for (int i = startIdx; i < endIdx; i++) {
                const auto* job = arrayMatcher.getJob(i);
                if (job) {
                    std::cout << std::left << std::setw(8) << job->id
                              << std::setw(40) << job->description.substr(0, 39)
                              << std::setw(15) << job->skills.size() << std::endl;
                }
            }
        }
        
        endOperation();
        showPaginationControls(page, totalPages);
    }
    
    void searchResumeById(int resumeId) {
        startOperation();
        
        printHeader("Resume Details");
        
        if (useArrayImpl) {
            const auto& resume = arrayMatcher.getResumeById(resumeId);
            if (resume.id == 0) {
                std::cout << "Resume not found!" << std::endl;
                endOperation();
                return;
            }
            
            std::cout << "ID: " << resume.id << std::endl;
            std::cout << "Description: " << resume.description << std::endl;
            std::cout << "Skills: ";
            for (size_t i = 0; i < resume.skills.size(); i++) {
                std::cout << resume.skills[i];
                if (i < resume.skills.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        }
        
        endOperation();
    }
    
    void searchJobById(int jobId) {
        startOperation();
        
        printHeader("Job Details");
        
        if (useArrayImpl) {
            const auto& job = arrayMatcher.getJobById(jobId);
            if (job.id == 0) {
                std::cout << "Job not found!" << std::endl;
                endOperation();
                return;
            }
            
            std::cout << "ID: " << job.id << std::endl;
            std::cout << "Description: " << job.description << std::endl;
            std::cout << "Required Skills: ";
            for (size_t i = 0; i < job.skills.size(); i++) {
                std::cout << job.skills[i];
                if (i < job.skills.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
        }
        
        endOperation();
    }
    
    void jobMatching(int jobId, int topK) {
        startOperation();
        
        if (useArrayImpl) {
            const auto& job = arrayMatcher.getJobById(jobId);
            if (job.id == 0) {
                std::cout << "Job not found!" << std::endl;
                endOperation();
                return;
            }
            
            printHeader("Top Matching Candidates");
            std::cout << "Job: " << job.description << std::endl;
            std::cout << "Required Skills: ";
            for (size_t i = 0; i < job.skills.size(); i++) {
                std::cout << job.skills[i];
                if (i < job.skills.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
            
            auto matches = arrayMatcher.findMatchesForJob(jobId, topK);
            std::cout << "Total: " << matches.size() << " matching resumes." << std::endl;
            printRule();
            
            std::cout << std::left << std::setw(8) << "ID" 
                      << std::setw(12) << "Score" 
                      << std::setw(40) << "Common Skills" << std::endl;
            printRule();
            
            for (const auto& match : matches) {
                std::string commonSkillsStr;
                for (size_t i = 0; i < match.commonSkills.size(); i++) {
                    commonSkillsStr += match.commonSkills[i];
                    if (i < match.commonSkills.size() - 1) commonSkillsStr += ", ";
                }
                if (commonSkillsStr.length() > 39) {
                    commonSkillsStr = commonSkillsStr.substr(0, 36) + "...";
                }
                
                std::cout << std::left << std::setw(8) << match.resumeId
                          << std::setw(12) << std::fixed << std::setprecision(2) << match.overallScore
                          << std::setw(40) << commonSkillsStr << std::endl;
            }
            
            // Add performance insights
            showPerformanceInsights();
        }
        
        endOperation();
    }
    
    void resumeMatching(int resumeId, int topK) {
        startOperation();
        
        if (useArrayImpl) {
            const auto& resume = arrayMatcher.getResumeById(resumeId);
            if (resume.id == 0) {
                std::cout << "Resume not found!" << std::endl;
                endOperation();
                return;
            }
            
            printHeader("Top Matching Jobs");
            std::cout << "Resume: " << resume.description << std::endl;
            std::cout << "Skills: ";
            for (size_t i = 0; i < resume.skills.size(); i++) {
                std::cout << resume.skills[i];
                if (i < resume.skills.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
            
            auto matches = arrayMatcher.findMatches(resumeId, topK);
            std::cout << "Total: " << matches.size() << " matching jobs." << std::endl;
            printRule();
            
            std::cout << std::left << std::setw(8) << "ID" 
                      << std::setw(12) << "Score" 
                      << std::setw(40) << "Common Skills" << std::endl;
            printRule();
            
            for (const auto& match : matches) {
                std::string commonSkillsStr;
                for (size_t i = 0; i < match.commonSkills.size(); i++) {
                    commonSkillsStr += match.commonSkills[i];
                    if (i < match.commonSkills.size() - 1) commonSkillsStr += ", ";
                }
                if (commonSkillsStr.length() > 39) {
                    commonSkillsStr = commonSkillsStr.substr(0, 36) + "...";
                }
                
                std::cout << std::left << std::setw(8) << match.jobId
                          << std::setw(12) << std::fixed << std::setprecision(2) << match.overallScore
                          << std::setw(40) << commonSkillsStr << std::endl;
            }
            
            // Add performance insights
            showPerformanceInsights();
        }
        
        endOperation();
    }
    
    void showPaginationControls(int currentPage, int totalPages) {
        std::cout << "\nControls: [n] next, [p] prev, [#] go to page, [q] quit to previous menu" << std::endl;
        std::cout << "Enter command: ";
        
        std::string command;
        std::getline(std::cin, command);
        
        if (command == "n" && currentPage < totalPages) {
            browseResumes(currentPage + 1);
        } else if (command == "p" && currentPage > 1) {
            browseResumes(currentPage - 1);
        } else if (command[0] == '#') {
            try {
                int page = std::stoi(command.substr(1));
                if (page >= 1 && page <= totalPages) {
                    browseResumes(page);
                } else {
                    std::cout << "Invalid page number!" << std::endl;
                    showPaginationControls(currentPage, totalPages);
                }
            } catch (const std::exception&) {
                std::cout << "Invalid command!" << std::endl;
                showPaginationControls(currentPage, totalPages);
            }
        } else if (command == "q") {
            return;
        } else {
            std::cout << "Invalid command!" << std::endl;
            showPaginationControls(currentPage, totalPages);
        }
    }
    
    void runInteractive() {
        while (true) {
            showMainMenu();
            
            int choice;
            if (!(std::cin >> choice) || std::cin.fail()) {
                if (std::cin.eof()) break;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cin.ignore(); // Clear newline
            
            if (choice == 0) break;
            else if (choice == 1) runEmployerMenu();
            else if (choice == 2) runSeekerMenu();
            else {
                std::cout << "Invalid choice!" << std::endl;
                std::cout << "Press Enter to continue...";
                std::cin.get();
            }
        }
    }
    
    void runEmployerMenu() {
        while (true) {
            showEmployerMenu();
            
            int choice;
            if (!(std::cin >> choice) || std::cin.fail()) {
                if (std::cin.eof()) break;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cin.ignore(); // Clear newline
            
            if (choice == 0) break;
            else if (choice == 1) browseResumes();
            else if (choice == 2) {
                int resumeId = getValidId("Enter Resume ID: ", useArrayImpl ? arrayMatcher.getResumeCount() : listMatcher.getResumeCount());
                if (resumeId > 0) {
                    searchResumeById(resumeId);
                    std::cout << "\nPress Enter to continue...";
                    std::cin.get();
                }
            }
            else if (choice == 3) {
                int jobId = getValidId("Enter Job ID: ", useArrayImpl ? arrayMatcher.getJobCount() : listMatcher.getJobCount());
                if (jobId > 0) {
                    int topK = getValidTopK();
                    jobMatching(jobId, topK);
                    std::cout << "\nPress Enter to continue...";
                    std::cin.get();
                }
            }
            else if (choice == 4) {
                showSystemStats();
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            else if (choice == 5) {
                showResumeIdInfo();
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            else if (choice == 6) {
                showSkillFrequencyAnalysis();
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            else {
                std::cout << RED << "Invalid choice!" << RESET << std::endl;
                std::cout << "Press Enter to continue...";
                std::cin.get();
            }
        }
    }
    
    void runSeekerMenu() {
        while (true) {
            showSeekerMenu();
            
            int choice;
            if (!(std::cin >> choice) || std::cin.fail()) {
                if (std::cin.eof()) break;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cin.ignore(); // Clear newline
            
            if (choice == 0) break;
            else if (choice == 1) browseJobs();
            else if (choice == 2) {
                int jobId = getValidId("Enter Job ID: ", useArrayImpl ? arrayMatcher.getJobCount() : listMatcher.getJobCount());
                if (jobId > 0) {
                    searchJobById(jobId);
                    std::cout << "\nPress Enter to continue...";
                    std::cin.get();
                }
            }
            else if (choice == 3) {
                int resumeId = getValidId("Enter Resume ID: ", useArrayImpl ? arrayMatcher.getResumeCount() : listMatcher.getResumeCount());
                if (resumeId > 0) {
                    int topK = getValidTopK();
                    resumeMatching(resumeId, topK);
                    std::cout << "\nPress Enter to continue...";
                    std::cin.get();
                }
            }
            else if (choice == 4) {
                showSystemStats();
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            else if (choice == 5) {
                showJobIdInfo();
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            else if (choice == 6) {
                showSkillFrequencyAnalysis();
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            else {
                std::cout << RED << "Invalid choice!" << RESET << std::endl;
                std::cout << "Press Enter to continue...";
                std::cin.get();
            }
        }
    }
    
    void runBatchMode(const std::string& mode, const std::string& action, 
                     const std::map<std::string, std::string>& params) {
        if (mode == "employer") {
            if (action == "browse-resumes") {
                int page = 1;
                if (params.count("page")) {
                    page = std::stoi(params.at("page"));
                }
                browseResumes(page);
            } else if (action == "search-resume") {
                int id = std::stoi(params.at("id"));
                searchResumeById(id);
            } else if (action == "match-job") {
                int jobId = std::stoi(params.at("job-id"));
                int topK = params.count("topk") ? std::stoi(params.at("topk")) : 20;
                jobMatching(jobId, topK);
            }
        } else if (mode == "seeker") {
            if (action == "browse-jobs") {
                int page = 1;
                if (params.count("page")) {
                    page = std::stoi(params.at("page"));
                }
                browseJobs(page);
            } else if (action == "search-job") {
                int id = std::stoi(params.at("id"));
                searchJobById(id);
            } else if (action == "match-resume") {
                int resumeId = std::stoi(params.at("resume-id"));
                int topK = params.count("topk") ? std::stoi(params.at("topk")) : 20;
                resumeMatching(resumeId, topK);
            }
        }
    }
};

void printUsage() {
    std::cout << "Job Portal CLI - Interactive Job Matching System\n" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  Interactive mode: ./job_portal_cli [options]" << std::endl;
    std::cout << "  Batch mode: ./job_portal_cli --mode=<mode> --action=<action> [options]\n" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --impl=array|list     Implementation to use (default: array)" << std::endl;
    std::cout << "  --page-size=N         Page size for pagination (default: 20)" << std::endl;
    std::cout << "  --mode=employer|seeker Mode for batch operations" << std::endl;
    std::cout << "  --action=<action>     Action for batch operations" << std::endl;
    std::cout << "  --id=N               ID for search operations" << std::endl;
    std::cout << "  --job-id=N           Job ID for matching" << std::endl;
    std::cout << "  --resume-id=N        Resume ID for matching" << std::endl;
    std::cout << "  --topk=N             Number of results (default: 20)" << std::endl;
    std::cout << "  --page=N             Page number for browsing" << std::endl;
    std::cout << "\nBatch Actions:" << std::endl;
    std::cout << "  employer: browse-resumes, search-resume, match-job" << std::endl;
    std::cout << "  seeker:   browse-jobs, search-job, match-resume" << std::endl;
}

int main(int argc, char* argv[]) {
    bool useArray = true;
    int pageSize = 20;
    bool batchMode = false;
    std::string mode, action;
    std::map<std::string, std::string> params;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
        
        if (arg.find("--impl=") == 0) {
            std::string impl = arg.substr(7);
            useArray = (impl == "array");
        } else if (arg.find("--page-size=") == 0) {
            pageSize = std::stoi(arg.substr(12));
        } else if (arg.find("--mode=") == 0) {
            mode = arg.substr(7);
            batchMode = true;
        } else if (arg.find("--action=") == 0) {
            action = arg.substr(9);
        } else if (arg.find("--id=") == 0) {
            params["id"] = arg.substr(5);
        } else if (arg.find("--job-id=") == 0) {
            params["job-id"] = arg.substr(9);
        } else if (arg.find("--resume-id=") == 0) {
            params["resume-id"] = arg.substr(12);
        } else if (arg.find("--topk=") == 0) {
            params["topk"] = arg.substr(7);
        } else if (arg.find("--page=") == 0) {
            params["page"] = arg.substr(7);
        }
    }
    
    try {
        JobPortalCLI cli(useArray, pageSize);
        
        if (batchMode) {
            cli.runBatchMode(mode, action, params);
        } else {
            cli.runInteractive();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
