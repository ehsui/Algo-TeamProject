#pragma once
/**
 * @file BasicSort.hpp
 * @brief 템플릿 기반 정렬 알고리즘 라이브러리
 * 
 * Score, key, 또는 비교 연산자를 정의한 어떤 타입이든 정렬 가능합니다.
 * 
 * [사용 예시]
 * vector<Score> scores = {5, 2, 8, 1};
 * selectsort(scores);  // Score 정렬
 * 
 * vector<key> keys = {...};
 * selectsort(keys);    // key 정렬 (operator< 사용)
 * 
 * [비교 방식]
 * - 기본: 내림차순 (큰 값이 앞으로)
 * - key 타입: operator<가 이미 내림차순으로 정의됨
 * 
 * [시간복잡도]
 * - O(n²): selectsort, insertionsort, bubblesort
 * - O(n log n): quicksort, mergesort, heapSort
 * - O(n^1.5): shellSort
 * - O(n+k): countingSort (정수 전용)
 * - O(d*n): radixSort (정수 전용)
 */

#include <vector>
#include <algorithm>
#include <type_traits>

using namespace std;

// ============================================================================
// 비교 헬퍼 (내림차순 기준)
// ============================================================================

namespace SortHelper {

/**
 * @brief 내림차순 비교: a가 b보다 "앞"에 와야 하는지 (a > b)
 * 
 * key 타입의 경우 operator<가 이미 내림차순으로 정의되어 있으므로
 * a < b가 true면 a가 앞에 온다.
 */
template <typename T>
inline bool comesBefore(const T& a, const T& b) {
    // key 타입은 operator<가 내림차순 (높은 점수 우선)
    // Score(int) 타입은 일반적인 < 이므로 >로 비교해야 내림차순
    return a < b;  // key: 이미 내림차순, Score: 오름차순
}

// Score(int) 타입 특수화: 내림차순을 위해 > 사용
template <>
inline bool comesBefore<int>(const int& a, const int& b) {
    return a > b;  // 큰 값이 앞으로
}

/**
 * @brief 내림차순 비교: a가 b보다 "뒤"에 와야 하는지 (a < b)
 */
template <typename T>
inline bool comesAfter(const T& a, const T& b) {
    return comesBefore(b, a);
}

/**
 * @brief a >= b (내림차순 기준)
 */
template <typename T>
inline bool comesBeforeOrEqual(const T& a, const T& b) {
    return !comesAfter(a, b);
}

} // namespace SortHelper

// ============================================================================
// O(n²) 정렬 알고리즘
// ============================================================================

/**
 * @brief 선택 정렬 (내림차순)
 * @tparam T 비교 가능한 타입 (Score, key 등)
 * @param p 정렬할 벡터
 * 
 * [시간] O(n²), [공간] O(1), [안정성] 불안정
 */
template <typename T>
void selectsort(vector<T>& p) {
    int size = static_cast<int>(p.size());
    for (int i = 0; i < size - 1; i++) {
        int bestIdx = i;
        for (int j = i + 1; j < size; j++) {
            if (SortHelper::comesBefore(p[j], p[bestIdx])) {
                bestIdx = j;
            }
        }
        if (bestIdx != i) {
            swap(p[i], p[bestIdx]);
        }
    }
}

/**
 * @brief 삽입 정렬 (내림차순)
 * @tparam T 비교 가능한 타입
 * @param p 정렬할 벡터
 * 
 * [시간] O(n²) 최악, O(n) 최선, [공간] O(1), [안정성] 안정
 */
template <typename T>
void insertionsort(vector<T>& p) {
    int size = static_cast<int>(p.size());
    for (int i = 1; i < size; i++) {
        T key = move(p[i]);
        int j = i - 1;
        while (j >= 0 && SortHelper::comesBefore(key, p[j])) {
            p[j + 1] = move(p[j]);
            j--;
        }
        p[j + 1] = move(key);
    }
}

/**
 * @brief 버블 정렬 (내림차순)
 * @tparam T 비교 가능한 타입
 * @param p 정렬할 벡터
 * 
 * [시간] O(n²), [공간] O(1), [안정성] 안정
 */
template <typename T>
void bubblesort(vector<T>& p) {
    int size = static_cast<int>(p.size());
    for (int i = 0; i < size - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < size - 1 - i; j++) {
            if (SortHelper::comesAfter(p[j], p[j + 1])) {
                swap(p[j], p[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// ============================================================================
// O(n log n) 정렬 알고리즘
// ============================================================================

namespace SortDetail {

/**
 * @brief Quick Sort용 파티션 함수
 */
template <typename T>
int partition(vector<T>& p, int left, int right) {
    // 중앙값 피벗 선택
    T pivot = p[(left + right) / 2];
    int i = left, j = right;
    
    while (i <= j) {
        while (SortHelper::comesBefore(p[i], pivot)) i++;
        while (SortHelper::comesAfter(p[j], pivot)) j--;
        
        if (i <= j) {
            swap(p[i], p[j]);
            i++;
            j--;
        }
    }
    return i;
}

/**
 * @brief Quick Sort 재귀 구현
 */
template <typename T>
void quicksortImpl(vector<T>& p, int left, int right) {
    if (left >= right) return;
    
    int idx = partition(p, left, right);
    if (left < idx - 1) quicksortImpl(p, left, idx - 1);
    if (idx < right) quicksortImpl(p, idx, right);
}

/**
 * @brief Merge Sort용 병합 함수
 */
template <typename T>
void merge(vector<T>& p, int left, int mid, int right) {
    int n = right - left + 1;
    vector<T> temp(n);
    int i = left, j = mid + 1, k = 0;
    
    while (i <= mid && j <= right) {
        if (SortHelper::comesBeforeOrEqual(p[i], p[j])) {
            temp[k++] = move(p[i++]);
        } else {
            temp[k++] = move(p[j++]);
        }
    }
    
    while (i <= mid) temp[k++] = move(p[i++]);
    while (j <= right) temp[k++] = move(p[j++]);
    
    for (int t = 0; t < n; t++) {
        p[left + t] = move(temp[t]);
    }
}

/**
 * @brief Merge Sort 재귀 구현
 */
template <typename T>
void mergesortImpl(vector<T>& p, int left, int right) {
    if (left >= right) return;
    
    int mid = left + (right - left) / 2;
    mergesortImpl(p, left, mid);
    mergesortImpl(p, mid + 1, right);
    merge(p, left, mid, right);
}

/**
 * @brief Heap Sort용 heapify 함수
 */
template <typename T>
void heapify(vector<T>& p, int n, int i) {
    int best = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    // 내림차순: 작은 값이 위로 (min-heap처럼)
    if (left < n && SortHelper::comesAfter(p[best], p[left])) {
        best = left;
    }
    if (right < n && SortHelper::comesAfter(p[best], p[right])) {
        best = right;
    }
    
    if (best != i) {
        swap(p[i], p[best]);
        heapify(p, n, best);
    }
}

} // namespace SortDetail

/**
 * @brief 퀵 정렬 (내림차순)
 * @tparam T 비교 가능한 타입
 * @param p 정렬할 벡터
 * 
 * [시간] O(n log n) 평균, O(n²) 최악, [공간] O(log n), [안정성] 불안정
 */
template <typename T>
void quicksort(vector<T>& p) {
    if (p.size() <= 1) return;
    SortDetail::quicksortImpl(p, 0, static_cast<int>(p.size()) - 1);
}

template <typename T>
void quicksort(vector<T>& p, int left, int right) {
    SortDetail::quicksortImpl(p, left, right);
}

/**
 * @brief 병합 정렬 (내림차순)
 * @tparam T 비교 가능한 타입
 * @param p 정렬할 벡터
 * 
 * [시간] O(n log n), [공간] O(n), [안정성] 안정
 */
template <typename T>
void mergesort(vector<T>& p) {
    if (p.size() <= 1) return;
    SortDetail::mergesortImpl(p, 0, static_cast<int>(p.size()) - 1);
}

template <typename T>
void mergesort(vector<T>& p, int left, int right) {
    SortDetail::mergesortImpl(p, left, right);
}

/**
 * @brief 셸 정렬 (내림차순)
 * @tparam T 비교 가능한 타입
 * @param p 정렬할 벡터
 * 
 * [시간] O(n^1.5) ~ O(n log²n), [공간] O(1), [안정성] 불안정
 */
template <typename T>
void shellSort(vector<T>& p) {
    int size = static_cast<int>(p.size());
    
    for (int gap = size / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < size; i++) {
            T temp = move(p[i]);
            int j = i;
            
            while (j >= gap && SortHelper::comesBefore(temp, p[j - gap])) {
                p[j] = move(p[j - gap]);
                j -= gap;
            }
            p[j] = move(temp);
        }
    }
}

/**
 * @brief 힙 정렬 (내림차순)
 * @tparam T 비교 가능한 타입
 * @param p 정렬할 벡터
 * 
 * [시간] O(n log n), [공간] O(1), [안정성] 불안정
 */
template <typename T>
void heapSort(vector<T>& p) {
    int size = static_cast<int>(p.size());
    if (size <= 1) return;
    
    // Build heap
    for (int i = size / 2 - 1; i >= 0; i--) {
        SortDetail::heapify(p, size, i);
    }
    
    // Extract elements
    for (int i = size - 1; i > 0; i--) {
        swap(p[0], p[i]);
        SortDetail::heapify(p, i, 0);
    }
}

// ============================================================================
// 특수 정렬 알고리즘 (정수 전용)
// ============================================================================

/**
 * @brief 카운팅 정렬 (내림차순, 정수 전용)
 * @param p 정렬할 벡터 (양의 정수)
 * 
 * [시간] O(n + k), [공간] O(k), [안정성] 안정
 * @note k = max(p)
 */
template <typename T>
typename enable_if<is_integral<T>::value>::type
countingSort(vector<T>& p) {
    int size = static_cast<int>(p.size());
    if (size <= 0) return;
    
    T maxVal = *max_element(p.begin(), p.end());
    T minVal = *min_element(p.begin(), p.end());
    
    int range = static_cast<int>(maxVal - minVal + 1);
    vector<int> count(range, 0);
    vector<T> output(size);
    
    // Count occurrences
    for (const auto& val : p) {
        count[static_cast<int>(val - minVal)]++;
    }
    
    // 내림차순: 뒤에서부터 누적합
    for (int i = range - 2; i >= 0; i--) {
        count[i] += count[i + 1];
    }
    
    // Build output (stable)
    for (int i = size - 1; i >= 0; i--) {
        int idx = static_cast<int>(p[i] - minVal);
        output[--count[idx]] = p[i];
    }
    
    p = move(output);
}

// 비정수 타입용 countingSort (사용 불가 알림)
template <typename T>
typename enable_if<!is_integral<T>::value>::type
countingSort(vector<T>& /* p */) {
    static_assert(is_integral<T>::value, 
        "countingSort는 정수 타입에만 사용 가능합니다. quicksort나 mergesort를 사용하세요.");
}

namespace SortDetail {

/**
 * @brief Radix Sort용 자릿수별 카운팅 정렬
 */
template <typename T>
void countingSortByDigit(vector<T>& p, T exp) {
    int size = static_cast<int>(p.size());
    vector<T> output(size);
    int count[10] = {0};
    
    // Count digits (내림차순을 위해 9-digit 사용)
    for (int i = 0; i < size; i++) {
        int digit = (p[i] / exp) % 10;
        count[9 - digit]++;
    }
    
    // Cumulative count
    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }
    
    // Build output (stable, reverse order)
    for (int i = size - 1; i >= 0; i--) {
        int digit = (p[i] / exp) % 10;
        output[--count[9 - digit]] = p[i];
    }
    
    p = move(output);
}

} // namespace SortDetail

/**
 * @brief 기수 정렬 (내림차순, 정수 전용)
 * @param p 정렬할 벡터 (양의 정수)
 * 
 * [시간] O(d * n), [공간] O(n), [안정성] 안정
 * @note d = 최대 자릿수
 */
template <typename T>
typename enable_if<is_integral<T>::value>::type
radixSort(vector<T>& p) {
    int size = static_cast<int>(p.size());
    if (size <= 0) return;
    
    T maxVal = *max_element(p.begin(), p.end());
    
    for (T exp = 1; maxVal / exp > 0; exp *= 10) {
        SortDetail::countingSortByDigit(p, exp);
    }
}

// 비정수 타입용 radixSort (사용 불가 알림)
template <typename T>
typename enable_if<!is_integral<T>::value>::type
radixSort(vector<T>& /* p */) {
    static_assert(is_integral<T>::value, 
        "radixSort는 정수 타입에만 사용 가능합니다. quicksort나 mergesort를 사용하세요.");
}

// ============================================================================
// 유틸리티 함수
// ============================================================================

/**
 * @brief 정렬 여부 확인 (내림차순)
 */
template <typename T>
bool isSorted(const vector<T>& p) {
    for (size_t i = 1; i < p.size(); i++) {
        if (SortHelper::comesAfter(p[i - 1], p[i])) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 정렬 알고리즘 열거형
 */
enum class SortAlgorithm {
    Selection,
    Insertion,
    Bubble,
    Quick,
    Merge,
    Shell,
    Heap,
    Counting,  // 정수 전용
    Radix      // 정수 전용
};

/**
 * @brief 통합 정렬 함수 (알고리즘 선택)
 */
template <typename T>
void sort(vector<T>& p, SortAlgorithm algo) {
    switch (algo) {
        case SortAlgorithm::Selection: selectsort(p); break;
        case SortAlgorithm::Insertion: insertionsort(p); break;
        case SortAlgorithm::Bubble:    bubblesort(p); break;
        case SortAlgorithm::Quick:     quicksort(p); break;
        case SortAlgorithm::Merge:     mergesort(p); break;
        case SortAlgorithm::Shell:     shellSort(p); break;
        case SortAlgorithm::Heap:      heapSort(p); break;
        case SortAlgorithm::Counting:  countingSort(p); break;
        case SortAlgorithm::Radix:     radixSort(p); break;
    }
}

