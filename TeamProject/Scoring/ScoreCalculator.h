#pragma once
/**
 * @file ScoreCalculator.h
 * @brief 점수 계산 전략 패턴
 * 
 * 다양한 점수 계산 방식을 지원하며, 런타임에 전략을 교체할 수 있습니다.
 * 
 * [지원 전략]
 * - ViewWeighted: 조회수 중심 점수
 * - EngagementBased: 참여도(좋아요, 댓글) 중심
 * - Trending: 최근성 + 성장률 중심
 * - Custom: 사용자 정의 가중치
 */

#include "Domain/VideoMetrics.h"
#include <functional>
#include <cmath>
#include <algorithm>
#include <chrono>

namespace Scoring {

/**
 * @enum Strategy
 * @brief 점수 계산 전략 열거형
 */
enum class Strategy {
    ViewWeighted,       // 조회수 중심
    EngagementBased,    // 참여도 중심
    Trending,           // 트렌딩 (최근성 가중)
    Balanced,           // 균형 잡힌 점수
    Custom              // 사용자 정의
};

/**
 * @struct ScoreWeights
 * @brief 점수 계산 가중치 설정
 */
struct ScoreWeights {
    double viewWeight = 1.0;        // 조회수 가중치
    double likeWeight = 2.0;        // 좋아요 가중치
    double commentWeight = 3.0;     // 댓글 가중치
    double recencyWeight = 1.5;     // 최근성 가중치
    double durationPenalty = 0.1;   // 영상 길이 패널티 (너무 짧거나 긴 영상)
    
    // 정규화 파라미터 (스케일 조정)
    double viewScale = 100000.0;
    double likeScale = 1000.0;
    double commentScale = 100.0;
};

/**
 * @class ScoreCalculator
 * @brief 점수 계산기 클래스
 */
class ScoreCalculator {
public:
    using CustomScoreFunc = std::function<double(const Domain::VideoMetrics&)>;

    /**
     * @brief 기본 생성자 (Balanced 전략 사용)
     */
    ScoreCalculator() : strategy_(Strategy::Balanced) {}

    /**
     * @brief 전략 지정 생성자
     */
    explicit ScoreCalculator(Strategy strategy) : strategy_(strategy) {}

    /**
     * @brief 커스텀 가중치로 생성
     */
    explicit ScoreCalculator(const ScoreWeights& weights) 
        : strategy_(Strategy::Custom), weights_(weights) {}

    /**
     * @brief 커스텀 함수로 생성
     */
    explicit ScoreCalculator(CustomScoreFunc func) 
        : strategy_(Strategy::Custom), customFunc_(std::move(func)) {}

    /**
     * @brief 단일 영상의 점수 계산
     */
    double calculate(const Domain::VideoMetrics& video) const {
        if (customFunc_) {
            return customFunc_(video);
        }

        switch (strategy_) {
            case Strategy::ViewWeighted:
                return calculateViewWeighted(video);
            case Strategy::EngagementBased:
                return calculateEngagementBased(video);
            case Strategy::Trending:
                return calculateTrending(video);
            case Strategy::Balanced:
            default:
                return calculateBalanced(video);
        }
    }

    /**
     * @brief 영상 목록 전체의 점수 계산 및 할당
     */
    void calculateAll(std::vector<Domain::VideoMetrics>& videos) const {
        for (auto& video : videos) {
            video.score = calculate(video);
        }
    }

    /**
     * @brief 전략 변경
     */
    void setStrategy(Strategy strategy) {
        strategy_ = strategy;
        customFunc_ = nullptr;  // 기본 전략 사용 시 커스텀 함수 초기화
    }

    /**
     * @brief 가중치 설정
     */
    void setWeights(const ScoreWeights& weights) {
        weights_ = weights;
    }

    /**
     * @brief 커스텀 점수 함수 설정
     */
    void setCustomFunction(CustomScoreFunc func) {
        customFunc_ = std::move(func);
    }

    Strategy getStrategy() const { return strategy_; }
    const ScoreWeights& getWeights() const { return weights_; }

private:
    Strategy strategy_;
    ScoreWeights weights_;
    CustomScoreFunc customFunc_;

    /**
     * @brief 조회수 중심 점수 계산
     * - 조회수가 가장 중요한 지표
     * - 좋아요/댓글은 보조 지표로만 사용
     */
    double calculateViewWeighted(const Domain::VideoMetrics& video) const {
        double viewScore = std::log10(std::max(1.0, static_cast<double>(video.viewCount)));
        double engagementBonus = 
            std::log10(std::max(1.0, static_cast<double>(video.likeCount))) * 0.2 +
            std::log10(std::max(1.0, static_cast<double>(video.commentCount))) * 0.1;
        
        return viewScore * 100.0 + engagementBonus * 10.0;
    }

    /**
     * @brief 참여도 중심 점수 계산
     * - 조회수 대비 좋아요/댓글 비율이 중요
     * - 높은 참여도 = 양질의 콘텐츠
     */
    double calculateEngagementBased(const Domain::VideoMetrics& video) const {
        if (video.viewCount == 0) return 0.0;

        double likeRatio = static_cast<double>(video.likeCount) / video.viewCount;
        double commentRatio = static_cast<double>(video.commentCount) / video.viewCount;
        
        // 참여율 점수 (0~100 스케일)
        double engagementRate = (likeRatio * 100.0 + commentRatio * 1000.0);
        
        // 최소 조회수 임계값 (노이즈 방지)
        double viewMultiplier = std::min(1.0, video.viewCount / 10000.0);
        
        return engagementRate * viewMultiplier * 100.0;
    }

    /**
     * @brief 트렌딩 점수 계산
     * - 최근 업로드된 영상에 가중치
     * - 시간당 조회수 증가율 고려
     */
    double calculateTrending(const Domain::VideoMetrics& video) const {
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::hours>(
            now - video.publishedAt).count();
        
        // 시간당 조회수 (velocity)
        double hoursAge = std::max(1.0, static_cast<double>(age));
        double velocity = video.viewCount / hoursAge;
        
        // 최근성 점수 (지수 감쇠)
        double recencyScore = std::exp(-age / (24.0 * 7.0));  // 1주일 반감기
        
        // 기본 점수
        double baseScore = std::log10(std::max(1.0, static_cast<double>(video.viewCount)));
        
        return (baseScore * 50.0) + (velocity * 0.01) + (recencyScore * 200.0);
    }

    /**
     * @brief 균형 잡힌 점수 계산 (기본 전략)
     * - 모든 지표를 종합적으로 고려
     * - 가중치 기반 선형 결합
     */
    double calculateBalanced(const Domain::VideoMetrics& video) const {
        // 로그 스케일로 정규화
        double viewNorm = std::log10(std::max(1.0, video.viewCount / weights_.viewScale + 1.0));
        double likeNorm = std::log10(std::max(1.0, video.likeCount / weights_.likeScale + 1.0));
        double commentNorm = std::log10(std::max(1.0, video.commentCount / weights_.commentScale + 1.0));
        
        // 최근성 점수
        auto now = std::chrono::system_clock::now();
        auto ageHours = std::chrono::duration_cast<std::chrono::hours>(
            now - video.publishedAt).count();
        double recencyNorm = 1.0 / (1.0 + ageHours / (24.0 * 30.0));  // 30일 기준
        
        // 가중치 적용 합산
        double score = 
            viewNorm * weights_.viewWeight +
            likeNorm * weights_.likeWeight +
            commentNorm * weights_.commentWeight +
            recencyNorm * weights_.recencyWeight;
        
        return score * 100.0;  // 0~1000 스케일로 조정
    }
};

// ============================================================================
// 편의 함수
// ============================================================================

/**
 * @brief 전략 이름 반환
 */
inline std::string getStrategyName(Strategy strategy) {
    switch (strategy) {
        case Strategy::ViewWeighted:    return "View Weighted";
        case Strategy::EngagementBased: return "Engagement Based";
        case Strategy::Trending:        return "Trending";
        case Strategy::Balanced:        return "Balanced";
        case Strategy::Custom:          return "Custom";
        default:                        return "Unknown";
    }
}

} // namespace Scoring

