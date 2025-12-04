#pragma once
/**
 * @file BasicSelect.hpp
 * @brief 템플릿 기반 Top-K 선택 알고리즘 라이브러리
 * 
 * Score, key, 또는 비교 연산자를 정의한 어떤 타입이든 선택 가능합니다.
 * 
 * [알고리즘 목록]
 * - SequentialSelect: 힙 기반 순차 선택 (O(n log k))
 * - QuickSelect: Quick Select 알고리즘 (O(n) 평균)
 * - BinarySelect: 이진 탐색 기반 선택 (O(n log max), 정수 전용)
 * - NthElement: std::nth_element 래퍼 (O(n) 평균)
 * 
 * [사용 예시]
 * vector<Score> scores = {100, 500, 200, 800, 300};
 * Score cutline = quickSelect(scores, 3);  // 3번째로 큰 값
 * 
 * vector<key> keys = {...};
 * auto topK = getTopK(keys, 10);  // 상위 10개
 */

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <queue>
#include "Heap.hpp"

using namespace std;

// ============================================================================
// 선택 알고리즘 열거형
// ============================================================================

enum class SelectAlgorithm {
    Sequential,    // 힙 기반 순차 선택 O(n log k)
    QuickSelect,   // Quick Select O(n) 평균
    BinarySelect,  // 이진 탐색 기반 (정수 전용)
    NthElement     // std::nth_element 사용
};

/**
 * @brief 알고리즘 이름 반환
 */
inline string getSelectAlgorithmName(SelectAlgorithm algo) {
    switch (algo) {
        case SelectAlgorithm::Sequential:   return "Sequential (Heap)";
        case SelectAlgorithm::QuickSelect:  return "Quick Select";
        case SelectAlgorithm::BinarySelect: return "Binary Select";
        case SelectAlgorithm::NthElement:   return "std::nth_element";
        default:                            return "Unknown";
    }
}

// ============================================================================
// 비교 헬퍼 (BasicSort.hpp와 동일한 방식)
// ============================================================================

namespace SelectHelper {

// 내림차순 비교 (큰 값이 앞)
template <typename T>
inline bool comesBefore(const T& a, const T& b) {
    return a < b;  // key 타입: operator<가 이미 내림차순
}

template <>
inline bool comesBefore<int>(const int& a, const int& b) {
    return a > b;  // Score(int): 큰 값이 앞
}

template <typename T>
inline bool comesAfter(const T& a, const T& b) {
    return comesBefore(b, a);
}

// 값 추출 헬퍼 (정수 변환용)
template <typename T>
inline int64_t getValue(const T& item) {
    return static_cast<int64_t>(item);
}

} // namespace SelectHelper

// ============================================================================
// 파티션 함수 (Quick Select용)
// ============================================================================

namespace SelectDetail {

template <typename T>
int partition(vector<T>& p, int left, int right) {
    // 중앙값 피벗
    T pivot = p[(left + right) / 2];
    int i = left, j = right;
    
    while (i <= j) {
        while (SelectHelper::comesBefore(p[i], pivot)) i++;
        while (SelectHelper::comesAfter(p[j], pivot)) j--;
        
        if (i <= j) {
            swap(p[i], p[j]);
            i++;
            j--;
        }
    }
    return i;
}

} // namespace SelectDetail

// ============================================================================
// 1. Sequential Select (힙 기반)
// ============================================================================

/**
 * @brief 힙을 사용한 순차적 Top-K 선택
 * @tparam T 비교 가능한 타입
 * @param p 원본 데이터 (수정되지 않음)
 * @param k 선택할 개수
 * @return 상위 k개 요소 벡터
 * 
 * [시간복잡도] O(n log k)
 * [공간복잡도] O(k)
 */
template <typename T>
vector<T> sequentialSelect(const vector<T>& p, int k) {
    if (p.empty() || k <= 0) return {};
    
    k = min(k, static_cast<int>(p.size()));
    
    // Min-heap (가장 작은 값이 top) - 내림차순 정렬을 위해
    // key 타입은 operator<가 내림차순이므로 max-heap처럼 동작
    auto cmp = [](const T& a, const T& b) {
        return SelectHelper::comesBefore(a, b);  // 큰 값이 아래로
    };
    priority_queue<T, vector<T>, decltype(cmp)> minHeap(cmp);
    
    for (const auto& item : p) {
        if (static_cast<int>(minHeap.size()) < k) {
            minHeap.push(item);
        } else if (SelectHelper::comesBefore(item, minHeap.top())) {
            minHeap.pop();
            minHeap.push(item);
        }
    }
    
    // 힙에서 결과 추출
    vector<T> result;
    result.reserve(k);
    while (!minHeap.empty()) {
        result.push_back(minHeap.top());
        minHeap.pop();
    }
    
    // 내림차순 정렬
    sort(result.begin(), result.end(), [](const T& a, const T& b) {
        return SelectHelper::comesBefore(a, b);
    });
    
    return result;
}

/**
 * @brief 힙 기반 선택으로 커트라인 반환
 * @return k번째로 큰 값
 */
template <typename T>
T sequentialSelectCutline(const vector<T>& p, int k) {
    auto topK = sequentialSelect(p, k);
    if (topK.empty()) throw out_of_range("empty result");
    return topK.back();  // k번째 값 (가장 작은 값)
}

// ============================================================================
// 2. Quick Select
// ============================================================================

namespace SelectDetail {

template <typename T>
T quickSelectImpl(vector<T>& p, int k, int left, int right) {
    if (left == right) return p[left];
    
    int pivotIdx = partition(p, left, right);
    
    if (k < pivotIdx) {
        return quickSelectImpl(p, k, left, pivotIdx - 1);
    } else {
        return quickSelectImpl(p, k, pivotIdx, right);
    }
}

} // namespace SelectDetail

/**
 * @brief Quick Select 알고리즘
 * @tparam T 비교 가능한 타입
 * @param p 데이터 (순서가 변경될 수 있음!)
 * @param k k번째 값 (0-indexed)
 * @return k번째로 큰 값
 * 
 * [시간복잡도] O(n) 평균, O(n²) 최악
 * [공간복잡도] O(log n) 재귀
 * 
 * @warning 원본 벡터의 순서가 변경됩니다!
 */
template <typename T>
T quickSelect(vector<T>& p, int k) {
    if (p.empty()) throw out_of_range("empty vector");
    if (k < 0 || k >= static_cast<int>(p.size())) {
        throw out_of_range("k out of range");
    }
    return SelectDetail::quickSelectImpl(p, k, 0, static_cast<int>(p.size()) - 1);
}

/**
 * @brief Quick Select로 상위 K개 추출
 * @warning 원본 벡터의 순서가 변경됩니다!
 */
template <typename T>
vector<T> quickSelectTopK(vector<T>& p, int k) {
    if (p.empty() || k <= 0) return {};
    
    k = min(k, static_cast<int>(p.size()));
    
    // k번째 요소를 제자리에 배치
    quickSelect(p, k - 1);
    
    // 상위 k개 복사 후 정렬
    vector<T> result(p.begin(), p.begin() + k);
    sort(result.begin(), result.end(), [](const T& a, const T& b) {
        return SelectHelper::comesBefore(a, b);
    });
    
    return result;
}

// ============================================================================
// 3. Binary Select (이진 탐색 기반, 정수 전용)
// ============================================================================

/**
 * @brief 이진 탐색 기반 선택 (정수 전용)
 * @param p 데이터 (양의 정수)
 * @param k 선택할 개수
 * @return {커트라인, 상위 k개 벡터}
 * 
 * [시간복잡도] O(n log max)
 * [공간복잡도] O(n)
 */
template <typename T>
typename enable_if<is_integral<T>::value, pair<T, vector<T>>>::type
binarySelect(const vector<T>& p, int k) {
    if (p.empty() || k <= 0) return {T{}, {}};
    
    k = min(k, static_cast<int>(p.size()));
    
    auto [minIt, maxIt] = minmax_element(p.begin(), p.end());
    T lo = *minIt, hi = *maxIt;
    
    vector<T> result;
    result.reserve(k);
    vector<T> cur = p;
    vector<T> big, small;
    int remain = k;
    
    while (lo < hi) {
        T mid = static_cast<T>((static_cast<int64_t>(lo) + hi + 1) / 2);
        
        big.clear();
        small.clear();
        
        for (const T& x : cur) {
            if (x >= mid) big.push_back(x);
            else small.push_back(x);
        }
        
        if (static_cast<int>(big.size()) >= remain) {
            lo = mid;
            cur.swap(big);
        } else {
            // big 전부 확정
            for (const T& x : big) {
                result.push_back(x);
            }
            remain -= static_cast<int>(big.size());
            hi = mid - 1;
            cur.swap(small);
        }
    }
    
    // 부족하면 lo 값으로 채우기
    if (static_cast<int>(result.size()) < k) {
        for (const T& x : p) {
            if (x == lo) {
                result.push_back(x);
                if (static_cast<int>(result.size()) == k) break;
            }
        }
    }
    
    // 초과하면 자르기
    if (static_cast<int>(result.size()) > k) {
        nth_element(result.begin(), result.begin() + k, result.end(), greater<T>());
        result.resize(k);
    }
    
    // 정렬
    sort(result.begin(), result.end(), greater<T>());
    
    return {lo, result};
}

// 비정수 타입용 (사용 불가)
template <typename T>
typename enable_if<!is_integral<T>::value, pair<T, vector<T>>>::type
binarySelect(const vector<T>& /* p */, int /* k */) {
    throw invalid_argument("binarySelect는 정수 타입에만 사용 가능합니다.");
}

// ============================================================================
// 4. std::nth_element 래퍼
// ============================================================================

/**
 * @brief std::nth_element를 사용한 Top-K 선택
 * @warning 원본 벡터의 순서가 변경됩니다!
 */
template <typename T>
vector<T> nthElementSelect(vector<T>& p, int k) {
    if (p.empty() || k <= 0) return {};
    
    k = min(k, static_cast<int>(p.size()));
    
    // nth_element로 k번째까지 파티셔닝
    nth_element(p.begin(), p.begin() + k, p.end(),
        [](const T& a, const T& b) {
            return SelectHelper::comesBefore(a, b);
        });
    
    // 상위 k개 복사 후 정렬
    vector<T> result(p.begin(), p.begin() + k);
    sort(result.begin(), result.end(), [](const T& a, const T& b) {
        return SelectHelper::comesBefore(a, b);
    });
    
    return result;
}

/**
 * @brief std::nth_element로 커트라인 반환
 * @warning 원본 벡터의 순서가 변경됩니다!
 */
template <typename T>
T nthElementCutline(vector<T>& p, int k) {
    if (p.empty() || k <= 0) throw out_of_range("invalid input");
    
    k = min(k, static_cast<int>(p.size()));
    
    nth_element(p.begin(), p.begin() + k - 1, p.end(),
        [](const T& a, const T& b) {
            return SelectHelper::comesBefore(a, b);
        });
    
    return p[k - 1];
}

// ============================================================================
// 통합 선택 함수
// ============================================================================

/**
 * @brief 알고리즘 선택하여 Top-K 추출
 * @param p 데이터 (일부 알고리즘은 순서 변경)
 * @param k 선택할 개수
 * @param algo 사용할 알고리즘
 * @return 상위 k개 벡터
 */
template <typename T>
vector<T> selectTopK(vector<T>& p, int k, SelectAlgorithm algo) {
    switch (algo) {
        case SelectAlgorithm::Sequential:
            return sequentialSelect(p, k);
            
        case SelectAlgorithm::QuickSelect:
            return quickSelectTopK(p, k);
            
        case SelectAlgorithm::BinarySelect:
            if constexpr (is_integral<T>::value) {
                auto [cutline, result] = binarySelect(p, k);
                return result;
            } else {
                // 비정수는 QuickSelect로 대체
                return quickSelectTopK(p, k);
            }
            
        case SelectAlgorithm::NthElement:
        default:
            return nthElementSelect(p, k);
    }
}

/**
 * @brief 알고리즘 선택하여 커트라인(k번째 값) 반환
 */
template <typename T>
T selectCutline(vector<T>& p, int k, SelectAlgorithm algo) {
    switch (algo) {
        case SelectAlgorithm::Sequential:
            return sequentialSelectCutline(p, k);
            
        case SelectAlgorithm::QuickSelect:
            return quickSelect(p, k - 1);  // 0-indexed
            
        case SelectAlgorithm::BinarySelect:
            if constexpr (is_integral<T>::value) {
                auto [cutline, result] = binarySelect(p, k);
                return cutline;
            } else {
                return quickSelect(p, k - 1);
            }
            
        case SelectAlgorithm::NthElement:
        default:
            return nthElementCutline(p, k);
    }
}

