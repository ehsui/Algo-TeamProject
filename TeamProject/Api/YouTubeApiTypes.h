#pragma once
/**
 * @file YouTubeApiTypes.h
 * @brief YouTube API 응답 DTO (Data Transfer Object)
 * 
 * 이 파일은 YouTube API의 실제 응답 구조를 그대로 반영합니다.
 * API 포맷이 변경되면 이 파일만 수정하면 됩니다.
 * 
 * [책임]
 * - API 응답 JSON 구조와 1:1 매핑
 * - 파싱된 원시 데이터 저장
 * 
 * [주의]
 * - 비즈니스 로직을 포함하지 않습니다.
 * - 내부 도메인 모델(VideoMetrics)과 직접 연결되지 않습니다.
 */

#include <string>
#include <vector>

namespace Api {
namespace YouTube {

/**
 * @struct ThumbnailInfo
 * @brief 썸네일 정보 (API snippet.thumbnails 대응)
 */
struct ThumbnailInfo {
    std::string url;
    int width = 0;
    int height = 0;
};

/**
 * @struct VideoSnippet
 * @brief 영상 기본 정보 (API snippet 대응)
 */
struct VideoSnippet {
    std::string publishedAt;     // ISO 8601 형식 문자열
    std::string channelId;
    std::string title;
    std::string description;
    std::string channelTitle;
    std::vector<std::string> tags;
    std::string categoryId;
    ThumbnailInfo defaultThumbnail;
    ThumbnailInfo mediumThumbnail;
    ThumbnailInfo highThumbnail;
};

/**
 * @struct VideoStatistics
 * @brief 영상 통계 (API statistics 대응)
 * 
 * [주의] YouTube API는 이 값들을 문자열로 반환합니다.
 */
struct VideoStatistics {
    std::string viewCount;       // 문자열로 저장 (API 원본)
    std::string likeCount;
    std::string commentCount;
};

/**
 * @struct VideoContentDetails
 * @brief 영상 상세 정보 (API contentDetails 대응)
 */
struct VideoContentDetails {
    std::string duration;        // ISO 8601 duration (예: "PT1H2M3S")
    std::string dimension;       // "2d" 또는 "3d"
    std::string definition;      // "hd" 또는 "sd"
    bool caption = false;
    bool licensedContent = false;
    std::vector<std::string> regionRestriction;
};

/**
 * @struct VideoStatus
 * @brief 영상 상태 (API status 대응)
 */
struct VideoStatus {
    std::string uploadStatus;
    std::string privacyStatus;   // "public", "private", "unlisted"
    bool embeddable = true;
    bool madeForKids = false;
};

/**
 * @struct VideoItem
 * @brief YouTube API video.list 응답의 단일 아이템
 */
struct VideoItem {
    std::string kind;            // "youtube#video"
    std::string etag;
    std::string id;              // 영상 ID
    
    VideoSnippet snippet;
    VideoStatistics statistics;
    VideoContentDetails contentDetails;
    VideoStatus status;
};

/**
 * @struct VideoListResponse
 * @brief YouTube API video.list 전체 응답
 */
struct VideoListResponse {
    std::string kind;            // "youtube#videoListResponse"
    std::string etag;
    std::string nextPageToken;
    std::string prevPageToken;
    
    struct PageInfo {
        int totalResults = 0;
        int resultsPerPage = 0;
    } pageInfo;
    
    std::vector<VideoItem> items;
};

} // namespace YouTube
} // namespace Api

