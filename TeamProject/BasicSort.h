#pragma once
/**
 * @file BasicSort.h
 * @brief 템플릿 기반 정렬 알고리즘 래퍼
 * 
 * 실제 구현은 BasicSort.hpp에 있습니다.
 * 이 파일은 하위 호환성을 위해 유지됩니다.
 * 
 * [지원 타입]
 * - Score (int): 기본 정수 점수
 * - key: 영상 정렬용 키 구조체
 * - 기타 비교 연산자를 정의한 모든 타입
 * 
 * [사용 예시]
 * vector<Score> scores = {5, 2, 8};
 * quicksort(scores);  // Score 정렬
 * 
 * vector<key> keys = {...};
 * quicksort(keys);    // key 정렬
 * 
 * // 알고리즘 선택
 * sort(scores, SortAlgorithm::Merge);
 */

// 템플릿 구현 헤더 포함
#include "BasicSort.hpp"
