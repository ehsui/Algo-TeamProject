#pragma once
/**
 * @file Score.h
 * @brief 점수 타입 정의 (레거시 호환용)
 * 
 * [주의] 새 코드에서는 Domain/VideoMetrics.h의 double score를 사용하세요.
 * 이 파일은 기존 코드와의 호환성을 위해 유지됩니다.
 */

// 필요한 헤더만 포함 (Headers.h 대신)
#include <cstdint>

// 점수 타입 정의
using Score = int;
using ScoPtr = Score*;

/**
 * @brief 점수 변환/정규화 함수
 * @param rawScore 원시 점수
 * @return 변환된 점수
 * 
 * [변환 로직]
 * - 음수 → 0
 * - 상한값 적용
 * - 정규화
 */
Score ConvScore(Score rawScore);

/**
 * @brief 다양한 지표를 종합한 점수 계산
 * @param viewCount 조회수
 * @param likeCount 좋아요 수
 * @param commentCount 댓글 수
 * @return 종합 점수
 */
Score CalculateScore(int64_t viewCount, int64_t likeCount, int64_t commentCount);
