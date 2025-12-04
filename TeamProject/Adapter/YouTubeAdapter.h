#pragma once
/**
 * @file YouTubeAdapter.h
 * @brief YouTube API 응답을 도메인 모델로 변환하는 어댑터
 * 
 * [책임]
 * - YouTube API 응답 DTO → VideoMetrics 변환
 * - ISO 8601 duration 파싱 (예: "PT1H2M3S" → 3723초)
 * - ISO 8601 datetime 파싱
 * - 문자열 숫자 → 정수 변환
 * 
 * [사용 예시]
 * Api::YouTube::VideoItem apiItem = parseFromJson(jsonString);
 * Domain::VideoMetrics metrics = YouTubeAdapter::toVideoMetrics(apiItem);
 */

#include "Api/YouTubeApiTypes.h"
#include "Domain/VideoMetrics.h"
#include "Adapter/IDataProvider.h"
#include <sstream>
#include <regex>
#include <chrono>
#include <iomanip>

namespace Adapter {

/**
 * @class YouTubeAdapter
 * @brief YouTube API DTO를 도메인 모델로 변환하는 정적 유틸리티 클래스
 */
class YouTubeAdapter {
public:
    /**
     * @brief YouTube API VideoItem을 VideoMetrics로 변환
     */
    static Domain::VideoMetrics toVideoMetrics(const Api::YouTube::VideoItem& item) {
        Domain::VideoMetrics metrics;
        
        // 기본 정보
        metrics.id = item.id;
        metrics.title = item.snippet.title;
        metrics.channelId = item.snippet.channelId;
        metrics.channelName = item.snippet.channelTitle;
        
        // 통계 (문자열 → 정수 변환)
        metrics.viewCount = safeStringToInt64(item.statistics.viewCount);
        metrics.likeCount = safeStringToInt64(item.statistics.likeCount);
        metrics.commentCount = safeStringToInt64(item.statistics.commentCount);
        
        // 영상 길이 (ISO 8601 duration 파싱)
        metrics.durationSeconds = parseIsoDuration(item.contentDetails.duration);
        
        // 시간 정보
        metrics.publishedAt = parseIsoDateTime(item.snippet.publishedAt);
        metrics.fetchedAt = std::chrono::system_clock::now();
        
        return metrics;
    }

    /**
     * @brief VideoListResponse 전체를 VideoMetrics 벡터로 변환
     */
    static std::vector<Domain::VideoMetrics> toVideoMetricsList(
        const Api::YouTube::VideoListResponse& response) {
        
        std::vector<Domain::VideoMetrics> result;
        result.reserve(response.items.size());
        
        for (const auto& item : response.items) {
            result.push_back(toVideoMetrics(item));
        }
        
        return result;
    }

    /**
     * @brief ISO 8601 duration 문자열을 초 단위로 변환
     * @param duration ISO 8601 형식 (예: "PT1H2M3S", "PT5M", "P1DT2H")
     * @return 총 초 수
     */
    static int parseIsoDuration(const std::string& duration) {
        if (duration.empty()) return 0;
        
        int totalSeconds = 0;
        int days = 0, hours = 0, minutes = 0, seconds = 0;
        
        // 정규식으로 파싱: P[nD]T[nH][nM][nS]
        std::regex pattern(R"(P(?:(\d+)D)?T?(?:(\d+)H)?(?:(\d+)M)?(?:(\d+)S)?)");
        std::smatch matches;
        
        if (std::regex_match(duration, matches, pattern)) {
            if (matches[1].matched) days = std::stoi(matches[1].str());
            if (matches[2].matched) hours = std::stoi(matches[2].str());
            if (matches[3].matched) minutes = std::stoi(matches[3].str());
            if (matches[4].matched) seconds = std::stoi(matches[4].str());
        }
        
        totalSeconds = days * 86400 + hours * 3600 + minutes * 60 + seconds;
        return totalSeconds;
    }

    /**
     * @brief ISO 8601 datetime 문자열을 time_point로 변환
     * @param datetime ISO 8601 형식 (예: "2024-01-15T12:30:00Z")
     */
    static std::chrono::system_clock::time_point parseIsoDateTime(
        const std::string& datetime) {
        
        if (datetime.empty()) {
            return std::chrono::system_clock::now();
        }
        
        std::tm tm = {};
        std::istringstream ss(datetime);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        
        if (ss.fail()) {
            return std::chrono::system_clock::now();
        }
        
        // UTC로 변환
        auto time_c = std::mktime(&tm);
        return std::chrono::system_clock::from_time_t(time_c);
    }

private:
    /**
     * @brief 안전한 문자열 → int64 변환
     */
    static int64_t safeStringToInt64(const std::string& str) {
        if (str.empty()) return 0;
        try {
            return std::stoll(str);
        } catch (...) {
            return 0;
        }
    }
};

} // namespace Adapter

