#pragma once
/**
 * @file DummyDataProvider.h
 * @brief 테스트용 더미 데이터 제공자
 * 
 * 실제 YouTube API 없이도 랭킹 시스템을 테스트할 수 있도록
 * 더미 데이터를 생성하는 구현체입니다.
 * 
 * [사용 용도]
 * - 단위 테스트
 * - 개발 중 API 없이 동작 확인
 * - 정렬 알고리즘 성능 벤치마크
 */

#include "IDataProvider.h"
#include <random>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace Adapter {

/**
 * @class DummyDataProvider
 * @brief 테스트용 더미 데이터 생성 및 제공
 */
class DummyDataProvider : public IDataProvider {
public:
    /**
     * @brief 생성자
     * @param videoCount 생성할 더미 영상 수
     * @param seed 난수 시드 (재현 가능한 테스트용)
     */
    explicit DummyDataProvider(int videoCount = 1000, unsigned int seed = 42)
        : videoCount_(videoCount), rng_(seed) {
        generateDummyData();
    }

    FetchResult fetchVideos(const FetchOptions& options = {}) override {
        FetchResult result;
        
        int startIdx = 0;
        if (!options.pageToken.empty()) {
            try {
                startIdx = std::stoi(options.pageToken);
            } catch (...) {
                startIdx = 0;
            }
        }
        
        int endIdx = std::min(startIdx + options.maxResults, 
                              static_cast<int>(cachedVideos_.size()));
        
        result.videos = std::vector<Domain::VideoMetrics>(
            cachedVideos_.begin() + startIdx,
            cachedVideos_.begin() + endIdx
        );
        
        result.totalResults = static_cast<int>(cachedVideos_.size());
        result.hasMore = endIdx < static_cast<int>(cachedVideos_.size());
        
        if (result.hasMore) {
            result.nextPageToken = std::to_string(endIdx);
        }
        
        result.success = true;
        return result;
    }

    bool fetchVideoById(const std::string& videoId, Domain::VideoMetrics& outVideo) override {
        auto it = std::find_if(cachedVideos_.begin(), cachedVideos_.end(),
            [&videoId](const Domain::VideoMetrics& v) { return v.id == videoId; });
        
        if (it != cachedVideos_.end()) {
            outVideo = *it;
            return true;
        }
        return false;
    }

    std::vector<Domain::VideoMetrics> refreshVideos(
        const std::vector<std::string>& videoIds) override {
        
        std::vector<Domain::VideoMetrics> result;
        for (const auto& id : videoIds) {
            Domain::VideoMetrics video;
            if (fetchVideoById(id, video)) {
                // 시뮬레이션: 랜덤하게 조회수/좋아요 증가
                std::uniform_int_distribution<int64_t> viewDist(100, 10000);
                std::uniform_int_distribution<int64_t> likeDist(10, 500);
                video.viewCount += viewDist(rng_);
                video.likeCount += likeDist(rng_);
                video.fetchedAt = std::chrono::system_clock::now();
                result.push_back(video);
            }
        }
        return result;
    }

    std::string getSourceName() const override {
        return "DummyDataProvider";
    }

    bool isAvailable() const override {
        return true;  // 항상 사용 가능
    }

    /**
     * @brief 더미 데이터 재생성
     */
    void regenerateData(int newCount = -1) {
        if (newCount > 0) videoCount_ = newCount;
        cachedVideos_.clear();
        generateDummyData();
    }

    /**
     * @brief 캐시된 전체 데이터 반환 (테스트용)
     */
    const std::vector<Domain::VideoMetrics>& getAllCachedData() const {
        return cachedVideos_;
    }

private:
    int videoCount_;
    std::mt19937 rng_;
    std::vector<Domain::VideoMetrics> cachedVideos_;

    void generateDummyData() {
        cachedVideos_.reserve(videoCount_);
        
        // 다양한 분포를 사용해 현실적인 데이터 생성
        std::uniform_int_distribution<int64_t> viewDist(1000, 100'000'000);      // 조회수
        std::uniform_int_distribution<int64_t> likeDist(100, 1'000'000);         // 좋아요
        std::uniform_int_distribution<int64_t> commentDist(10, 50'000);          // 댓글
        std::uniform_int_distribution<int> durationDist(30, 7200);               // 30초~2시간
        std::uniform_int_distribution<int> daysDist(0, 365);                     // 업로드 일수
        
        const std::vector<std::string> titlePrefixes = {
            "[ENG SUB]", "[Official MV]", "[LIVE]", "[COVER]", "",
            "[4K]", "[Lyrics]", "[Reaction]", "[Tutorial]", "[Vlog]"
        };
        
        const std::vector<std::string> titleContents = {
            "Amazing Video", "Best Moments", "Epic Compilation",
            "How To Guide", "Top 10 List", "Full Documentary",
            "Music Video", "Game Highlights", "Cooking Show",
            "Travel Vlog", "Tech Review", "Comedy Sketch"
        };
        
        const std::vector<std::string> channelNames = {
            "PopularChannel", "TechReview", "MusicWorld",
            "GamingPro", "CookingMaster", "TravelDiary",
            "ComedyKing", "ScienceExplained", "FitnessGuru"
        };
        
        for (int i = 0; i < videoCount_; ++i) {
            Domain::VideoMetrics video;
            
            // ID 생성 (YouTube 스타일: 11자 영숫자)
            video.id = generateVideoId(i);
            
            // 제목 생성
            std::uniform_int_distribution<size_t> prefixDist(0, titlePrefixes.size() - 1);
            std::uniform_int_distribution<size_t> contentDist(0, titleContents.size() - 1);
            video.title = titlePrefixes[prefixDist(rng_)] + " " + 
                         titleContents[contentDist(rng_)] + " #" + std::to_string(i + 1);
            
            // 채널 정보
            std::uniform_int_distribution<size_t> channelDist(0, channelNames.size() - 1);
            video.channelId = "UC" + generateVideoId(channelDist(rng_));
            video.channelName = channelNames[channelDist(rng_)];
            
            // 통계
            video.viewCount = viewDist(rng_);
            video.likeCount = std::min(likeDist(rng_), video.viewCount / 10);  // 현실적인 비율
            video.commentCount = std::min(commentDist(rng_), video.likeCount);
            video.durationSeconds = durationDist(rng_);
            
            // 시간
            auto now = std::chrono::system_clock::now();
            auto daysAgo = std::chrono::hours(24 * daysDist(rng_));
            video.publishedAt = now - daysAgo;
            video.fetchedAt = now;
            
            cachedVideos_.push_back(std::move(video));
        }
    }
    
    std::string generateVideoId(int seed) {
        static const char chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";
        
        std::mt19937 localRng(seed);
        std::uniform_int_distribution<size_t> dist(0, sizeof(chars) - 2);
        
        std::string id;
        id.reserve(11);
        for (int i = 0; i < 11; ++i) {
            id += chars[dist(localRng)];
        }
        return id;
    }
};

} // namespace Adapter

