/**
 * @file Score.cpp
 * @brief 점수 계산/변환 함수 구현
 */

#include "Score.h"
#include <cmath>
#include <algorithm>

// 점수 상한값
constexpr Score MAX_SCORE = 1'000'000;

Score ConvScore(Score rawScore) {
    // 음수 방지
    if (rawScore < 0) return 0;
    
    // 상한값 적용
    return std::min(rawScore, MAX_SCORE);
}

Score CalculateScore(int64_t viewCount, int64_t likeCount, int64_t commentCount) {
    // 로그 스케일 적용 (큰 값의 영향력 감소)
    double viewScore = std::log10(std::max(1LL, viewCount)) * 100.0;
    double likeScore = std::log10(std::max(1LL, likeCount)) * 200.0;
    double commentScore = std::log10(std::max(1LL, commentCount)) * 300.0;
    
    // 종합 점수
    double totalScore = viewScore + likeScore + commentScore;
    
    // 정수로 변환 및 정규화
    return ConvScore(static_cast<Score>(totalScore));
}