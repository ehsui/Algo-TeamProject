#pragma once
/**
 * @file BasicSelect.h
 * @brief 템플릿 기반 Top-K 선택 알고리즘 래퍼
 * 
 * 실제 구현은 BasicSelect.hpp에 있습니다.
 * 
 * [지원 타입]
 * - Score (int): 기본 정수 점수
 * - key: 영상 정렬용 키 구조체
 * - 기타 비교 연산자를 정의한 모든 타입
 * 
 * [알고리즘]
 * - sequentialSelect: 힙 기반 O(n log k)
 * - quickSelect: Quick Select O(n) 평균
 * - binarySelect: 이진 탐색 기반 (정수 전용)
 * - nthElementSelect: std::nth_element 래퍼
 * 
 * [사용 예시]
 * vector<Score> scores = {100, 500, 200};
 * auto topK = selectTopK(scores, 2, SelectAlgorithm::QuickSelect);
 * 
 * vector<key> keys = {...};
 * auto topKeys = sequentialSelect(keys, 10);
 */

// 템플릿 구현 헤더 포함
#include "BasicSelect.hpp"