#pragma once
/**
 * @file RankingEngine.h
 * @brief 통합 랭킹 엔진
 * 
 * 데이터 수집, 점수 계산, 정렬, 랭킹 관리를 통합하는 핵심 엔진입니다.
 * 
 * [아키텍처]
 * DataProvider → RankingEngine ← ScoreCalculator
 *                     ↓
 *              SortingStrategy
 *                     ↓
 *              Ranking Results
 * 
 * [특징]
 * - 데이터 소스 독립적 (IDataProvider 인터페이스)
 * - 점수 계산 전략 교체 가능
 * - 정렬 알고리즘 선택 가능
 * - 실시간 업데이트 지원
 */

#include "Domain/VideoMetrics.h"
#include "Adapter/IDataProvider.h"
#include "Scoring/ScoreCalculator.h"
#include "Sorting/SortingStrategy.hpp"
#include <memory>
#include <unordered_map>
#include <functional>
#include <iostream>

namespace Engine {

/**
 * @struct RankingConfig
 * @brief 랭킹 엔진 설정
 */
struct RankingConfig {
    int topK = 100;                                     // 유지할 상위 K개 수
    Sorting::Algorithm sortAlgorithm = Sorting::Algorithm::QuickSort;
    Scoring::Strategy scoreStrategy = Scoring::Strategy::Balanced;
    bool autoRefresh = false;                           // 자동 갱신 여부
    int refreshIntervalSeconds = 300;                   // 갱신 주기 (초)
};

/**
 * @struct RankingEntry
 * @brief 랭킹 항목 (순위 + 영상 정보)
 */
struct RankingEntry {
    int rank;
    Domain::VideoMetrics video;
    double previousScore = 0.0;     // 이전 점수 (변화량 계산용)
    int previousRank = 0;           // 이전 순위
};

/**
 * @class RankingEngine
 * @brief 랭킹 시스템의 핵심 엔진
 */
class RankingEngine {
public:
    /**
     * @brief 기본 생성자
     */
    RankingEngine() : config_(), calculator_(config_.scoreStrategy) {}

    /**
     * @brief 설정 지정 생성자
     */
    explicit RankingEngine(const RankingConfig& config)
        : config_(config), calculator_(config.scoreStrategy) {}

    /**
     * @brief 데이터 제공자 설정
     */
    void setDataProvider(std::unique_ptr<Adapter::IDataProvider> provider) {
        dataProvider_ = std::move(provider);
    }

    /**
     * @brief 설정 변경
     */
    void setConfig(const RankingConfig& config) {
        config_ = config;
        calculator_.setStrategy(config.scoreStrategy);
    }

    /**
     * @brief 점수 계산 전략 변경
     */
    void setScoreStrategy(Scoring::Strategy strategy) {
        config_.scoreStrategy = strategy;
        calculator_.setStrategy(strategy);
    }

    /**
     * @brief 정렬 알고리즘 변경
     */
    void setSortAlgorithm(Sorting::Algorithm algo) {
        config_.sortAlgorithm = algo;
    }

    /**
     * @brief 데이터 로드 및 초기 랭킹 구축
     * @return 성공 여부
     */
    bool build() {
        if (!dataProvider_) {
            lastError_ = "Data provider not set";
            return false;
        }

        // 데이터 조회
        Adapter::FetchOptions options;
        options.maxResults = config_.topK * 2;  // 여유분 확보
        
        auto result = dataProvider_->fetchVideos(options);
        if (!result.success) {
            lastError_ = result.errorMessage;
            return false;
        }

        videos_ = std::move(result.videos);
        
        // 점수 계산
        calculator_.calculateAll(videos_);
        
        // 정렬
        sortVideos();
        
        // Top-K 유지
        if (static_cast<int>(videos_.size()) > config_.topK) {
            videos_.resize(config_.topK);
        }
        
        // 랭킹 맵 구축
        rebuildRankMap();
        
        return true;
    }

    /**
     * @brief 외부 데이터로 랭킹 구축
     */
    bool buildFromData(std::vector<Domain::VideoMetrics> videos) {
        videos_ = std::move(videos);
        
        // 점수 계산
        calculator_.calculateAll(videos_);
        
        // 정렬
        sortVideos();
        
        // Top-K 유지
        if (static_cast<int>(videos_.size()) > config_.topK) {
            videos_.resize(config_.topK);
        }
        
        // 랭킹 맵 구축
        rebuildRankMap();
        
        return true;
    }

    /**
     * @brief 랭킹 갱신 (데이터 리프레시)
     */
    bool refresh() {
        if (!dataProvider_) {
            lastError_ = "Data provider not set";
            return false;
        }

        // 현재 영상 ID 목록 추출
        std::vector<std::string> videoIds;
        videoIds.reserve(videos_.size());
        for (const auto& v : videos_) {
            videoIds.push_back(v.id);
        }

        // 데이터 갱신
        auto refreshed = dataProvider_->refreshVideos(videoIds);
        
        // 이전 점수 저장
        std::unordered_map<std::string, double> prevScores;
        for (const auto& v : videos_) {
            prevScores[v.id] = v.score;
        }

        // 갱신된 데이터로 교체
        videos_ = std::move(refreshed);
        
        // 점수 재계산
        calculator_.calculateAll(videos_);
        
        // 정렬
        sortVideos();
        
        // 랭킹 맵 재구축
        rebuildRankMap();
        
        return true;
    }

    /**
     * @brief 상위 K개 랭킹 반환
     */
    std::vector<RankingEntry> getTopK(int k = -1) const {
        if (k < 0) k = config_.topK;
        k = std::min(k, static_cast<int>(videos_.size()));
        
        std::vector<RankingEntry> result;
        result.reserve(k);
        
        for (int i = 0; i < k; ++i) {
            RankingEntry entry;
            entry.rank = i + 1;
            entry.video = videos_[i];
            result.push_back(entry);
        }
        
        return result;
    }

    /**
     * @brief 특정 영상의 현재 순위 조회
     * @return 순위 (1-indexed), 없으면 -1
     */
    int getRank(const std::string& videoId) const {
        auto it = rankMap_.find(videoId);
        if (it != rankMap_.end()) {
            return it->second + 1;  // 1-indexed
        }
        return -1;  // 찾지 못함
    }

    /**
     * @brief 특정 순위의 영상 정보 조회
     * @param rank 순위 (1-indexed)
     * @param outVideo 결과를 받을 VideoMetrics 참조
     * @return 성공 여부
     */
    bool getVideoAtRank(int rank, Domain::VideoMetrics& outVideo) const {
        if (rank < 1 || rank > static_cast<int>(videos_.size())) {
            return false;
        }
        outVideo = videos_[rank - 1];
        return true;
    }

    /**
     * @brief 특정 순위의 영상 정보 조회 (포인터 반환)
     * @param rank 순위 (1-indexed)
     * @return 영상 정보 포인터, 없으면 nullptr
     */
    const Domain::VideoMetrics* getVideoAtRank(int rank) const {
        if (rank < 1 || rank > static_cast<int>(videos_.size())) {
            return nullptr;
        }
        return &videos_[rank - 1];
    }

    /**
     * @brief 현재 랭킹 결과를 콘솔에 출력
     */
    void printRanking(int count = 10) const {
        std::cout << "\n========== TOP " << count << " RANKING ==========\n";
        std::cout << "Sort: " << Sorting::getAlgorithmName(config_.sortAlgorithm);
        std::cout << " | Score: " << Scoring::getStrategyName(config_.scoreStrategy) << "\n";
        std::cout << "---------------------------------------\n";
        
        int limit = std::min(count, static_cast<int>(videos_.size()));
        for (int i = 0; i < limit; ++i) {
            const auto& v = videos_[i];
            std::cout << "#" << (i + 1) << " | Score: " << static_cast<int>(v.score)
                      << " | Views: " << v.viewCount
                      << " | " << v.title << "\n";
        }
        std::cout << "=======================================\n";
    }

    // Getters
    const RankingConfig& getConfig() const { return config_; }
    const std::vector<Domain::VideoMetrics>& getAllVideos() const { return videos_; }
    size_t getVideoCount() const { return videos_.size(); }
    const std::string& getLastError() const { return lastError_; }

private:
    RankingConfig config_;
    Scoring::ScoreCalculator calculator_;
    std::unique_ptr<Adapter::IDataProvider> dataProvider_;
    std::vector<Domain::VideoMetrics> videos_;
    std::unordered_map<std::string, int> rankMap_;  // videoId → index
    std::string lastError_;

    /**
     * @brief 영상 정렬 수행
     */
    void sortVideos() {
        Sorting::sort(videos_, config_.sortAlgorithm,
            [](const Domain::VideoMetrics& a, const Domain::VideoMetrics& b) {
                return a.score > b.score;  // 내림차순
            });
    }

    /**
     * @brief 랭킹 맵 재구축
     */
    void rebuildRankMap() {
        rankMap_.clear();
        for (size_t i = 0; i < videos_.size(); ++i) {
            rankMap_[videos_[i].id] = static_cast<int>(i);
        }
    }
};

} // namespace Engine

