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
 * - 실시간 갱신 및 시간 측정
 * 
 * [알고리즘 조합]
 * - BasicSort: 전체 정렬 후 상위 K개 추출
 * - SelectThenSort: 선택 알고리즘으로 K개 찾고 정렬
 * - OnlineInsert: 변경된 데이터만 부분 정렬
 * - MultiMetric: 다중 지표 사전식 랭킹
 */

#include "Utility.h"
#include "Video.h"
#include "MultiMetric.h"
#include "AVLTree.hpp"
#include <unordered_map>
#include <chrono>

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
    void interface(vector<Video>& Src);   // 외부 데이터 사용
    void interface();                      // CSV 파일 로드
    void sortInterface();
    void selectInterface();      // 선택 알고리즘 인터페이스
    void multiMetricInterface(); // 다중 지표 설정 인터페이스
    void resultMenu();           // 결과 출력 후 메뉴 (내부 데이터 사용)
    
    // === 데이터 관리 ===
    void setData(vector<key>& Data, vector<Video>& Src);
    void build();
    
    // === 갱신 (모든 방식에 적용) ===
    /**
     * @brief 데이터 갱신 (현재 설정된 방식에 따라 처리)
     * @param newData 갱신된 영상 데이터
     */
    void refresh(vector<Video>& newData);
    
    // === 방식별 갱신 전략 ===
    void refresh_sortAll(vector<Video>& newData);       // 전체 재정렬
    void refresh_selectThenSort(vector<Video>& newData); // 선택 후 정렬
    void refresh_onlineInsert(vector<Video>& newData);   // 부분 갱신 (기존 refreshOnlineInsert)
    void refresh_AVLTree(vector<Video>& newData);        // AVL 트리
    void refresh_MultiMetric(vector<Video>& newData);    // 다중 지표
    
    // === 출력 ===
    void printRanking();       // 현재 랭킹 출력
    void printPrevRanking();   // 직전 랭킹 출력
    void printTimeStats();     // 시간 통계 출력
    
    // === 조회 ===
    void mapping(string videoId, int rank);
    void updateScore(const string& videoId, Score newScore);
    vector<key> getTopK() const;
    
    // === Getter ===
    const RankPolicy& getPolicy() const { return P; }
    double getBuildTime() const { return buildTimeMs; }
    double getRefreshTime() const { return refreshTimeMs; }
    
private:
    RankPolicy P;
    vector<key> cur;
    vector<key> prevCur;                  // 직전 랭킹 (비교용)
    vector<MultiMetricKey> curMulti;      // 다중 지표 모드용
    vector<MultiMetricKey> prevMulti;     // 직전 다중 지표 랭킹
    unordered_map<string, int> pos;       // videoId → index 매핑
    
    // === AVL 트리 (AVLTree 모드용) ===
    using KeyComparator = std::function<bool(const key&, const key&)>;
    using KeyExtractor = std::function<std::string(const key&)>;
    RankAVLTree<key, KeyComparator, KeyExtractor> avlTree;  // Order Statistics AVL 트리
    
    // === CSV 데이터 저장 ===
    vector<Video> srcData;                // 초기 데이터 (빌드용)
    vector<Video> refreshData;            // 갱신 데이터 (두 시점용)
    bool hasRefreshData = false;          // 갱신 데이터 존재 여부
    
    // === 시간 측정 ===
    double buildTimeMs = 0.0;             // 초기 빌드 시간 (ms)
    double refreshTimeMs = 0.0;           // 갱신 시간 (ms)
    int refreshCount = 0;                 // 갱신 횟수
    
    // === 빌드 전략 ===
    void build_sortAll();           // 전체 정렬
    void build_selectThenSort();    // 선택 후 정렬
    void build_onlineInsert();      // 온라인 삽입 정렬
    void build_AVLTree();           // AVL 트리 기반
    void build_MultiMetric();       // 다중 지표 사전식
    
    // === 정렬된 배열 유틸리티 ===
    void shiftUp(int idx);
    void shiftDown(int idx);
    void adjust(int idx);
    
    // === 실시간 삽입정렬 유틸리티 (OnlineInsert 전략용) ===
    /**
     * @brief 특정 위치의 요소를 이웃과 비교하여 올바른 위치로 이동
     * 점수가 올랐으면 앞으로, 내렸으면 뒤로 이동
     */
    void adjustNeighbor(int idx);
    
    /**
     * @brief 새 요소를 정렬된 배열에 삽입 (이진 탐색 + 삽입)
     * @return 삽입된 위치
     */
    int insertSorted(const key& item);
    
    /**
     * @brief 위치 맵 재구축
     */
    void rebuildPosMap();
    
    /**
     * @brief 직전 랭킹 저장
     */
    void savePrevRanking();
    
    // === Lazy Delete (빈자리 마킹) 유틸리티 ===
    vector<int> emptySlots;           // 빈자리 인덱스 목록
    
    /**
     * @brief 특정 위치를 빈자리로 마킹 (실제 삭제 안 함)
     */
    void markDeleted(int idx);
    
    /**
     * @brief 빈자리 여부 확인
     */
    bool isDeleted(int idx) const;
    
    /**
     * @brief 빈자리에 새 요소 배치 후 위치 조정
     * @return 최종 위치
     */
    int placeInEmptySlot(const key& item);
    
    /**
     * @brief 배열 압축 (빈자리 제거) - 필요시 호출
     */
    void compactArray();
};