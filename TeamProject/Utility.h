#pragma once
/**
 * @file Utility.h
 * @brief 공용 유틸리티 함수 및 상수
 * 
 * [포함 내용]
 * - UI 출력 도우미
 * - 공통 알고리즘 함수 (partition 등)
 * - 타입 정의
 */

// 필요한 헤더만 포함
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "Heap.hpp"
#include "Score.h"

using namespace std;

// ============================================================================
// UI 유틸리티
// ============================================================================

/// 구분선 문자열
inline const string CUTLINE = "------------------------------------";

/// 구분선 출력
inline void print_cutline() {
    cout << CUTLINE << endl;
}

/// 제목 있는 구분선 출력
inline void print_section(const string& title) {
    cout << "\n===== " << title << " =====" << endl;
}

// ============================================================================
// 알고리즘 유틸리티
// ============================================================================

/**
 * @brief Hoare 파티션 함수
 * 
 * Quick Sort와 Quick Select에서 사용되는 파티션 함수입니다.
 * 중앙값을 피벗으로 사용하여 worst case를 방지합니다.
 * 
 * @param p 파티션할 벡터
 * @param left 시작 인덱스
 * @param right 끝 인덱스
 * @return 파티션 경계 인덱스
 * 
 * [시간복잡도] O(n)
 * [정렬 방향] 내림차순 (큰 값이 앞으로)
 */
int partition_d(vector<Score>& p, int left, int right);

