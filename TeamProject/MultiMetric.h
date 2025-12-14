#pragma once
/**
 * @file MultiMetric.h
 * @brief 다중 지표 사전식 랭킹 (Multi-Metric Lexicographic Ranking)
 * 
 * 단일 점수 대신 여러 지표의 우선순위 체인으로 순위를 결정합니다.
 * 
 * [사전식 비교 예시]
 * 우선순위: Δviews → Δlikes → Δcomments
 * 
 * A: [1000, 50, 10]
 * B: [1000, 80, 5]
 * 
 * 비교: A vs B
 * - Δviews: 1000 == 1000 → 다음 지표
 * - Δlikes: 50 < 80 → B 승리
 * 
 * [사용 예시]
 * MultiMetricKey a, b;
 * a.metrics = {1000, 50, 10};
 * b.metrics = {1000, 80, 5};
 * 
 * if (a < b) { ... }  // 사전식 비교 (내림차순)
 */

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

// ============================================================================
// 지표 타입 정의
// ============================================================================

/// 지표 우선순위 열거형
enum class MetricType {
    DeltaViews = 0,     // 조회수 변화량
    DeltaLikes,         // 좋아요 변화량
    DeltaComments,      // 댓글 변화량
    AbsoluteViews,      // 절대 조회수
    AbsoluteLikes,      // 절대 좋아요
    AbsoluteComments,   // 절대 댓글
    Recency,            // 최근성 (업로드 시간)
    Duration,           // 영상 길이
    EngagementRate,     // 참여율 (likes/views)
    CustomScore         // 커스텀 점수
};

/// 지표 이름 반환
inline string getMetricName(MetricType type) {
    switch (type) {
        case MetricType::DeltaViews:      return "DeltaViews";
        case MetricType::DeltaLikes:      return "DeltaLikes";
        case MetricType::DeltaComments:   return "DeltaComments";
        case MetricType::AbsoluteViews:   return "Views";
        case MetricType::AbsoluteLikes:   return "Likes";
        case MetricType::AbsoluteComments:return "Comments";
        case MetricType::Recency:         return "Recency";
        case MetricType::Duration:        return "Duration";
        case MetricType::EngagementRate:  return "EngagementRate";
        case MetricType::CustomScore:     return "CustomScore";
        default:                          return "Unknown";
    }
}

// ============================================================================
// 다중 지표 키 구조체
// ============================================================================

/**
 * @struct MultiMetricKey
 * @brief 다중 지표 기반 정렬용 키
 * 
 * metrics 벡터의 인덱스가 우선순위를 나타냅니다.
 * metrics[0]이 가장 높은 우선순위입니다.
 */
struct MultiMetricKey {
    string videoId;                 // 영상 ID
    string title;                   // 제목 (최종 동점 처리용)
    vector<int64_t> metrics;        // 지표 값들 (우선순위 순서)
    
    /**
     * @brief 사전식 비교 (내림차순)
     * 
     * 첫 번째 지표부터 비교하여:
     * - 다르면 큰 값이 앞으로 (내림차순)
     * - 같으면 다음 지표로
     * - 모든 지표가 같으면 제목순 (오름차순)
     */
    bool operator<(const MultiMetricKey& other) const {
        size_t len = min(metrics.size(), other.metrics.size());
        
        for (size_t i = 0; i < len; ++i) {
            if (metrics[i] != other.metrics[i]) {
                return metrics[i] > other.metrics[i];  // 내림차순: 큰 값이 앞
            }
        }
        
        // 지표 개수가 다르면 더 많은 쪽이 앞
        if (metrics.size() != other.metrics.size()) {
            return metrics.size() > other.metrics.size();
        }
        
        // 모든 지표가 같으면 제목순
        return title < other.title;
    }
    
    bool operator>(const MultiMetricKey& other) const {
        return other < *this;
    }
    
    bool operator==(const MultiMetricKey& other) const {
        return videoId == other.videoId;
    }
    
    bool operator!=(const MultiMetricKey& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief 디버그 출력
     */
    void print() const {
        cout << "[" << videoId << "] ";
        for (size_t i = 0; i < metrics.size(); ++i) {
            if (i > 0) cout << " > ";
            cout << metrics[i];
        }
        cout << " | " << title << endl;
    }
};

// ============================================================================
// 다중 지표 설정
// ============================================================================

/**
 * @struct MultiMetricConfig
 * @brief 다중 지표 랭킹 설정
 */
struct MultiMetricConfig {
    vector<MetricType> priority;    // 지표 우선순위 (앞이 높음)
    bool descending = true;         // 내림차순 여부 (기본: 큰 값 우선)
    
    /// 기본 생성자
    MultiMetricConfig() = default;
    
    /**
     * @brief 기본 설정 (조회수 → 좋아요 → 댓글)
     */
    static MultiMetricConfig defaultConfig() {
        MultiMetricConfig cfg;
        cfg.priority.push_back(MetricType::AbsoluteViews);
        cfg.priority.push_back(MetricType::AbsoluteLikes);
        cfg.priority.push_back(MetricType::AbsoluteComments);
        return cfg;
    }
    
    /**
     * @brief 트렌딩 설정 (변화량 기반)
     */
    static MultiMetricConfig trendingConfig() {
        MultiMetricConfig cfg;
        cfg.priority.push_back(MetricType::DeltaViews);
        cfg.priority.push_back(MetricType::DeltaLikes);
        cfg.priority.push_back(MetricType::DeltaComments);
        return cfg;
    }
    
    /**
     * @brief 참여도 설정 (좋아요 → 댓글 → 조회수)
     */
    static MultiMetricConfig engagementConfig() {
        MultiMetricConfig cfg;
        cfg.priority.push_back(MetricType::AbsoluteLikes);
        cfg.priority.push_back(MetricType::AbsoluteComments);
        cfg.priority.push_back(MetricType::AbsoluteViews);
        return cfg;
    }
    
    /**
     * @brief 설정 출력
     */
    void print() const {
        cout << "MultiMetric Priority: ";
        for (size_t i = 0; i < priority.size(); ++i) {
            if (i > 0) cout << " > ";
            cout << getMetricName(priority[i]);
        }
        cout << (descending ? " (DESC)" : " (ASC)") << endl;
    }
};

// ============================================================================
// 정렬 함수 (BasicSort.hpp 스타일)
// ============================================================================

namespace MultiMetricSort {

/**
 * @brief 다중 지표 퀵 정렬
 */
inline void quicksort(vector<MultiMetricKey>& data, int left, int right) {
    if (left >= right) return;
    
    MultiMetricKey pivot = data[(left + right) / 2];
    int i = left, j = right;
    
    while (i <= j) {
        while (data[i] < pivot) i++;
        while (data[j] > pivot) j--;  // 수정: data[i] → data[j]
        if (i <= j) {
            swap(data[i], data[j]);
            i++; j--;
        }
    }
    
    if (left < j) quicksort(data, left, j);
    if (i < right) quicksort(data, i, right);
}

inline void quicksort(vector<MultiMetricKey>& data) {
    if (data.size() <= 1) return;
    quicksort(data, 0, static_cast<int>(data.size()) - 1);
}

/**
 * @brief std::sort 래퍼
 */
inline void stdsort(vector<MultiMetricKey>& data) {
    sort(data.begin(), data.end());
}

/**
 * @brief Top-K 선택 후 정렬
 */
inline vector<MultiMetricKey> selectTopK(vector<MultiMetricKey>& data, int k) {
    if (data.empty() || k <= 0) return {};
    
    k = min(k, static_cast<int>(data.size()));
    
    // nth_element로 K번째까지 파티셔닝
    nth_element(data.begin(), data.begin() + k, data.end());
    
    // 상위 K개 복사 후 정렬
    vector<MultiMetricKey> topK(data.begin(), data.begin() + k);
    sort(topK.begin(), topK.end());
    
    return topK;
}

} // namespace MultiMetricSort

// ============================================================================
// 유틸리티 함수
// ============================================================================

/**
 * @brief Video 정보에서 MultiMetricKey 생성
 * 
 * @param videoId 영상 ID
 * @param title 영상 제목
 * @param views 조회수
 * @param likes 좋아요
 * @param comments 댓글
 * @param config 지표 설정
 * @param deltaViews 조회수 변화량 (옵션)
 * @param deltaLikes 좋아요 변화량 (옵션)
 * @param deltaComments 댓글 변화량 (옵션)
 */
inline MultiMetricKey createMultiMetricKey(
    const string& videoId,
    const string& title,
    int64_t views,
    int64_t likes,
    int64_t comments,
    const MultiMetricConfig& config,
    int64_t deltaViews = 0,
    int64_t deltaLikes = 0,
    int64_t deltaComments = 0,
    int64_t recency = 0,
    int duration = 0
) {
    MultiMetricKey key;
    key.videoId = videoId;
    key.title = title;
    
    // 설정된 우선순위에 따라 지표 추가
    for (MetricType type : config.priority) {
        int64_t value = 0;
        switch (type) {
            case MetricType::DeltaViews:      value = deltaViews; break;
            case MetricType::DeltaLikes:      value = deltaLikes; break;
            case MetricType::DeltaComments:   value = deltaComments; break;
            case MetricType::AbsoluteViews:   value = views; break;
            case MetricType::AbsoluteLikes:   value = likes; break;
            case MetricType::AbsoluteComments:value = comments; break;
            case MetricType::Recency:         value = recency; break;
            case MetricType::Duration:        value = duration; break;
            case MetricType::EngagementRate:  
                value = views > 0 ? (likes * 10000 / views) : 0;  // 0.01% 단위
                break;
            default: break;
        }
        key.metrics.push_back(value);
    }
    
    return key;
}

