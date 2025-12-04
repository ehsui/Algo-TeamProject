#pragma once
/**
 * @file IDataProvider.h
 * @brief 데이터 제공자 인터페이스
 * 
 * 이 인터페이스를 통해 다양한 데이터 소스를 추상화합니다:
 * - YouTube API
 * - 테스트용 더미 데이터
 * - 다른 플랫폼 (Twitch, TikTok 등)
 * - 로컬 캐시/파일
 * 
 * [사용 예시]
 * std::unique_ptr<IDataProvider> provider = std::make_unique<YouTubeDataProvider>();
 * auto videos = provider->fetchVideos(query);
 */

#include "Domain/VideoMetrics.h"
#include <vector>
#include <string>
#include <memory>

namespace Adapter {

/**
 * @struct FetchOptions
 * @brief 데이터 조회 옵션
 */
struct FetchOptions {
    int maxResults = 50;                    // 최대 결과 수
    std::string pageToken;                  // 페이지네이션 토큰
    std::string categoryId;                 // 카테고리 필터
    std::string regionCode = "KR";          // 지역 코드
    bool includeTrending = true;            // 인기 동영상 포함 여부
};

/**
 * @struct FetchResult
 * @brief 데이터 조회 결과
 */
struct FetchResult {
    std::vector<Domain::VideoMetrics> videos;
    std::string nextPageToken;
    int totalResults = 0;
    bool hasMore = false;
    std::string errorMessage;               // 에러 발생 시 메시지
    bool success = true;
};

/**
 * @interface IDataProvider
 * @brief 데이터 제공자 인터페이스 (추상 클래스)
 * 
 * 모든 데이터 소스는 이 인터페이스를 구현해야 합니다.
 * 이를 통해 랭킹 엔진은 데이터 소스에 독립적으로 동작합니다.
 */
class IDataProvider {
public:
    virtual ~IDataProvider() = default;

    /**
     * @brief 영상 목록 조회
     * @param options 조회 옵션
     * @return 조회 결과
     */
    virtual FetchResult fetchVideos(const FetchOptions& options = {}) = 0;

    /**
     * @brief 특정 영상 조회
     * @param videoId 영상 ID
     * @param outVideo 결과를 받을 VideoMetrics 참조
     * @return 성공 여부 (찾지 못하면 false)
     */
    virtual bool fetchVideoById(const std::string& videoId, Domain::VideoMetrics& outVideo) = 0;

    /**
     * @brief 영상 정보 업데이트 (실시간 갱신용)
     * @param videoIds 업데이트할 영상 ID 목록
     * @return 업데이트된 영상 목록
     */
    virtual std::vector<Domain::VideoMetrics> refreshVideos(
        const std::vector<std::string>& videoIds) = 0;

    /**
     * @brief 데이터 소스 이름 반환
     */
    virtual std::string getSourceName() const = 0;

    /**
     * @brief 연결 상태 확인
     */
    virtual bool isAvailable() const = 0;
};

/**
 * @brief 데이터 제공자 팩토리 함수 타입
 */
using DataProviderFactory = std::unique_ptr<IDataProvider>(*)();

} // namespace Adapter

