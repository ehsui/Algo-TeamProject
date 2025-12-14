/**
 * @file Score.cpp
 * @brief Score Calculation Implementation
 * 
 * Implements 4 different scoring strategies:
 * 1. Engagement-based (recommended)
 * 2. Weighted sum
 * 3. Normalized composite
 * 4. Legacy
 */

#include "Score.h"
#include <cmath>
#include <algorithm>

// Score limits
constexpr Score MAX_SCORE = 1'000'000;
constexpr Score MIN_SCORE = 0;

Score ConvScore(Score rawScore) {
    if (rawScore < MIN_SCORE) return MIN_SCORE;
    return std::min(rawScore, MAX_SCORE);
}

// ============================================================================
// Main Calculator (Strategy Dispatcher)
// ============================================================================

Score CalculateScore(int64_t viewCount, int64_t likeCount, int64_t commentCount,
                     ScoringStrategy strategy) {
    switch (strategy) {
        case ScoringStrategy::Engagement:
            return CalculateEngagementScore(viewCount, likeCount, commentCount);
        case ScoringStrategy::Weighted:
            return CalculateWeightedScore(viewCount, likeCount, commentCount);
        case ScoringStrategy::Normalized:
            return CalculateNormalizedScore(viewCount, likeCount, commentCount);
        case ScoringStrategy::Legacy:
        default:
            return CalculateLegacyScore(viewCount, likeCount, commentCount);
    }
}

// ============================================================================
// Strategy 1: Engagement-Based (RECOMMENDED)
// ============================================================================

Score CalculateEngagementScore(int64_t viewCount, int64_t likeCount, int64_t commentCount) {
    // Prevent division by zero
    if (viewCount <= 0) {
        viewCount = 1;
    }
    
    // Base score from views (log scale)
    double baseScore = std::log10(static_cast<double>(viewCount)) * 100.0;
    
    // Engagement rates
    double likeRate = static_cast<double>(likeCount) / static_cast<double>(viewCount);
    double commentRate = static_cast<double>(commentCount) / static_cast<double>(viewCount);
    
    // Engagement bonus
    // - Average like rate: ~2-4% → bonus ~20-40
    // - Average comment rate: ~0.1-0.5% → bonus ~5-25
    double engagementBonus = (likeRate * 1000.0) + (commentRate * 5000.0);
    
    // Cap engagement bonus to prevent extreme outliers
    engagementBonus = std::min(engagementBonus, 100.0);
    
    // Final score: base * (1 + bonus ratio)
    double finalScore = baseScore * (1.0 + engagementBonus / 100.0);
    
    return ConvScore(static_cast<Score>(finalScore));
}

// ============================================================================
// Strategy 2: Weighted Sum
// ============================================================================

Score CalculateWeightedScore(int64_t viewCount, int64_t likeCount, int64_t commentCount) {
    // Weights based on typical engagement rates:
    // - Like rate ~2% → 1 like worth ~50 views
    // - Comment rate ~0.5% → 1 comment worth ~200 views
    constexpr double VIEW_WEIGHT = 1.0;
    constexpr double LIKE_WEIGHT = 50.0;
    constexpr double COMMENT_WEIGHT = 200.0;
    
    // Calculate raw weighted sum
    double rawScore = 
        static_cast<double>(viewCount) * VIEW_WEIGHT +
        static_cast<double>(likeCount) * LIKE_WEIGHT +
        static_cast<double>(commentCount) * COMMENT_WEIGHT;
    
    // Apply log scale to compress large values
    double logScore = std::log10(std::max(1.0, rawScore)) * 1000.0;
    
    return ConvScore(static_cast<Score>(logScore));
}

// ============================================================================
// Strategy 3: Normalized Composite
// ============================================================================

Score CalculateNormalizedScore(int64_t viewCount, int64_t likeCount, int64_t commentCount) {
    // Normalize each metric to 0-100 scale
    // Target: 10M views → 100, 100K likes → 100, 10K comments → 100
    
    auto normalize = [](int64_t value, double multiplier, double cap = 100.0) {
        if (value <= 0) return 0.0;
        double logValue = std::log10(static_cast<double>(value));
        return std::min(cap, logValue * multiplier);
    };
    
    double viewNorm = normalize(viewCount, 15.0);      // 10^6.67 ≈ 10M → 100
    double likeNorm = normalize(likeCount, 20.0);      // 10^5 = 100K → 100
    double commentNorm = normalize(commentCount, 25.0); // 10^4 = 10K → 100
    
    // Weighted combination (total weight = 1.0)
    constexpr double VIEW_WEIGHT = 0.50;    // 50% weight on views
    constexpr double LIKE_WEIGHT = 0.30;    // 30% weight on likes
    constexpr double COMMENT_WEIGHT = 0.20; // 20% weight on comments
    
    double compositeScore = 
        viewNorm * VIEW_WEIGHT +
        likeNorm * LIKE_WEIGHT +
        commentNorm * COMMENT_WEIGHT;
    
    // Scale to 0-1000
    double finalScore = compositeScore * 10.0;
    
    return ConvScore(static_cast<Score>(finalScore));
}

// ============================================================================
// Strategy 4: Legacy (Original Formula)
// ============================================================================

Score CalculateLegacyScore(int64_t viewCount, int64_t likeCount, int64_t commentCount) {
    // Original weights
    constexpr double VIEW_MULTIPLIER = 100.0;
    constexpr double LIKE_MULTIPLIER = 200.0;
    constexpr double COMMENT_MULTIPLIER = 300.0;
    
    double viewScore = std::log10(std::max(1LL, viewCount)) * VIEW_MULTIPLIER;
    double likeScore = std::log10(std::max(1LL, likeCount)) * LIKE_MULTIPLIER;
    double commentScore = std::log10(std::max(1LL, commentCount)) * COMMENT_MULTIPLIER;
    
    double totalScore = viewScore + likeScore + commentScore;
    
    return ConvScore(static_cast<Score>(totalScore));
}
