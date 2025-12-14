#pragma once
/**
 * @file RankEngine.h
 * @brief Ranking Engine
 * 
 * [Supported Features]
 * - Sorting algorithm selection (8 types)
 * - Selection algorithm selection (4 types)
 * - Scoring strategy selection (4 types)
 * - Top-K ranking generation
 * - Multi-metric lexicographic ranking
 * - Real-time updates with time measurement
 * - Benchmark history recording
 * 
 * [Algorithm Combinations]
 * - BasicSort: Full sort then extract top K
 * - SelectThenSort: Find K with selection then sort
 * - OnlineInsert: Partial update for changed data
 * - MultiMetric: Multi-metric lexicographic ranking
 */

#include "Utility.h"
#include "Video.h"
#include "MultiMetric.h"
#include "AVLTree.hpp"
#include "BenchmarkHistory.h"
#include <unordered_map>
#include <chrono>

// ============================================================================
// Algorithm Configuration
// ============================================================================

/// Ranking method names
inline const string AlgoName[5] = {
    "BasicSort", "SelectThenSort", "AVLTreeRank", "OnlineInsert", "MultiMetric"
};

/// Sorting algorithm names
inline const string sortname[8] = {
    "Selection Sort", "Bubble Sort", "Quick Sort", "Merge Sort",
    "Shell Sort", "Heap Sort", "Counting Sort", "Radix Sort"
};

/// Selection algorithm names
inline const string selectname[4] = {
    "Sequential (Heap)", "Quick Select", "Binary Select", "std::nth_element"
};

/// Ranking method types
enum AlgorithmType {
    BasicSort = 0,      // Basic sort (full sort)
    SelectThenSort,     // Select then sort
    AVLTreeRank,        // AVL Tree based ranking
    OnlineInsert,       // Online insertion
    MultiMetric         // Multi-metric lexicographic ranking
};

/// Sorting algorithm types
enum SortType {
    sort_selection = 0,
    sort_bubble,
    sort_quick,
    sort_merge,
    sort_shell,
    sort_heap,
    sort_counting,
    sort_radix
};

/// Selection algorithm types
enum SelectType {
    select_sequential = 0,  // Heap-based sequential O(n log k)
    select_quick,           // Quick Select O(n) avg
    select_binary,          // Binary search based (integers only)
    select_nth              // std::nth_element
};

/**
 * @struct RankPolicy
 * @brief Ranking policy configuration
 */
struct RankPolicy {
    AlgorithmType a = BasicSort;        // Ranking method
    SortType s = sort_quick;            // Sorting algorithm
    SelectType sel = select_nth;        // Selection algorithm
    ScoringStrategy scoring = ScoringStrategy::Engagement;  // Scoring strategy
    int k = 100;                        // Top-K count
    MultiMetricConfig metricConfig;     // Multi-metric config (for MultiMetric mode)
};

/**
 * @class RankEngine
 * @brief Ranking Engine Class
 */
class RankEngine {
public:
    explicit RankEngine(RankPolicy policy);
    ~RankEngine() = default;
    
    // === Interface ===
    void interface(vector<Video>& Src);   // Use external data
    void interface();                      // Load from CSV file
    void sortInterface();
    void selectInterface();       // Selection algorithm interface
    void scoringInterface();      // Scoring strategy interface
    void multiMetricInterface();  // Multi-metric config interface
    void resultMenu();            // Result menu (after build)
    
    // === Data Management ===
    void setData(vector<key>& Data, vector<Video>& Src);
    void build();
    
    // === Refresh (applies to all methods) ===
    /**
     * @brief Refresh data (processed according to current method)
     * @param newData Updated video data
     */
    void refresh(vector<Video>& newData);
    
    // === Method-specific refresh strategies ===
    void refresh_sortAll(vector<Video>& newData);        // Full re-sort
    void refresh_selectThenSort(vector<Video>& newData); // Select then sort
    void refresh_onlineInsert(vector<Video>& newData);   // Partial update
    void refresh_AVLTree(vector<Video>& newData);        // AVL Tree
    void refresh_MultiMetric(vector<Video>& newData);    // Multi-metric
    
    // === Output ===
    void printRanking();       // Print current ranking
    void printPrevRanking();   // Print previous ranking
    void printTimeStats();     // Print time statistics
    
    // === Query ===
    void mapping(string videoId, int rank);
    void updateScore(const string& videoId, Score newScore);
    vector<key> getTopK() const;
    
    // === Getters/Setters ===
    const RankPolicy& getPolicy() const { return P; }
    double getBuildTime() const { return buildTimeMs; }
    double getRefreshTime() const { return refreshTimeMs; }
    void setDataSourceType(DataSourceType type) { dataSourceType = type; }
    DataSourceType getDataSourceType() const { return dataSourceType; }
    
private:
    RankPolicy P;
    vector<key> cur;
    vector<key> prevCur;                  // Previous ranking (for comparison)
    vector<MultiMetricKey> curMulti;      // For multi-metric mode
    vector<MultiMetricKey> prevMulti;     // Previous multi-metric ranking
    unordered_map<string, int> pos;       // videoId → index mapping
    
    // === AVL Tree (for AVLTreeRank mode) ===
    using KeyComparator = std::function<bool(const key&, const key&)>;
    using KeyExtractor = std::function<std::string(const key&)>;
    DataStructure::RankAVLTree<key, KeyComparator, KeyExtractor> avlTree;
    
    // === CSV Data Storage ===
    vector<Video> srcData;                // Initial data (for build)
    vector<Video> refreshData;            // Refresh data (for two-timestamp)
    bool hasRefreshData = false;          // Has refresh data
    DataSourceType dataSourceType = DataSourceType::Dummy;  // Data source type for benchmark
    
    // === Time Measurement ===
    double buildTimeMs = 0.0;             // Initial build time (ms)
    double refreshTimeMs = 0.0;           // Refresh time (ms)
    int refreshCount = 0;                 // Refresh count
    
    // === Build Strategies ===
    void build_sortAll();           // Full sort
    void build_selectThenSort();    // Select then sort
    void build_onlineInsert();      // Online insertion sort
    void build_AVLTree();           // AVL Tree based
    void build_MultiMetric();       // Multi-metric lexicographic
    
    // === Sorted Array Utilities ===
    void shiftUp(int idx);
    void shiftDown(int idx);
    void adjust(int idx);
    
    // === Online Insertion Sort Utilities ===
    /**
     * @brief Move element to correct position by comparing with neighbors
     */
    void adjustNeighbor(int idx);
    
    /**
     * @brief Insert new element into sorted array
     * @return Inserted position
     */
    int insertSorted(const key& item);
    
    /**
     * @brief Rebuild position map
     */
    void rebuildPosMap();
    
    /**
     * @brief Save previous ranking
     */
    void savePrevRanking();
    
    // === Lazy Delete Utilities ===
    vector<int> emptySlots;           // Empty slot indices
    
    /**
     * @brief Mark position as deleted (no physical removal)
     */
    void markDeleted(int idx);
    
    /**
     * @brief Check if position is deleted
     */
    bool isDeleted(int idx) const;
    
    /**
     * @brief Place new element in empty slot and adjust position
     * @return Final position
     */
    int placeInEmptySlot(const key& item);
    
    /**
     * @brief Compact array (remove empty slots) - call when needed
     */
    void compactArray();
};
