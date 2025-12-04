/**
 * @file SortingAlgorithm.cpp
 * @brief RankEngine의 정렬/선택 빌드 메서드 구현
 * 
 * 템플릿 기반 BasicSort.hpp, BasicSelect.hpp를 사용합니다.
 */

#include "Utility.h"
#include "RankEngine.h"
#include "Video.h"
#include "BasicSort.hpp"    // 템플릿 정렬 함수
#include "BasicSelect.hpp"  // 템플릿 선택 함수

// ============================================================================
// 헬퍼: 정렬 알고리즘 적용
// ============================================================================
static void applySortAlgorithm(vector<key>& data, SortType s) {
    switch (s) {
        case SortType::sort_selection:
            selectsort(data);
            break;
        case SortType::sort_bubble:
            bubblesort(data);
            break;
        case SortType::sort_quick:
            quicksort(data);
            break;
        case SortType::sort_merge:
            mergesort(data);
            break;
        case SortType::sort_shell:
            shellSort(data);
            break;
        case SortType::sort_heap:
            heapSort(data);
            break;
        case SortType::sort_counting:
        case SortType::sort_radix:
        default:
            // countingSort/radixSort는 정수 전용이므로 key에 사용 불가
            std::sort(data.begin(), data.end());
            break;
    }
}

// ============================================================================
// 헬퍼: 선택 알고리즘을 SelectAlgorithm으로 변환
// ============================================================================
static SelectAlgorithm toSelectAlgorithm(SelectType sel) {
    switch (sel) {
        case SelectType::select_sequential: return SelectAlgorithm::Sequential;
        case SelectType::select_quick:      return SelectAlgorithm::QuickSelect;
        case SelectType::select_binary:     return SelectAlgorithm::BinarySelect;
        case SelectType::select_nth:
        default:                            return SelectAlgorithm::NthElement;
    }
}

// ============================================================================
// 빌드 메서드 구현
// ============================================================================

void RankEngine::build_sortAll() {
    // 전체 정렬 후 상위 K개 유지
    applySortAlgorithm(cur, P.s);
    
    // 상위 K개만 유지
    if (static_cast<int>(cur.size()) > P.k) {
        cur.resize(P.k);
    }
}

void RankEngine::build_selectThenSort() {
    // 1. 선택 알고리즘으로 상위 K개 추출
    // 2. 추출된 K개만 정렬
    
    if (cur.empty() || P.k <= 0) return;
    
    int k = std::min(P.k, static_cast<int>(cur.size()));
    SelectAlgorithm algo = toSelectAlgorithm(P.sel);
    
    // 선택 알고리즘으로 Top-K 추출
    vector<key> topK = selectTopK(cur, k, algo);
    
    // 결과를 cur로 교체
    cur = std::move(topK);
    
    // 선택 알고리즘이 이미 정렬된 결과를 반환하지만,
    // 사용자가 선택한 정렬 알고리즘으로 다시 정렬 (일관성)
    applySortAlgorithm(cur, P.s);
}

void RankEngine::shiftUp(int idx) {
    // 힙에서 위로 이동 (삽입 정렬용)
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (cur[idx] < cur[parent]) {
            std::swap(cur[idx], cur[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void RankEngine::shiftDown(int idx) {
    // 힙에서 아래로 이동
    int size = static_cast<int>(cur.size());
    while (true) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;
        
        if (left < size && cur[left] < cur[smallest]) smallest = left;
        if (right < size && cur[right] < cur[smallest]) smallest = right;
        
        if (smallest != idx) {
            std::swap(cur[idx], cur[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }
}

void RankEngine::adjust(int idx) {
    // 위치 조정 (삽입/삭제 후)
    if (idx > 0 && cur[idx] < cur[(idx - 1) / 2]) {
        shiftUp(idx);
    } else {
        shiftDown(idx);
    }
}

void RankEngine::build_onlineInsert() {
    // 온라인 삽입 정렬 (실시간 업데이트용)
    // 이미 정렬된 상태에서 새 요소 삽입
    for (size_t i = 1; i < cur.size(); i++) {
        key temp = cur[i];
        int j = static_cast<int>(i) - 1;
        
        while (j >= 0 && temp < cur[j]) {
            cur[j + 1] = cur[j];
            j--;
        }
        cur[j + 1] = temp;
    }
}

void RankEngine::build_AVLTree() {
    // AVL 트리 기반 정렬은 별도 구현 필요
    // 현재는 std::sort로 대체
    std::sort(cur.begin(), cur.end());
}

void RankEngine::build_MultiMetric() {
    // 다중 지표 사전식 랭킹
    // curMulti에 이미 다중 지표 키가 설정되어 있어야 함
    
    if (curMulti.empty()) return;
    
    int k = std::min(P.k, static_cast<int>(curMulti.size()));
    
    // Top-K 선택 후 정렬
    curMulti = MultiMetricSort::selectTopK(curMulti, k);
    
    // cur도 동기화 (호환성)
    cur.clear();
    cur.reserve(curMulti.size());
    for (const auto& mmk : curMulti) {
        key k;
        k.videoId = mmk.videoId;
        k.title = mmk.title;
        k.Value = mmk.metrics.empty() ? 0 : static_cast<Score>(mmk.metrics[0]);
        cur.push_back(k);
    }
}