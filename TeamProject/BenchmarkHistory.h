#pragma once
/**
 * @file BenchmarkHistory.h
 * @brief Benchmark Record System for 5 Ranking Algorithms
 * 
 * Records each ranking operation's performance data including:
 * - Data type (Real/Dummy)
 * - Algorithm used
 * - Time complexity
 * - Execution time
 * - Data size
 */

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

// ============================================================================
// Data Types
// ============================================================================

enum class DataSourceType {
    Real,   // Real data from YouTube API
    Dummy   // Generated dummy data
};

inline std::string getDataSourceName(DataSourceType type) {
    return (type == DataSourceType::Real) ? "Real" : "Dummy";
}

// ============================================================================
// Time Complexity Information
// ============================================================================

struct TimeComplexity {
    std::string build;    // Build time complexity
    std::string refresh;  // Refresh time complexity
    std::string note;     // Additional note
    
    std::string toString() const {
        return build;
    }
};

inline TimeComplexity getTimeComplexity(int algoType, int sortType = -1, int selectType = -1) {
    TimeComplexity tc;
    
    // Sort algorithm complexities
    const char* sortComplexities[] = {
        "O(n^2)",      // Selection Sort
        "O(n^2)",      // Bubble Sort
        "O(n log n)",  // Quick Sort (average)
        "O(n log n)",  // Merge Sort
        "O(n log^2 n)", // Shell Sort
        "O(n log n)",  // Heap Sort
        "O(n + k)",    // Counting Sort
        "O(d(n+k))"    // Radix Sort
    };
    
    // Selection algorithm complexities
    const char* selectComplexities[] = {
        "O(n log k)",  // Sequential (Heap)
        "O(n)",        // Quick Select (average)
        "O(n log M)",  // Binary Select
        "O(n)"         // std::nth_element (average)
    };
    
    switch (algoType) {
        case 0:  // BasicSort
            if (sortType >= 0 && sortType < 8) {
                tc.build = sortComplexities[sortType];
            } else {
                tc.build = "O(n log n)";
            }
            tc.refresh = tc.build;
            tc.note = "Full sort";
            break;
            
        case 1:  // SelectThenSort
            if (selectType >= 0 && selectType < 4) {
                tc.build = std::string(selectComplexities[selectType]) + " + O(k log k)";
            } else {
                tc.build = "O(n) + O(k log k)";
            }
            tc.refresh = tc.build;
            tc.note = "Select K then sort";
            break;
            
        case 2:  // AVLTreeRank
            tc.build = "O(n log n)";
            tc.refresh = "O(m log n)";
            tc.note = "m = changed items";
            break;
            
        case 3:  // OnlineInsert
            if (selectType >= 0 && selectType < 4) {
                tc.build = std::string(selectComplexities[selectType]) + " + O(k log k)";
            } else {
                tc.build = "O(n log k) + O(k log k)";
            }
            tc.refresh = "O(m * k)";
            tc.note = "Partial update";
            break;
            
        case 4:  // MultiMetric
            tc.build = "O(n log n * d)";
            tc.refresh = "O(n log n * d)";
            tc.note = "d = metric count";
            break;
            
        default:
            tc.build = "O(?)";
            tc.refresh = "O(?)";
    }
    
    return tc;
}

// ============================================================================
// Benchmark Record
// ============================================================================

struct BenchmarkRecord {
    // Identification
    int recordId;
    
    // Data info
    DataSourceType dataType;
    int dataSize;
    int topK;
    
    // Algorithm info
    int algorithmType;        // 0-4: BasicSort, SelectThenSort, AVLTreeRank, OnlineInsert, MultiMetric
    int sortAlgorithm;        // 0-7 for sort type
    int selectAlgorithm;      // 0-3 for selection type
    int scoringStrategy;      // 0-3 for scoring type
    
    // Time info
    double buildTimeMs;
    double refreshTimeMs;
    bool isRefresh;           // Is this a refresh operation?
    
    // Computed
    TimeComplexity complexity;
    
    // Get algorithm name
    std::string getAlgorithmName() const {
        static const char* names[] = {"BasicSort", "SelectThenSort", "AVLTreeRank", "OnlineInsert", "MultiMetric"};
        if (algorithmType >= 0 && algorithmType < 5) {
            return names[algorithmType];
        }
        return "Unknown";
    }
    
    // Get sort algorithm name
    std::string getSortName() const {
        static const char* names[] = {
            "Selection", "Bubble", "Quick", "Merge", "Shell", "Heap", "Counting", "Radix"
        };
        if (sortAlgorithm >= 0 && sortAlgorithm < 8) {
            return names[sortAlgorithm];
        }
        return "-";
    }
    
    // Get select algorithm name
    std::string getSelectName() const {
        static const char* names[] = {"Heap", "QuickSel", "BinarySel", "nth_elem"};
        if (selectAlgorithm >= 0 && selectAlgorithm < 4) {
            return names[selectAlgorithm];
        }
        return "-";
    }
    
    // Get time per item (for fair comparison)
    double getTimePerItem() const {
        double time = isRefresh ? refreshTimeMs : buildTimeMs;
        return (dataSize > 0) ? (time / dataSize) : 0.0;
    }
    
    // Get effective time for comparison (time per item in microseconds)
    double getEffectiveTime() const {
        return getTimePerItem() * 1000.0;  // Convert to us/item
    }
};

// ============================================================================
// Benchmark History Manager
// ============================================================================

class BenchmarkHistory {
public:
    static BenchmarkHistory& getInstance() {
        static BenchmarkHistory instance;
        return instance;
    }
    
    // Add a new record
    void addRecord(const BenchmarkRecord& record) {
        BenchmarkRecord r = record;
        r.recordId = nextId++;
        r.complexity = getTimeComplexity(r.algorithmType, r.sortAlgorithm, r.selectAlgorithm);
        records.push_back(r);
    }
    
    // Get all records
    const std::vector<BenchmarkRecord>& getRecords() const {
        return records;
    }
    
    // Get records by data type
    std::vector<BenchmarkRecord> getRecordsByType(DataSourceType type) const {
        std::vector<BenchmarkRecord> result;
        for (const auto& r : records) {
            if (r.dataType == type) {
                result.push_back(r);
            }
        }
        return result;
    }
    
    // Check if empty
    bool isEmpty() const {
        return records.empty();
    }
    
    // Clear all records
    void clear() {
        records.clear();
        nextId = 1;
    }
    
    // Find fastest record (by time per item)
    const BenchmarkRecord* findFastest(DataSourceType type) const {
        const BenchmarkRecord* fastest = nullptr;
        double minTime = std::numeric_limits<double>::max();
        
        for (const auto& r : records) {
            if (r.dataType == type) {
                double effTime = r.getEffectiveTime();
                if (effTime < minTime && effTime > 0) {
                    minTime = effTime;
                    fastest = &r;
                }
            }
        }
        return fastest;
    }
    
    // Print formatted history
    void printHistory() const {
        if (records.empty()) {
            std::cout << "\n  No benchmark records yet.\n";
            std::cout << "  Run ranking operations first, then check back here.\n\n";
            return;
        }
        
        // Separate by data type
        auto realRecords = getRecordsByType(DataSourceType::Real);
        auto dummyRecords = getRecordsByType(DataSourceType::Dummy);
        
        std::cout << "\n";
        std::cout << "================================================================================\n";
        std::cout << "                         BENCHMARK HISTORY\n";
        std::cout << "================================================================================\n";
        
        // Print Real Data Records
        if (!realRecords.empty()) {
            std::cout << "\n  [REAL DATA]\n";
            std::cout << "  ---------------------------------------------------------------------------\n";
            printRecordTable(realRecords);
            
            auto* fastest = findFastest(DataSourceType::Real);
            if (fastest) {
                std::cout << "\n  >> Fastest (Real): #" << fastest->recordId << " - "
                          << fastest->getAlgorithmName() << " (" 
                          << std::fixed << std::setprecision(3) 
                          << fastest->getEffectiveTime() << " us/item)\n";
            }
        }
        
        // Print Dummy Data Records
        if (!dummyRecords.empty()) {
            std::cout << "\n  [DUMMY DATA]\n";
            std::cout << "  ---------------------------------------------------------------------------\n";
            printRecordTable(dummyRecords);
            
            auto* fastest = findFastest(DataSourceType::Dummy);
            if (fastest) {
                std::cout << "\n  >> Fastest (Dummy): #" << fastest->recordId << " - "
                          << fastest->getAlgorithmName() << " ("
                          << std::fixed << std::setprecision(3)
                          << fastest->getEffectiveTime() << " us/item)\n";
            }
        }
        
        std::cout << "\n================================================================================\n";
    }
    
private:
    BenchmarkHistory() : nextId(1) {}
    
    void printRecordTable(const std::vector<BenchmarkRecord>& recs) const {
        // Header
        std::cout << "  " << std::left 
                  << std::setw(4) << "#"
                  << std::setw(16) << "Algorithm"
                  << std::setw(10) << "Sort"
                  << std::setw(10) << "Select"
                  << std::setw(8) << "Size"
                  << std::setw(6) << "K"
                  << std::setw(14) << "Complexity"
                  << std::setw(12) << "Time(ms)"
                  << "\n";
        std::cout << "  " << std::string(78, '-') << "\n";
        
        // Rows
        for (const auto& r : recs) {
            std::string typeMarker = r.isRefresh ? "(R)" : "(B)";
            
            std::cout << "  " << std::left
                      << std::setw(4) << r.recordId
                      << std::setw(16) << (r.getAlgorithmName() + typeMarker)
                      << std::setw(10) << r.getSortName()
                      << std::setw(10) << r.getSelectName()
                      << std::setw(8) << r.dataSize
                      << std::setw(6) << r.topK
                      << std::setw(14) << r.complexity.build;
            
            double time = r.isRefresh ? r.refreshTimeMs : r.buildTimeMs;
            std::cout << std::right << std::setw(10) << std::fixed << std::setprecision(2) << time;
            std::cout << "\n";
        }
        
        std::cout << "\n  (B) = Build, (R) = Refresh\n";
    }
    
    std::vector<BenchmarkRecord> records;
    int nextId;
};

// ============================================================================
// Global Helper Function
// ============================================================================

inline void addBenchmarkRecord(
    DataSourceType dataType,
    int dataSize,
    int topK,
    int algorithmType,
    int sortAlgorithm,
    int selectAlgorithm,
    int scoringStrategy,
    double buildTimeMs,
    double refreshTimeMs,
    bool isRefresh
) {
    BenchmarkRecord record;
    record.dataType = dataType;
    record.dataSize = dataSize;
    record.topK = topK;
    record.algorithmType = algorithmType;
    record.sortAlgorithm = sortAlgorithm;
    record.selectAlgorithm = selectAlgorithm;
    record.scoringStrategy = scoringStrategy;
    record.buildTimeMs = buildTimeMs;
    record.refreshTimeMs = refreshTimeMs;
    record.isRefresh = isRefresh;
    
    BenchmarkHistory::getInstance().addRecord(record);
}

