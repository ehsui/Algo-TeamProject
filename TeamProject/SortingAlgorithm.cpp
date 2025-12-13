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
    /**
     * AVL 트리 기반 Top-K 랭킹
     * 
     * [Order Statistics AVL Tree]
     * - 각 노드에 서브트리 크기 저장 → O(log n)에 k번째 요소 조회
     * - ID → 노드 포인터 맵 → O(1)에 노드 접근
     * - 삽입/삭제/수정 모두 O(log n)
     * 
     * 시간복잡도: O(n log n) 빌드 + O(k) Top-K 추출
     */
    
    if (cur.empty() || P.k <= 0) return;
    
    // 비교자: 높은 점수 우선 (내림차순)
    KeyComparator comp = [](const key& a, const key& b) {
        if (a.Value != b.Value) {
            return a.Value > b.Value;  // 높은 점수가 앞으로
        }
        return a.title < b.title;      // 동점 시 제목순
    };
    
    // 키 추출자: videoId 반환
    KeyExtractor extractor = [](const key& k) -> std::string {
        return k.videoId;
    };
    
    // 새 트리로 초기화
    avlTree = RankAVLTree<key, KeyComparator, KeyExtractor>(comp, extractor);
    
    // 모든 요소 삽입 O(n log n)
    for (const auto& item : cur) {
        avlTree.insert(item);
    }
    
    // Top-K 추출 O(k)
    int k = std::min(P.k, static_cast<int>(avlTree.size()));
    vector<key> topK = avlTree.topK(k);
    cur = std::move(topK);
    
    // 위치 맵 구축 (호환성)
    rebuildPosMap();
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
// Lazy Delete (빈자리 마킹) 유틸리티
// ============================================================================

void RankEngine::markDeleted(int idx) {
    // 특정 위치를 빈자리로 마킹
    // videoId를 빈 문자열로 설정하여 삭제 표시
    if (idx < 0 || idx >= static_cast<int>(cur.size())) return;
    
    pos.erase(cur[idx].videoId);  // 위치 맵에서 제거
    cur[idx].videoId = "";        // 삭제 표시
    cur[idx].Value = -1;          // 최저 점수로 설정 (맨 뒤로 밀림)
    emptySlots.push_back(idx);    // 빈자리 목록에 추가
}

bool RankEngine::isDeleted(int idx) const {
    // 빈자리 여부 확인
    if (idx < 0 || idx >= static_cast<int>(cur.size())) return true;
    return cur[idx].videoId.empty();
}

int RankEngine::placeInEmptySlot(const key& item) {
    // 빈자리에 새 요소 배치 후 이웃 비교로 위치 조정
    
    if (emptySlots.empty()) {
        // 빈자리 없으면 맨 끝에 추가 후 shiftUp
        cur.push_back(item);
        int idx = static_cast<int>(cur.size()) - 1;
        pos[item.videoId] = idx;
        shiftUp(idx);
        return pos[item.videoId];
    }
    
    // 빈자리 중 하나 사용
    int slotIdx = emptySlots.back();
    emptySlots.pop_back();
    
    // 빈자리에 배치
    cur[slotIdx] = item;
    pos[item.videoId] = slotIdx;
    
    // 이웃 비교로 올바른 위치로 이동
    adjustNeighbor(slotIdx);
    
    return pos[item.videoId];
}

void RankEngine::compactArray() {
    // 배열 압축: 빈자리(삭제된 요소) 제거
    // 빈자리가 많이 쌓였을 때 호출
    
    if (emptySlots.empty()) return;
    
    // 삭제되지 않은 요소만 남기기
    vector<key> compacted;
    compacted.reserve(cur.size() - emptySlots.size());
    
    for (const key& k : cur) {
        if (!k.videoId.empty()) {  // 삭제되지 않은 것만
            compacted.push_back(k);
        }
    }
    
    cur = std::move(compacted);
    emptySlots.clear();
    rebuildPosMap();
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
     * AVLTree 갱신: 증분 업데이트 (Augmented AVL with ID Map)
     * 
     * [전략]
     * 1. 새 데이터를 ID 맵으로 변환
     * 2. 기존 트리 순회:
     *    - 새 데이터에 있으면 → 점수 비교 후 필요시 update() O(log n)
     *    - 새 데이터에 없으면 → 삭제 목록에 추가
     * 3. 삭제 목록의 항목 삭제 O(m log n)
     * 4. 새 데이터 중 기존에 없는 것 삽입 O(m log n)
     * 
     * 시간복잡도: O(m log n), m = 변경된 항목 수
     * - 전체 재빌드 O(n log n)보다 효율적 (m << n인 경우)
     */
    
    if (newData.empty()) return;
    
    // 새 데이터를 ID → key 맵으로 변환
    unordered_map<string, key> newDataMap;
    for (auto& v : newData) {
        v.calculateScore();
        newDataMap[v.videoId] = v.makekey();
    }
    
    // 트리가 비어있으면 초기 빌드
    if (avlTree.empty()) {
        for (const auto& pair : newDataMap) {
            avlTree.insert(pair.second);
        }
    } else {
        // 1. 삭제할 항목 & 업데이트할 항목 수집
        vector<string> toRemove;
        vector<pair<string, key>> toUpdate;
        
        avlTree.inorder([&](const key& item) {
            auto it = newDataMap.find(item.videoId);
            if (it == newDataMap.end()) {
                // 새 데이터에 없음 → 삭제 대상
                toRemove.push_back(item.videoId);
            } else {
                // 새 데이터에 있음 → 점수 변경 확인
                if (item.Value != it->second.Value) {
                    toUpdate.push_back({item.videoId, it->second});
                }
                // 처리 완료한 항목 표시 (나중에 삽입 대상에서 제외)
                newDataMap.erase(it);
            }
        });
        
        // 2. 삭제 수행 O(m log n)
        for (const string& id : toRemove) {
            avlTree.removeById(id);
        }
        
        // 3. 업데이트 수행 O(m log n)
        for (const auto& pair : toUpdate) {
            avlTree.update(pair.first, pair.second);
        }
        
        // 4. 새 항목 삽입 O(m log n)
        // newDataMap에 남아있는 것 = 기존에 없던 새 영상
        for (const auto& pair : newDataMap) {
            avlTree.insert(pair.second);
        }
    }
    
    // Top-K 추출 O(k)
    int k = std::min(P.k, static_cast<int>(avlTree.size()));
    cur = avlTree.topK(k);
    
    // 위치 맵 갱신
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
     * OnlineInsert 갱신: Lazy Delete 방식 (효율적)
     * 
     * [개요 - 빈자리 마킹 방식]
     * 기존 정렬된 배열(cur)에서:
     * 1. 갱신된 영상: 점수 업데이트 후 이웃 비교로 위치 조정
     * 2. 사라진 영상: 빈자리로 마킹 (실제 삭제 안 함)
     * 3. 새 영상: 빈자리에 배치 후 이웃 비교로 위치 조정
     * 
     * [시간 복잡도]
     * - 기존 영상 업데이트: O(m × d) (m: 변경 수, d: 평균 이동 거리)
     * - 사라진 영상 마킹: O(1) per item
     * - 새 영상 배치: O(d) per item (이웃 비교)
     * - 전체: O(m × d), 배열 이동 없이 효율적
     */
    
    if (cur.empty()) {
        // 초기 상태면 일반 빌드
        setData(cur, newData);
        build_selectThenSort();
        return;
    }
    
    // 빈자리 목록 초기화
    emptySlots.clear();
    
    // newData의 videoId를 빠르게 조회하기 위한 맵
    // videoId → (새 점수, 처리 여부)
    std::unordered_map<string, std::pair<Score, bool>> newDataMap;
    for (const Video& v : newData) {
        newDataMap[v.videoId] = {v.score, false};
    }
    
    // === 1단계: 기존 영상 처리 ===
    for (int i = 0; i < static_cast<int>(cur.size()); ++i) {
        // 이미 삭제된 슬롯은 건너뛰기
        if (isDeleted(i)) continue;
        
        auto it = newDataMap.find(cur[i].videoId);
        
        if (it != newDataMap.end()) {
            // 기존 영상이 갱신 데이터에 존재 → 점수 업데이트
            Score oldScore = cur[i].Value;
            Score newScore = it->second.first;
            
            if (oldScore != newScore) {
                cur[i].Value = newScore;
                adjustNeighbor(i);  // 이웃과 비교하여 위치 조정
                
                // 위치가 변경되었을 수 있으므로 i 재조정
                auto posIt = pos.find(cur[i].videoId);
                if (posIt != pos.end()) {
                    i = posIt->second;
                }
            }
            
            it->second.second = true;  // 처리됨 표시
        } else {
            // 기존 영상이 갱신 데이터에 없음 → 빈자리로 마킹
            markDeleted(i);
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
    
    // === 3단계: 새 영상을 빈자리에 배치 ===
    for (const key& item : newItems) {
        placeInEmptySlot(item);  // 빈자리에 배치 + 이웃 비교로 위치 조정
    }
    
    // === 4단계: 빈자리 정리 (남은 빈자리가 있으면) ===
    // 빈자리가 많이 남아있으면 압축
    if (!emptySlots.empty()) {
        compactArray();
    }
    
    // === 5단계: Top-K 유지 ===
    if (static_cast<int>(cur.size()) > P.k) {
        // 넘치는 부분 제거
        for (size_t i = P.k; i < cur.size(); ++i) {
            if (!cur[i].videoId.empty()) {
                pos.erase(cur[i].videoId);
            }
        }
        cur.resize(P.k);
    }
}
