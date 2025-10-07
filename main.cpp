#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include "ArrayJobMatcher.hpp"
#include "LinkedListJobMatcher.hpp"

class PerformanceAnalyzer {
private:
    struct PerformanceMetrics {
        double arraySearchTime;
        double linkedListSearchTime;
        double arraySortTime;
        double linkedListSortTime;
        size_t arrayMemory;
        size_t linkedListMemory;
        int arrayJobCount;
        int linkedListJobCount;
        int arrayResumeCount;
        int linkedListResumeCount;
    };
    
    PerformanceMetrics metrics;
    
public:
    void runComprehensiveTest() {
        std::cout << "===============================================" << std::endl;
        std::cout << "    JOB MATCHING SYSTEM PERFORMANCE ANALYSIS" << std::endl;
        std::cout << "===============================================" << std::endl;
        
        // Test Array Implementation
        std::cout << "\n1. Testing Array-Based Implementation..." << std::endl;
        testArrayImplementation();
        
        // Test Linked List Implementation
        std::cout << "\n2. Testing Linked List-Based Implementation..." << std::endl;
        testLinkedListImplementation();
        
        // Performance Comparison
        std::cout << "\n3. Performance Comparison Analysis..." << std::endl;
        comparePerformance();
        
        // Interactive Testing
        std::cout << "\n4. Interactive Testing Mode..." << std::endl;
        interactiveTesting();
    }
    
private:
    void testArrayImplementation() {
        ArrayJobMatcher arrayMatcher;
        
        // Load data
        std::cout << "Loading data into array-based structure..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        if (!arrayMatcher.loadJobsFromCSV("job_description.csv")) {
            std::cerr << "Failed to load job descriptions" << std::endl;
            return;
        }
        
        if (!arrayMatcher.loadResumesFromCSV("resume.csv")) {
            std::cerr << "Failed to load resumes" << std::endl;
            return;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double loadTime = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "Data loaded in " << loadTime << " ms" << std::endl;
        
        // Validate data
        arrayMatcher.validateData();
        
        // Test job matching
        std::cout << "\nTesting job matching for resume ID 1..." << std::endl;
        auto matches = arrayMatcher.findMatches(1, 5);
        arrayMatcher.displayTopMatches(matches, 5);
        
        // Test sorting
        std::cout << "\nTesting sorting algorithms..." << std::endl;
        arrayMatcher.sortJobsByScore();
        arrayMatcher.sortResumesByScore();
        
        // Test searching
        std::cout << "\nTesting search algorithms..." << std::endl;
        auto job = arrayMatcher.linearSearchJob(1);
        if (job) {
            std::cout << "Found job ID 1: " << job->description.substr(0, 50) << "..." << std::endl;
        }
        
        auto resume = arrayMatcher.linearSearchResume(1);
        if (resume) {
            std::cout << "Found resume ID 1: " << resume->description.substr(0, 50) << "..." << std::endl;
        }
        
        // Store metrics
        metrics.arraySearchTime = arrayMatcher.getLastSearchTime();
        metrics.arraySortTime = arrayMatcher.getLastSortTime();
        metrics.arrayMemory = arrayMatcher.getMemoryUsed();
        metrics.arrayJobCount = arrayMatcher.getJobCount();
        metrics.arrayResumeCount = arrayMatcher.getResumeCount();
        
        arrayMatcher.displayPerformanceStats();
    }
    
    void testLinkedListImplementation() {
        LinkedListJobMatcher linkedListMatcher;
        
        // Load data
        std::cout << "Loading data into linked list-based structure..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        if (!linkedListMatcher.loadJobsFromCSV("job_description.csv")) {
            std::cerr << "Failed to load job descriptions" << std::endl;
            return;
        }
        
        if (!linkedListMatcher.loadResumesFromCSV("resume.csv")) {
            std::cerr << "Failed to load resumes" << std::endl;
            return;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double loadTime = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "Data loaded in " << loadTime << " ms" << std::endl;
        
        // Validate data
        linkedListMatcher.validateData();
        
        // Test job matching
        std::cout << "\nTesting job matching for resume ID 1..." << std::endl;
        auto matches = linkedListMatcher.findMatches(1, 5);
        linkedListMatcher.displayTopMatches(matches, 5);
        
        // Test sorting
        std::cout << "\nTesting sorting algorithms..." << std::endl;
        linkedListMatcher.sortJobsByScore();
        linkedListMatcher.sortResumesByScore();
        
        // Test searching
        std::cout << "\nTesting search algorithms..." << std::endl;
        auto job = linkedListMatcher.linearSearchJob(1);
        if (job) {
            std::cout << "Found job ID 1: " << job->description.substr(0, 50) << "..." << std::endl;
        }
        
        auto resume = linkedListMatcher.linearSearchResume(1);
        if (resume) {
            std::cout << "Found resume ID 1: " << resume->description.substr(0, 50) << "..." << std::endl;
        }
        
        // Store metrics
        metrics.linkedListSearchTime = linkedListMatcher.getLastSearchTime();
        metrics.linkedListSortTime = linkedListMatcher.getLastSortTime();
        metrics.linkedListMemory = linkedListMatcher.getMemoryUsed();
        metrics.linkedListJobCount = linkedListMatcher.getJobCount();
        metrics.linkedListResumeCount = linkedListMatcher.getResumeCount();
        
        linkedListMatcher.displayPerformanceStats();
    }
    
    void comparePerformance() {
        std::cout << "\n=== PERFORMANCE COMPARISON ===" << std::endl;
        std::cout << std::fixed << std::setprecision(3);
        
        // Memory comparison
        std::cout << "\nMemory Usage:" << std::endl;
        std::cout << "Array Implementation: " << metrics.arrayMemory << " bytes" << std::endl;
        std::cout << "Linked List Implementation: " << metrics.linkedListMemory << " bytes" << std::endl;
        
        double memoryDiff = ((double)metrics.linkedListMemory - metrics.arrayMemory) / metrics.arrayMemory * 100;
        std::cout << "Memory Difference: " << std::abs(memoryDiff) << "% ";
        if (memoryDiff > 0) {
            std::cout << "(Linked List uses more)" << std::endl;
        } else {
            std::cout << "(Array uses more)" << std::endl;
        }
        
        // Search time comparison
        std::cout << "\nSearch Performance:" << std::endl;
        std::cout << "Array Implementation: " << metrics.arraySearchTime << " ms" << std::endl;
        std::cout << "Linked List Implementation: " << metrics.linkedListSearchTime << " ms" << std::endl;
        
        double searchDiff = ((double)metrics.linkedListSearchTime - metrics.arraySearchTime) / metrics.arraySearchTime * 100;
        std::cout << "Search Time Difference: " << std::abs(searchDiff) << "% ";
        if (searchDiff > 0) {
            std::cout << "(Linked List is slower)" << std::endl;
        } else {
            std::cout << "(Array is slower)" << std::endl;
        }
        
        // Sort time comparison
        std::cout << "\nSort Performance:" << std::endl;
        std::cout << "Array Implementation: " << metrics.arraySortTime << " ms" << std::endl;
        std::cout << "Linked List Implementation: " << metrics.linkedListSortTime << " ms" << std::endl;
        
        double sortDiff = ((double)metrics.linkedListSortTime - metrics.arraySortTime) / metrics.arraySortTime * 100;
        std::cout << "Sort Time Difference: " << std::abs(sortDiff) << "% ";
        if (sortDiff > 0) {
            std::cout << "(Linked List is slower)" << std::endl;
        } else {
            std::cout << "(Array is slower)" << std::endl;
        }
        
        // Data integrity check
        std::cout << "\nData Integrity:" << std::endl;
        std::cout << "Array - Jobs: " << metrics.arrayJobCount << ", Resumes: " << metrics.arrayResumeCount << std::endl;
        std::cout << "Linked List - Jobs: " << metrics.linkedListJobCount << ", Resumes: " << metrics.linkedListResumeCount << std::endl;
        
        if (metrics.arrayJobCount == metrics.linkedListJobCount && 
            metrics.arrayResumeCount == metrics.linkedListResumeCount) {
            std::cout << "✓ Data integrity verified - both implementations loaded same data" << std::endl;
        } else {
            std::cout << "✗ Data integrity issue detected!" << std::endl;
        }
        
        // Algorithm analysis
        std::cout << "\n=== ALGORITHM ANALYSIS ===" << std::endl;
        std::cout << "Array Implementation:" << std::endl;
        std::cout << "  - Search: O(n) linear search, O(log n) binary search" << std::endl;
        std::cout << "  - Sort: O(n log n) quicksort" << std::endl;
        std::cout << "  - Memory: O(n) contiguous memory" << std::endl;
        std::cout << "  - Cache: Better cache locality" << std::endl;
        
        std::cout << "\nLinked List Implementation:" << std::endl;
        std::cout << "  - Search: O(n) linear search only" << std::endl;
        std::cout << "  - Sort: O(n log n) merge sort" << std::endl;
        std::cout << "  - Memory: O(n) with pointer overhead" << std::endl;
        std::cout << "  - Cache: Poor cache locality due to scattered memory" << std::endl;
        
        std::cout << "\nRecommendation:" << std::endl;
        if (metrics.arraySearchTime < metrics.linkedListSearchTime && 
            metrics.arraySortTime < metrics.linkedListSortTime) {
            std::cout << "Array implementation is more efficient for this use case." << std::endl;
        } else {
            std::cout << "Linked list implementation shows better performance in some areas." << std::endl;
        }
    }
    
    void interactiveTesting() {
        std::cout << "\n=== INTERACTIVE TESTING MODE ===" << std::endl;
        std::cout << "Choose implementation to test:" << std::endl;
        std::cout << "1. Array-based implementation" << std::endl;
        std::cout << "2. Linked list-based implementation" << std::endl;
        std::cout << "3. Compare both implementations" << std::endl;
        std::cout << "4. Exit" << std::endl;
        
        int choice;
        std::cout << "Enter your choice (1-4): ";
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                testArrayInteractive();
                break;
            case 2:
                testLinkedListInteractive();
                break;
            case 3:
                testBothInteractive();
                break;
            case 4:
                std::cout << "Exiting..." << std::endl;
                break;
            default:
                std::cout << "Invalid choice!" << std::endl;
        }
    }
    
    void testArrayInteractive() {
        ArrayJobMatcher matcher;
        
        std::cout << "Loading data..." << std::endl;
        matcher.loadJobsFromCSV("job_description.csv");
        matcher.loadResumesFromCSV("resume.csv");
        
        int resumeId;
        std::cout << "Enter resume ID to find matches (1-" << matcher.getResumeCount() << "): ";
        std::cin >> resumeId;
        
        if (resumeId >= 1 && resumeId <= matcher.getResumeCount()) {
            auto matches = matcher.findMatches(resumeId, 10);
            matcher.displayTopMatches(matches, 10);
        } else {
            std::cout << "Invalid resume ID!" << std::endl;
        }
    }
    
    void testLinkedListInteractive() {
        LinkedListJobMatcher matcher;
        
        std::cout << "Loading data..." << std::endl;
        matcher.loadJobsFromCSV("job_description.csv");
        matcher.loadResumesFromCSV("resume.csv");
        
        int resumeId;
        std::cout << "Enter resume ID to find matches (1-" << matcher.getResumeCount() << "): ";
        std::cin >> resumeId;
        
        if (resumeId >= 1 && resumeId <= matcher.getResumeCount()) {
            auto matches = matcher.findMatches(resumeId, 10);
            matcher.displayTopMatches(matches, 10);
        } else {
            std::cout << "Invalid resume ID!" << std::endl;
        }
    }
    
    void testBothInteractive() {
        std::cout << "Testing both implementations with same data..." << std::endl;
        
        int resumeId;
        std::cout << "Enter resume ID to test (1-10000): ";
        std::cin >> resumeId;
        
        // Test Array
        std::cout << "\n--- Array Implementation ---" << std::endl;
        ArrayJobMatcher arrayMatcher;
        arrayMatcher.loadJobsFromCSV("job_description.csv");
        arrayMatcher.loadResumesFromCSV("resume.csv");
        
        auto start = std::chrono::high_resolution_clock::now();
        auto arrayMatches = arrayMatcher.findMatches(resumeId, 5);
        auto end = std::chrono::high_resolution_clock::now();
        double arrayTime = std::chrono::duration<double, std::milli>(end - start).count();
        
        arrayMatcher.displayTopMatches(arrayMatches, 5);
        std::cout << "Array search time: " << arrayTime << " ms" << std::endl;
        
        // Test Linked List
        std::cout << "\n--- Linked List Implementation ---" << std::endl;
        LinkedListJobMatcher linkedListMatcher;
        linkedListMatcher.loadJobsFromCSV("job_description.csv");
        linkedListMatcher.loadResumesFromCSV("resume.csv");
        
        start = std::chrono::high_resolution_clock::now();
        auto linkedListMatches = linkedListMatcher.findMatches(resumeId, 5);
        end = std::chrono::high_resolution_clock::now();
        double linkedListTime = std::chrono::duration<double, std::milli>(end - start).count();
        
        linkedListMatcher.displayTopMatches(linkedListMatches, 5);
        std::cout << "Linked List search time: " << linkedListTime << " ms" << std::endl;
        
        // Compare results
        std::cout << "\n--- Comparison ---" << std::endl;
        std::cout << "Time difference: " << std::abs(linkedListTime - arrayTime) << " ms" << std::endl;
        if (arrayTime < linkedListTime) {
            std::cout << "Array implementation is " << (linkedListTime / arrayTime) << "x faster" << std::endl;
        } else {
            std::cout << "Linked List implementation is " << (arrayTime / linkedListTime) << "x faster" << std::endl;
        }
    }
};

int main() {
    try {
        PerformanceAnalyzer analyzer;
        analyzer.runComprehensiveTest();
        
        std::cout << "\n===============================================" << std::endl;
        std::cout << "    JOB MATCHING SYSTEM TEST COMPLETED" << std::endl;
        std::cout << "===============================================" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

