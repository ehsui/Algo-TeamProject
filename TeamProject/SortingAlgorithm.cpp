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
#include <algorithm>        // std::sort, std::swap
#include <utility>          // std::pair, std::move
#include <unordered_map>    // std::unordered_map

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
    // 정렬된 배열에서 앞쪽(낮은 인덱스)으로 이동 (점수가 올랐을 때)
    // key의 operator<는 내림차순 (높은 점수가 앞)
    while (idx > 0 && cur[idx] < cur[idx - 1]) {
        std::swap(cur[idx], cur[idx - 1]);
        // 위치 맵 업데이트
        pos[cur[idx].videoId] = idx;
        pos[cur[idx - 1].videoId] = idx - 1;
        idx--;
    }
}

void RankEngine::shiftDown(int idx) {
    // 정렬된 배열에서 뒤쪽(높은 인덱스)으로 이동 (점수가 내렸을 때)
    int size = static_cast<int>(cur.size());
    while (idx < size - 1 && cur[idx + 1] < cur[idx]) {
        std::swap(cur[idx], cur[idx + 1]);
        // 위치 맵 업데이트
        pos[cur[idx].videoId] = idx;
        pos[cur[idx + 1].videoId] = idx + 1;
        idx++;
    }
}

void RankEngine::adjust(int idx) {
    // 이웃과 비교하여 위치 조정
    int size = static_cast<int>(cur.size());
    if (idx < 0 || idx >= size) return;
    
    // 앞쪽 이웃보다 점수가 높으면 앞으로 이동
    if (idx > 0 && cur[idx] < cur[idx - 1]) {
        shiftUp(idx);
    }
    // 뒤쪽 이웃보다 점수가 낮으면 뒤로 이동
    else if (idx < size - 1 && cur[idx + 1] < cur[idx]) {
        shiftDown(idx);
    }
}

void RankEngine::build_onlineInsert() {
    // OnlineInsert 전략 초기 빌드
    // 초기 데이터는 효율적인 정렬로 처리하고, 이후 갱신은 refresh_onlineInsert()로 처리
    
    if (cur.empty() || P.k <= 0) return;
    
    int k = std::min(P.k, static_cast<int>(cur.size()));
    
    // 초기 빌드는 selectThenSort 방식 사용 (효율성)
    SelectAlgorithm algo = toSelectAlgorithm(P.sel);
    vector<key> topK = selectTopK(cur, k, algo);
    cur = std::move(topK);
    applySortAlgorithm(cur, P.s);
    
    // 위치 맵 구축 (이후 실시간 갱신용)
    rebuildPosMap();
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

// ============================================================================
// 실시간 삽입정렬 (OnlineInsert 전략)
// ============================================================================

void RankEngine::rebuildPosMap() {
    this->pos.clear();
    for (size_t i = 0; i < this->cur.size(); ++i) {
        this->pos[this->cur[i].videoId] = static_cast<int>(i);
    }
}

void RankEngine::adjustNeighbor(int idx) {
    // 이웃과 비교하여 올바른 위치로 이동
    // shiftUp/shiftDown 헬퍼 함수 사용
    
    int size = static_cast<int>(cur.size());
    if (idx < 0 || idx >= size) return;
    
    // 앞쪽 이웃보다 점수가 높으면 앞으로 이동 (점수가 올랐을 때)
    if (idx > 0 && cur[idx] < cur[idx - 1]) {
        shiftUp(idx);
    }
    // 뒤쪽 이웃보다 점수가 낮으면 뒤로 이동 (점수가 내렸을 때)
    else if (idx < size - 1 && cur[idx + 1] < cur[idx]) {
        shiftDown(idx);
    }
}

int RankEngine::insertSorted(const key& item) {
    // 이진 탐색으로 삽입 위치 찾기
    // key의 operator<는 내림차순이므로, 높은 점수가 앞에 와야 함
    
    int left = 0, right = static_cast<int>(this->cur.size());
    
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (this->cur[mid] < item) {
            // cur[mid]가 item보다 뒤에 와야 함 (item 점수가 더 낮음)
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    // left 위치에 삽입
    this->cur.insert(this->cur.begin() + left, item);
    
    // 삽입된 위치 이후 모든 요소의 위치 맵 업데이트
    for (int i = left; i < static_cast<int>(this->cur.size()); ++i) {
        this->pos[this->cur[i].videoId] = i;
    }
    
    return left;
}

// ============================================================================
// 통합 갱신 메서드
// ============================================================================

void RankEngine::refresh(vector<Video>& newData) {
    // 직전 랭킹 저장
    savePrevRanking();
    refreshCount++;
    
    // 시간 측정 시작
    auto start = std::chrono::high_resolution_clock::now();
    
    // 방식에 따라 갱신 전략 선택
    switch (P.a) {
        case AlgorithmType::BasicSort:
            refresh_sortAll(newData);
            break;
        case AlgorithmType::SelectThenSort:
            refresh_selectThenSort(newData);
            break;
        case AlgorithmType::AVLTree:
            refresh_AVLTree(newData);
            break;
        case AlgorithmType::OnlineInsert:
            refresh_onlineInsert(newData);
            break;
        case AlgorithmType::MultiMetric:
            refresh_MultiMetric(newData);
            break;
    }
    
    // 시간 측정 종료
    auto end = std::chrono::high_resolution_clock::now();
    refreshTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
}

// ============================================================================
// 방식별 갱신 전략
// ============================================================================

void RankEngine::refresh_sortAll(vector<Video>& newData) {
    /**
     * BasicSort 갱신: 전체 재정렬
     * 시간복잡도: O(n log n)
     */
    setData(cur, newData);
    build_sortAll();
    rebuildPosMap();
}

void RankEngine::refresh_selectThenSort(vector<Video>& newData) {
    /**
     * SelectThenSort 갱신: 선택 후 정렬
     * 시간복잡도: O(n + k log k)
     */
    setData(cur, newData);
    build_selectThenSort();
    rebuildPosMap();
}

void RankEngine::refresh_AVLTree(vector<Video>& newData) {
    /**
     * AVLTree 갱신: AVL 트리 재구축
     * 시간복잡도: O(n log n)
     */
    setData(cur, newData);
    build_AVLTree();
    rebuildPosMap();
}

void RankEngine::refresh_MultiMetric(vector<Video>& newData) {
    /**
     * MultiMetric 갱신: 다중 지표 재정렬
     * 시간복잡도: O(n log n)
     */
    curMulti.clear();
    curMulti.reserve(newData.size());
    for (const Video& v : newData) {
        MultiMetricKey mmk = createMultiMetricKey(
            v.videoId,
            v.title,
            v.viewCount,
            v.likeCount,
            v.commentCount,
            P.metricConfig
        );
        curMulti.push_back(mmk);
    }
    build_MultiMetric();
}

void RankEngine::refresh_onlineInsert(vector<Video>& newData) {
    /**
     * OnlineInsert 갱신: 부분 갱신 (효율적)
     * 
     * [개요]
     * 기존 정렬된 배열(cur)에서:
     * 1. 갱신된 영상: 점수 업데이트 후 이웃 비교로 위치 조정
     * 2. 사라진 영상: 제거
     * 3. 새 영상: 이진 탐색으로 올바른 위치에 삽입
     * 
     * [시간 복잡도]
     * - 기존 영상 업데이트: O(m × d) (m: 변경 수, d: 평균 이동 거리)
     * - 새 영상 삽입: O(n × log k)
     * - 전체: O(m × d + n log k), 평균적으로 전체 재정렬보다 빠름
     */
    
    if (cur.empty()) {
        // 초기 상태면 일반 빌드
        setData(cur, newData);
        build_selectThenSort();
        return;
    }
    
    // newData의 videoId를 빠르게 조회하기 위한 맵
    // videoId → (새 점수, 처리 여부)
    std::unordered_map<string, std::pair<Score, bool>> newDataMap;
    for (const Video& v : newData) {
        newDataMap[v.videoId] = {v.score, false};
    }
    
    // === 1단계: 기존 영상 처리 ===
    vector<int> removedIndices;  // 사라진 영상 위치들
    
    for (int i = 0; i < static_cast<int>(cur.size()); ++i) {
        auto it = newDataMap.find(cur[i].videoId);
        
        if (it != newDataMap.end()) {
            // 기존 영상이 갱신 데이터에 존재 → 점수 업데이트
            Score oldScore = cur[i].Value;
            Score newScore = it->second.first;
            
            if (oldScore != newScore) {
                cur[i].Value = newScore;
                adjustNeighbor(i);  // 이웃과 비교하여 위치 조정
                
                // 위치가 변경되었을 수 있으므로 i 재조정
                i = pos[cur[i].videoId];
            }
            
            it->second.second = true;  // 처리됨 표시
        } else {
            // 기존 영상이 갱신 데이터에 없음 → 사라진 영상
            removedIndices.push_back(i);
        }
    }
    
    // === 2단계: 새 영상 수집 ===
    vector<key> newItems;
    for (const Video& v : newData) {
        auto it = newDataMap.find(v.videoId);
        if (it != newDataMap.end() && !it->second.second) {
            // 처리되지 않은 영상 = 새 영상
            newItems.push_back(v.makekey());
        }
    }
    
    // === 3단계: 사라진 영상 제거 및 새 영상 삽입 ===
    if (!removedIndices.empty()) {
        // 뒤에서부터 제거 (인덱스 꼬임 방지)
        std::sort(removedIndices.rbegin(), removedIndices.rend());
        for (int idx : removedIndices) {
            pos.erase(cur[idx].videoId);
            cur.erase(cur.begin() + idx);
        }
        // 위치 맵 재구축
        rebuildPosMap();
    }
    
    // 새 영상 삽입 (이진 탐색으로 올바른 위치에)
    for (const key& item : newItems) {
        insertSorted(item);
    }
    
    // === 4단계: Top-K 유지 ===
    if (static_cast<int>(cur.size()) > P.k) {
        // 넘치는 부분 제거
        for (size_t i = P.k; i < cur.size(); ++i) {
            pos.erase(cur[i].videoId);
        }
        cur.resize(P.k);
    }
}
