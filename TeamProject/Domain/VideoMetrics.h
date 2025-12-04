#pragma once
/**
 * @file VideoMetrics.h
 * @brief 내부 도메인 모델 정의
 * 
 * 이 파일은 API 포맷과 완전히 독립적인 내부 도메인 모델을 정의합니다.
 * API 응답 형식이 변경되어도 이 파일은 수정할 필요가 없습니다.
 * 
 * [아키텍처 레이어]
 * API 응답 → Adapter → VideoMetrics (도메인 모델) → RankEngine → 랭킹 결과
 */

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace Domain {

/**
 * @struct VideoMetrics
 * @brief 영상의 랭킹 계산에 필요한 핵심 지표만 포함하는 도메인 모델
 * 
 * API 포맷에 종속되지 않는 추상화된 데이터 구조입니다.
 * 어떤 플랫폼(YouTube, Twitch 등)에서 데이터가 오든 이 형식으로 변환됩니다.
 */
struct VideoMetrics {
    // === 식별자 ===
    std::string id;              // 영상 고유 ID
    std::string title;           // 영상 제목
    std::string channelId;       // 채널 ID
    std::string channelName;     // 채널명

    // === 핵심 통계 (랭킹 계산에 사용) ===
    int64_t viewCount = 0;       // 조회수
    int64_t likeCount = 0;       // 좋아요 수
    int64_t commentCount = 0;    // 댓글 수
    int durationSeconds = 0;     // 영상 길이 (초)

    // === 시간 정보 ===
    std::chrono::system_clock::time_point publishedAt;  // 업로드 시각
    std::chrono::system_clock::time_point fetchedAt;    // 데이터 수집 시각

    // === 계산된 점수 ===
    double score = 0.0;          // 최종 랭킹 점수

    // 비교 연산자 (점수 기반 내림차순 정렬)
    bool operator<(const VideoMetrics& other) const {
        if (score != other.score) return score > other.score;  // 높은 점수 우선
        return title < other.title;  // 동점 시 제목순
    }

    bool operator>(const VideoMetrics& other) const {
        return other < *this;
    }

    bool operator==(const VideoMetrics& other) const {
        return id == other.id;
    }
};

/**
 * @struct RankingKey
 * @brief 정렬 시 사용하는 경량 키 구조체
 * 
 * 전체 VideoMetrics를 정렬하면 메모리 이동 비용이 크므로,
 * 정렬에는 경량 키만 사용하고 필요 시 원본을 참조합니다.
 */
struct RankingKey {
    double score = 0.0;
    std::string videoId;
    std::string title;

    bool operator<(const RankingKey& other) const {
        if (score != other.score) return score > other.score;
        return title < other.title;
    }

    bool operator>(const RankingKey& other) const {
        return other < *this;
    }

    bool operator==(const RankingKey& other) const {
        return videoId == other.videoId;
    }

    bool operator!=(const RankingKey& other) const {
        return !(*this == other);
    }

    // Score 타입과의 호환성을 위한 변환
    explicit operator double() const { return score; }
};

/**
 * @struct ChannelMetrics
 * @brief 채널 관련 지표 (향후 확장용)
 */
struct ChannelMetrics {
    std::string id;
    std::string name;
    int64_t subscriberCount = 0;
    int64_t totalVideoCount = 0;
};

} // namespace Domain

