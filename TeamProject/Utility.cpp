/**
 * @file Utility.cpp
 * @brief 공용 유틸리티 함수 구현
 */

#include "Utility.h"

int partition_d(vector<Score>& p, int left, int right) {
    // 중앙값을 피벗으로 선택 (worst case 방지)
    Score pivot = p[(left + right) / 2];
    int i = left, j = right;
    
    while (i <= j) {
        // 내림차순: 피벗보다 큰 값은 왼쪽에
        while (p[i] > pivot) i++;
        while (p[j] < pivot) j--;
        
        if (i <= j) {
            swap(p[i], p[j]);
            i++;
            j--;
        }
    }
    
    return i;
}
