#pragma once
/**
 * @file Video.h
 * @brief YouTube 영상 정보 구조체 (레거시)
 * 
 * [주의] 새 코드에서는 다음을 사용하세요:
 * - Domain/VideoMetrics.h: 내부 도메인 모델
 * - Api/YouTubeApiTypes.h: API 응답 DTO
 * 
 * 이 파일은 기존 코드와의 호환성을 위해 유지됩니다.
 */

#include "Utility.h"

// 전방 선언
struct key;

/**
 * @struct YouTubeVideoInfo
 * @brief YouTube 영상 정보를 담는 구조체
 * 
 * [필드 분류]
 * - 기본 정보: ID, 제목, 채널 정보
 * - 통계: 조회수, 좋아요, 댓글
 * - 상세: 영상 길이, 화질, 자막 등
 * - 상태: 공개 여부, 임베드 가능 여부
 * - 메타: 수집 시각 등
 */
typedef struct YouTubeVideoInfo {
    // === 기본 정보 ===
    string videoId;            // 영상 고유 ID (예: "dQw4w9WgXcQ")
    string channelId;          // 채널 ID
    string channelTitle;       // 채널 이름
    string title;              // 영상 제목
    string description;        // 영상 설명
    string categoryId;         // 카테고리 ID
    vector<string> tags;       // 태그 목록
    string publishedAt;        // 업로드 일시 (ISO 8601)
    
    // === 계산된 점수 ===
    Score score = 0;

    // === 썸네일 ===
    string thumbnailUrl;

    // === 통계 ===
    long long viewCount = 0;
    long long likeCount = 0;
    long long commentCount = 0;

    // === 상세 정보 ===
    int durationSeconds = 0;   // 영상 길이 (초)
    string definition;         // "hd" 또는 "sd"
    string dimension;          // "2d" 또는 "3d"
    bool caption = false;      // 자막 존재 여부
    bool licensedContent = false;
    vector<string> regionBlocked;  // 차단된 국가 코드

    // === 상태 ===
    string privacyStatus;      // "public", "private", "unlisted"
    bool embeddable = true;

    // === 메타 데이터 ===
    string fetchTimestamp;     // 데이터 수집 시각

    // === 메서드 ===
    
    /**
     * @brief 다른 Video 정보로 업데이트
     */
    void updateInfo(const YouTubeVideoInfo& v);
    
    /**
     * @brief 정렬용 경량 키 생성
     */
    key makekey() const;
    
    /**
     * @brief 점수 계산 및 할당
     */
    void calculateScore();
    
} Video;

/**
 * @struct key
 * @brief 정렬용 경량 키 구조체
 * 
 * Video 전체를 정렬하면 메모리 이동 비용이 크므로,
 * 정렬에는 이 경량 키만 사용합니다.
 */
struct key {
    Score Value = 0;       // 점수
    string videoId;        // 영상 ID (원본 참조용)
    string title;          // 제목 (동점 시 정렬용)
    
    // 비교 연산자 (내림차순: 높은 점수 우선)
    bool operator<(const key& other) const {
        if (Value != other.Value) {
            return Value > other.Value;  // 높은 점수가 앞으로
        }
        return title < other.title;      // 동점 시 제목순
    }
    
    bool operator>(const key& other) const {
        return other < *this;
    }
    
    bool operator==(const key& other) const {
        return videoId == other.videoId;
    }
    
    bool operator!=(const key& other) const {
        return !(*this == other);
    }
};