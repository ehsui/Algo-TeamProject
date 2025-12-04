#pragma once
/**
 * @file SortingStrategy.hpp
 * @brief 템플릿 기반 정렬 알고리즘 라이브러리
 * 
 * [설계 원칙]
 * 1. 제네릭: 어떤 비교 가능한 타입도 정렬 가능
 * 2. 전략 패턴: 런타임에 정렬 알고리즘 교체 가능
 * 3. 확장성: 새 알고리즘 추가 시 기존 코드 수정 불필요
 * 
 * [사용 예시]
 * std::vector<int> data = {5, 2, 8, 1, 9};
 * Sorting::sort(data, Sorting::Algorithm::QuickSort);
 * 
 * // 또는 커스텀 비교자 사용
 * Sorting::sort(data, Sorting::Algorithm::MergeSort, std::greater<int>());
 */

#include <vector>
#include <functional>
#include <algorithm>
#include <string>
#include <memory>
#include <stdexcept>

namespace Sorting {

/**
 * @enum Algorithm
 * @brief 지원하는 정렬 알고리즘 목록
 */
enum class Algorithm {
    SelectionSort,
    InsertionSort,
    BubbleSort,
    QuickSort,
    MergeSort,
    ShellSort,
    HeapSort,
    CountingSort,   // 정수 전용
    RadixSort,      // 정수 전용
    StdSort         // std::sort 래퍼
};

/**
 * @brief 알고리즘 이름 반환
 */
inline std::string getAlgorithmName(Algorithm algo) {
    switch (algo) {
        case Algorithm::SelectionSort: return "Selection Sort";
        case Algorithm::InsertionSort: return "Insertion Sort";
        case Algorithm::BubbleSort:    return "Bubble Sort";
        case Algorithm::QuickSort:     return "Quick Sort";
        case Algorithm::MergeSort:     return "Merge Sort";
        case Algorithm::ShellSort:     return "Shell Sort";
        case Algorithm::HeapSort:      return "Heap Sort";
        case Algorithm::CountingSort:  return "Counting Sort";
        case Algorithm::RadixSort:     return "Radix Sort";
        case Algorithm::StdSort:       return "std::sort";
        default:                       return "Unknown";
    }
}

// ============================================================================
// 내부 구현 (namespace detail)
// ============================================================================

namespace detail {

/**
 * @brief 기본 비교자 (내림차순 - 랭킹용)
 */
template <typename T>
struct DescendingComparator {
    bool operator()(const T& a, const T& b) const {
        return a > b;  // 큰 값이 앞으로
    }
};

// --- Selection Sort ---
template <typename T, typename Compare>
void selectionSort(std::vector<T>& data, Compare comp) {
    const int n = static_cast<int>(data.size());
    for (int i = 0; i < n - 1; ++i) {
        int bestIdx = i;
        for (int j = i + 1; j < n; ++j) {
            if (comp(data[j], data[bestIdx])) {
                bestIdx = j;
            }
        }
        if (bestIdx != i) {
            std::swap(data[i], data[bestIdx]);
        }
    }
}

// --- Insertion Sort ---
template <typename T, typename Compare>
void insertionSort(std::vector<T>& data, Compare comp) {
    const int n = static_cast<int>(data.size());
    for (int i = 1; i < n; ++i) {
        T key = std::move(data[i]);
        int j = i - 1;
        while (j >= 0 && comp(key, data[j])) {
            data[j + 1] = std::move(data[j]);
            --j;
        }
        data[j + 1] = std::move(key);
    }
}

// --- Bubble Sort ---
template <typename T, typename Compare>
void bubbleSort(std::vector<T>& data, Compare comp) {
    const int n = static_cast<int>(data.size());
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; ++j) {
            if (comp(data[j + 1], data[j])) {
                std::swap(data[j], data[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

// --- Quick Sort ---
template <typename T, typename Compare>
int partition(std::vector<T>& data, int left, int right, Compare comp) {
    // 중앙값 피벗 선택 (worst case 방지)
    int mid = left + (right - left) / 2;
    T pivot = data[mid];
    
    int i = left, j = right;
    while (i <= j) {
        while (comp(data[i], pivot)) ++i;
        while (comp(pivot, data[j])) --j;
        if (i <= j) {
            std::swap(data[i], data[j]);
            ++i; --j;
        }
    }
    return i;
}

template <typename T, typename Compare>
void quickSortImpl(std::vector<T>& data, int left, int right, Compare comp) {
    if (left >= right) return;
    int idx = partition(data, left, right, comp);
    if (left < idx - 1) quickSortImpl(data, left, idx - 1, comp);
    if (idx < right) quickSortImpl(data, idx, right, comp);
}

template <typename T, typename Compare>
void quickSort(std::vector<T>& data, Compare comp) {
    if (data.size() <= 1) return;
    quickSortImpl(data, 0, static_cast<int>(data.size()) - 1, comp);
}

// --- Merge Sort ---
template <typename T, typename Compare>
void merge(std::vector<T>& data, int left, int mid, int right, Compare comp) {
    std::vector<T> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;
    
    while (i <= mid && j <= right) {
        if (comp(data[i], data[j]) || (!comp(data[j], data[i]) && i < j)) {
            temp[k++] = std::move(data[i++]);
        } else {
            temp[k++] = std::move(data[j++]);
        }
    }
    while (i <= mid) temp[k++] = std::move(data[i++]);
    while (j <= right) temp[k++] = std::move(data[j++]);
    
    for (int t = 0; t < k; ++t) {
        data[left + t] = std::move(temp[t]);
    }
}

template <typename T, typename Compare>
void mergeSortImpl(std::vector<T>& data, int left, int right, Compare comp) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortImpl(data, left, mid, comp);
    mergeSortImpl(data, mid + 1, right, comp);
    merge(data, left, mid, right, comp);
}

template <typename T, typename Compare>
void mergeSort(std::vector<T>& data, Compare comp) {
    if (data.size() <= 1) return;
    mergeSortImpl(data, 0, static_cast<int>(data.size()) - 1, comp);
}

// --- Shell Sort ---
template <typename T, typename Compare>
void shellSort(std::vector<T>& data, Compare comp) {
    const int n = static_cast<int>(data.size());
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; ++i) {
            T temp = std::move(data[i]);
            int j = i;
            while (j >= gap && comp(temp, data[j - gap])) {
                data[j] = std::move(data[j - gap]);
                j -= gap;
            }
            data[j] = std::move(temp);
        }
    }
}

// --- Heap Sort ---
template <typename T, typename Compare>
void heapify(std::vector<T>& data, int n, int i, Compare comp) {
    int best = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n && comp(data[best], data[left])) best = left;
    if (right < n && comp(data[best], data[right])) best = right;
    
    if (best != i) {
        std::swap(data[i], data[best]);
        heapify(data, n, best, comp);
    }
}

template <typename T, typename Compare>
void heapSort(std::vector<T>& data, Compare comp) {
    const int n = static_cast<int>(data.size());
    
    // Build max-heap (반대 방향으로 comp 사용)
    for (int i = n / 2 - 1; i >= 0; --i) {
        heapify(data, n, i, comp);
    }
    
    // Extract elements
    for (int i = n - 1; i > 0; --i) {
        std::swap(data[0], data[i]);
        heapify(data, i, 0, comp);
    }
}

// --- Counting Sort (정수 전용, 내림차순) ---
template <typename T>
void countingSort(std::vector<T>& data) {
    static_assert(std::is_integral<T>::value, "Counting sort requires integral type");
    
    if (data.empty()) return;
    
    T maxVal = *std::max_element(data.begin(), data.end());
    T minVal = *std::min_element(data.begin(), data.end());
    
    int range = static_cast<int>(maxVal - minVal + 1);
    std::vector<int> count(range, 0);
    std::vector<T> output(data.size());
    
    // Count occurrences
    for (const auto& val : data) {
        count[static_cast<int>(val - minVal)]++;
    }
    
    // 내림차순: 뒤에서부터 누적합
    for (int i = range - 2; i >= 0; --i) {
        count[i] += count[i + 1];
    }
    
    // Build output
    for (int i = static_cast<int>(data.size()) - 1; i >= 0; --i) {
        int idx = static_cast<int>(data[i] - minVal);
        output[--count[idx]] = data[i];
    }
    
    data = std::move(output);
}

// --- Radix Sort (정수 전용, 내림차순) ---
template <typename T>
void radixSort(std::vector<T>& data) {
    static_assert(std::is_integral<T>::value, "Radix sort requires integral type");
    
    if (data.empty()) return;
    
    T maxVal = *std::max_element(data.begin(), data.end());
    
    for (T exp = 1; maxVal / exp > 0; exp *= 10) {
        std::vector<T> output(data.size());
        int count[10] = {0};
        
        for (const auto& val : data) {
            int digit = (val / exp) % 10;
            count[9 - digit]++;  // 내림차순
        }
        
        for (int i = 1; i < 10; ++i) {
            count[i] += count[i - 1];
        }
        
        for (int i = static_cast<int>(data.size()) - 1; i >= 0; --i) {
            int digit = (data[i] / exp) % 10;
            output[--count[9 - digit]] = data[i];
        }
        
        data = std::move(output);
    }
}

} // namespace detail

// ============================================================================
// 공개 API
// ============================================================================

/**
 * @brief 메인 정렬 함수 (알고리즘 선택)
 * @tparam T 정렬할 요소 타입
 * @tparam Compare 비교자 타입
 * @param data 정렬할 데이터
 * @param algo 사용할 알고리즘
 * @param comp 비교 함수 (기본: 내림차순)
 */
template <typename T, typename Compare = detail::DescendingComparator<T>>
void sort(std::vector<T>& data, Algorithm algo, Compare comp = Compare{}) {
    switch (algo) {
        case Algorithm::SelectionSort:
            detail::selectionSort(data, comp);
            break;
        case Algorithm::InsertionSort:
            detail::insertionSort(data, comp);
            break;
        case Algorithm::BubbleSort:
            detail::bubbleSort(data, comp);
            break;
        case Algorithm::QuickSort:
            detail::quickSort(data, comp);
            break;
        case Algorithm::MergeSort:
            detail::mergeSort(data, comp);
            break;
        case Algorithm::ShellSort:
            detail::shellSort(data, comp);
            break;
        case Algorithm::HeapSort:
            detail::heapSort(data, comp);
            break;
        case Algorithm::CountingSort:
            if constexpr (std::is_integral<T>::value) {
                detail::countingSort(data);
            } else {
                throw std::invalid_argument("Counting sort requires integral type");
            }
            break;
        case Algorithm::RadixSort:
            if constexpr (std::is_integral<T>::value) {
                detail::radixSort(data);
            } else {
                throw std::invalid_argument("Radix sort requires integral type");
            }
            break;
        case Algorithm::StdSort:
            std::sort(data.begin(), data.end(), comp);
            break;
    }
}

/**
 * @brief 기본 정렬 (Quick Sort, 내림차순)
 */
template <typename T>
void sort(std::vector<T>& data) {
    sort(data, Algorithm::QuickSort);
}

// ============================================================================
// Top-K 선택 알고리즘
// ============================================================================

/**
 * @brief Quick Select로 상위 K개 요소의 경계값 찾기
 * @return K번째로 큰 값
 */
template <typename T, typename Compare = detail::DescendingComparator<T>>
T quickSelect(std::vector<T>& data, int k, Compare comp = Compare{}) {
    if (data.empty() || k <= 0 || k > static_cast<int>(data.size())) {
        throw std::out_of_range("Invalid k for quickSelect");
    }
    
    int left = 0, right = static_cast<int>(data.size()) - 1;
    int targetIdx = k - 1;  // 0-indexed
    
    while (left < right) {
        int pivotIdx = detail::partition(data, left, right, comp);
        
        if (targetIdx < pivotIdx) {
            right = pivotIdx - 1;
        } else {
            left = pivotIdx;
        }
    }
    
    return data[targetIdx];
}

/**
 * @brief 상위 K개 요소만 정렬하여 반환
 * @note 원본 데이터의 순서가 변경될 수 있음
 */
template <typename T, typename Compare = detail::DescendingComparator<T>>
std::vector<T> getTopK(std::vector<T>& data, int k, Compare comp = Compare{}) {
    if (data.empty()) return {};
    
    k = std::min(k, static_cast<int>(data.size()));
    
    // nth_element로 K번째까지 파티셔닝
    std::nth_element(data.begin(), data.begin() + k, data.end(), comp);
    
    // 상위 K개만 정렬
    std::vector<T> topK(data.begin(), data.begin() + k);
    std::sort(topK.begin(), topK.end(), comp);
    
    return topK;
}

} // namespace Sorting

