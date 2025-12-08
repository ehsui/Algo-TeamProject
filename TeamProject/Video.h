#pragma once
/**
 * @file Video.h
 * @brief YouTube 영상 정보 구조체
 * 
 * [통합된 데이터 모델]
 * - RankEngine에서 사용하는 주요 데이터 구조
 * - CSV 파일에서 로드 가능
 * - Domain::VideoMetrics와 상호 변환 지원
 */

#include "Utility.h"
#include "Domain/VideoMetrics.h"
#include <fstream>
#include <sstream>

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
    
    // === VideoMetrics 변환 ===
    
    /**
     * @brief VideoMetrics로 변환
     */
    Domain::VideoMetrics toVideoMetrics() const;
    
    /**
     * @brief VideoMetrics에서 Video 생성
     */
    static Video fromVideoMetrics(const Domain::VideoMetrics& vm);
    
    // === CSV 로드 ===
    
    /**
     * @brief CSV 행에서 Video 생성
     * @param videoId 영상 ID
     * @param timestamp 수집 시각
     * @param views 조회수
     * @param likes 좋아요
     * @param comments 댓글 수
     * @param title 제목
     */
    static Video fromCsvRow(const string& videoId, const string& timestamp,
                            long long views, long long likes, long long comments,
                            const string& title);
    
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

// ============================================================================
// CSV 데이터 로더
// ============================================================================

/**
 * @class CsvDataLoader
 * @brief CSV 파일에서 Video 데이터를 로드하는 유틸리티 클래스
 * 
 * [CSV 형식]
 * VIDEO_ID,TIMESTAMP,VIEWS,LIKES,COMMENTS,TITLE
 * 
 * [사용 예시]
 * auto [initial, refresh] = CsvDataLoader::loadAndSplit("data.csv");
 * engine.build(initial);
 * engine.refresh(refresh);
 */
class CsvDataLoader {
public:
    /**
     * @brief CSV 파일에서 모든 Video 로드
     * @param filePath CSV 파일 경로
     * @return Video 벡터
     */
    static vector<Video> loadAll(const string& filePath);
    
    /**
     * @brief CSV 파일에서 특정 시점의 Video만 로드
     * @param filePath CSV 파일 경로
     * @param timestamp 필터링할 시점 (예: "2025-11-26")
     * @return 해당 시점의 Video 벡터
     */
    static vector<Video> loadByTimestamp(const string& filePath, const string& timestamp);
    
    /**
     * @brief CSV 파일을 두 시점으로 분리하여 로드
     * @param filePath CSV 파일 경로
     * @return pair<초기 데이터, 갱신 데이터>
     * 
     * 첫 번째 시점 → 초기 데이터 (build용)
     * 두 번째 시점 → 갱신 데이터 (refresh용)
     */
    static pair<vector<Video>, vector<Video>> loadAndSplit(const string& filePath);
    
    /**
     * @brief CSV 파일에서 고유한 시점(timestamp) 목록 추출
     * @param filePath CSV 파일 경로
     * @return 시점 문자열 벡터 (정렬됨)
     */
    static vector<string> getTimestamps(const string& filePath);
    
private:
    /**
     * @brief CSV 행 파싱 (쉼표 구분, 따옴표 처리)
     */
    static vector<string> parseCsvLine(const string& line);
};