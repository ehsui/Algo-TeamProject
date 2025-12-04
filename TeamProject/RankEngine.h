#pragma once
/**
 * @file RankEngine.h
 * @brief 랭킹 엔진
 * 
 * [지원 기능]
 * - 정렬 알고리즘 선택 (8가지)
 * - 선택 알고리즘 선택 (4가지)
 * - Top-K 랭킹 생성
 * - 다중 지표 사전식 랭킹
 * 
 * [알고리즘 조합]
 * - BasicSort: 전체 정렬 후 상위 K개 추출
 * - SelectThenSort: 선택 알고리즘으로 K개 찾고 정렬
 * - MultiMetric: 다중 지표 사전식 랭킹
 */

#include "Utility.h"
#include "Video.h"
#include "MultiMetric.h"
#include <unordered_map>

// ============================================================================
// 알고리즘 설정
// ============================================================================

/// 랭킹 방식 이름
inline const string AlgoName[5] = {
    "BasicSort", "SelectThenSort", "AVLTree", "OnlineInsert", "MultiMetric"
};

/// 정렬 알고리즘 이름
inline const string sortname[8] = {
    "Selection Sort", "Bubble Sort", "Quick Sort", "Merge Sort",
    "Shell Sort", "Heap Sort", "Counting Sort", "Radix Sort"
};

/// 선택 알고리즘 이름
inline const string selectname[4] = {
    "Sequential (Heap)", "Quick Select", "Binary Select", "std::nth_element"
};

/// 랭킹 방식 타입
enum AlgorithmType {
    BasicSort = 0,      // 기본 정렬 (전체 정렬)
    SelectThenSort,     // 선택 후 정렬
    AVLTree,            // AVL 트리 (향후 구현)
    OnlineInsert,       // 온라인 삽입
    MultiMetric         // 다중 지표 사전식 랭킹
};

/// 정렬 알고리즘 타입
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

/// 선택 알고리즘 타입
enum SelectType {
    select_sequential = 0,  // 힙 기반 순차 선택 O(n log k)
    select_quick,           // Quick Select O(n) 평균
    select_binary,          // 이진 탐색 기반 (정수 전용)
    select_nth              // std::nth_element
};

/**
 * @struct RankPolicy
 * @brief 랭킹 정책 설정
 */
struct RankPolicy {
    AlgorithmType a = BasicSort;        // 랭킹 방식
    SortType s = sort_quick;            // 정렬 알고리즘
    SelectType sel = select_nth;        // 선택 알고리즘
    int k = 100;                        // Top-K 개수
    MultiMetricConfig metricConfig;     // 다중 지표 설정 (MultiMetric 모드용)
};

/**
 * @class RankEngine
 * @brief 랭킹 엔진 클래스
 */
class RankEngine {
public:
    explicit RankEngine(RankPolicy policy);
    ~RankEngine() = default;
    
    // === 인터페이스 ===
    void interface(vector<Video>& Src);
    void sortInterface();
    void selectInterface();      // 선택 알고리즘 인터페이스
    void multiMetricInterface(); // 다중 지표 설정 인터페이스
    
    // === 데이터 관리 ===
    void setData(vector<key>& Data, vector<Video>& Src);
    void build();
    
    // === 조회 ===
    void mapping(string videoId, int rank);
    void updateScore(const string& videoId, Score newScore);
    vector<key> getTopK() const;
    
    // === Getter ===
    const RankPolicy& getPolicy() const { return P; }
    
private:
    RankPolicy P;
    vector<key> cur;
    vector<MultiMetricKey> curMulti;  // 다중 지표 모드용
    unordered_map<string, int> pos;   // videoId → index 매핑
    
    // === 빌드 전략 ===
    void build_sortAll();           // 전체 정렬
    void build_selectThenSort();    // 선택 후 정렬
    void build_onlineInsert();      // 온라인 삽입 정렬
    void build_AVLTree();           // AVL 트리 기반
    void build_MultiMetric();       // 다중 지표 사전식
    
    // === 힙 유틸리티 ===
    void shiftUp(int idx);
    void shiftDown(int idx);
    void adjust(int idx);
};