#pragma once
/**
 * @file Score.h
 * @brief Score Type Definition and Calculation Strategies
 * 
 * Supports multiple scoring strategies for ranking:
 * - Engagement-based (recommended)
 * - Weighted sum
 * - Normalized composite
 * - Legacy (original)
 */

#include <cstdint>
#include <string>

// Score type definition
using Score = int;
using ScoPtr = Score*;

/**
 * @enum ScoringStrategy
 * @brief Available scoring strategies
 */
enum class ScoringStrategy {
    Engagement = 0,    // Engagement rate based (recommended)
    Weighted,          // Realistic weighted sum
    Normalized,        // Normalized composite (0-1000 scale)
    Legacy             // Original log-based formula
};

// Strategy names for UI
inline const char* ScoringStrategyName[] = {
    "Engagement Rate",      // 참여율 기반
    "Weighted Sum",         // 가중 합산
    "Normalized",           // 정규화 복합
    "Legacy"                // 기존 방식
};

// Strategy descriptions for UI
inline const char* ScoringStrategyDesc[] = {
    "Views + engagement bonus (like/comment ratio)",
    "Views*1 + Likes*50 + Comments*200",
    "Balanced 0-1000 scale with caps",
    "Original log-based formula"
};

/**
 * @brief Convert/normalize raw score
 * @param rawScore Raw score value
 * @return Normalized score
 */
Score ConvScore(Score rawScore);

/**
 * @brief Calculate score using selected strategy
 * @param viewCount View count
 * @param likeCount Like count
 * @param commentCount Comment count
 * @param strategy Scoring strategy to use
 * @return Calculated score
 */
Score CalculateScore(int64_t viewCount, int64_t likeCount, int64_t commentCount, 
                     ScoringStrategy strategy = ScoringStrategy::Engagement);

/**
 * @brief Legacy function for backward compatibility
 */
inline Score CalculateScore(int64_t viewCount, int64_t likeCount, int64_t commentCount) {
    return CalculateScore(viewCount, likeCount, commentCount, ScoringStrategy::Engagement);
}

// ============================================================================
// Individual Scoring Functions
// ============================================================================

/**
 * @brief Engagement-based scoring (RECOMMENDED)
 * 
 * Formula:
 *   base = log10(views) * 100
 *   engagementBonus = (likes/views * 1000) + (comments/views * 5000)
 *   score = base * (1 + engagementBonus)
 * 
 * Benefits:
 * - Rewards high engagement rate
 * - Small channels with good engagement can rank high
 * - Similar to YouTube's algorithm philosophy
 */
Score CalculateEngagementScore(int64_t viewCount, int64_t likeCount, int64_t commentCount);

/**
 * @brief Weighted sum scoring
 * 
 * Formula:
 *   raw = views*1 + likes*50 + comments*200
 *   score = log10(raw) * 1000
 * 
 * Rationale:
 * - 1 like ≈ 50 views (avg like rate ~2%)
 * - 1 comment ≈ 200 views (avg comment rate ~0.5%)
 */
Score CalculateWeightedScore(int64_t viewCount, int64_t likeCount, int64_t commentCount);

/**
 * @brief Normalized composite scoring
 * 
 * Formula:
 *   V = min(100, log10(views) * 15)     // 10M views → ~100
 *   L = min(100, log10(likes) * 20)     // 100K likes → ~100
 *   C = min(100, log10(comments) * 25)  // 10K comments → ~100
 *   score = (V*0.5 + L*0.3 + C*0.2) * 10
 * 
 * Benefits:
 * - Balanced across metrics
 * - Score capped at 1000
 * - No single metric dominates
 */
Score CalculateNormalizedScore(int64_t viewCount, int64_t likeCount, int64_t commentCount);

/**
 * @brief Legacy scoring (original formula)
 * 
 * Formula:
 *   score = log10(views)*100 + log10(likes)*200 + log10(comments)*300
 */
Score CalculateLegacyScore(int64_t viewCount, int64_t likeCount, int64_t commentCount);
