#pragma once
/**
 * @file RankingUI.h
 * @brief 랭킹 테이블 및 상세 정보 UI
 */

#include "ConsoleUI.h"
#include <vector>
#include <iomanip>

namespace UI {

// ============================================================================
// 랭킹 테이블
// ============================================================================

/**
 * @struct RankingRow
 * @brief 랭킹 테이블 행 데이터
 */
struct RankingRow {
    int rank;
    std::string title;
    std::string channel;
    int64_t score;
    int64_t views;
    int64_t likes;
    int rankChange;  // 순위 변동 (+: 상승, -: 하락, 0: 유지)
};

/**
 * @brief 랭킹 테이블 헤더 출력
 */
inline void printRankingHeader(const std::string& sortAlgo = "", const std::string& scoreStrategy = "") {
    using namespace Color;
    
    const int W = 85;
    
    std::cout << "\n";
    std::cout << apply(BRIGHT_RED, boxTop(W)) << "\n";
    std::cout << apply(BRIGHT_RED, boxRow("", W)) << "\n";
    
    // 제목
    std::cout << apply(BRIGHT_RED, boxRow(
        "   " + std::string(apply(BRIGHT_WHITE, "▶ 실시간 랭킹  |  Live Rankings")), W)) << "\n";
    
    // 설정 정보
    if (!sortAlgo.empty() || !scoreStrategy.empty()) {
        std::string info = "   Sort: " + apply(CYAN, sortAlgo) + 
                          "  |  Score: " + apply(CYAN, scoreStrategy);
        std::cout << apply(BRIGHT_RED, boxRow(info, W)) << "\n";
    }
    
    std::cout << apply(BRIGHT_RED, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_RED, boxMiddle(W)) << "\n";
    
    // 컬럼 헤더
    std::ostringstream header;
    header << "  " << std::left
           << std::setw(6)  << "순위"
           << std::setw(35) << "제목"
           << std::setw(15) << "채널"
           << std::setw(10) << "점수"
           << std::setw(12) << "조회수"
           << "변동";
    
    std::cout << apply(BRIGHT_RED, boxRow(apply(DIM, header.str()), W)) << "\n";
    std::cout << apply(BRIGHT_RED, boxMiddle(W)) << "\n";
}

/**
 * @brief 랭킹 테이블 행 출력
 */
inline void printRankingRow(const RankingRow& row, int totalWidth = 85) {
    using namespace Color;
    
    // 순위 변동 아이콘
    std::string changeIcon;
    const char* changeColor;
    if (row.rankChange > 0) {
        changeIcon = "▲" + std::to_string(row.rankChange);
        changeColor = BRIGHT_GREEN;
    } else if (row.rankChange < 0) {
        changeIcon = "▼" + std::to_string(-row.rankChange);
        changeColor = BRIGHT_RED;
    } else {
        changeIcon = "─";
        changeColor = DIM;
    }
    
    // 순위별 색상
    const char* rankColor;
    std::string rankIcon;
    if (row.rank == 1) {
        rankColor = BRIGHT_YELLOW;
        rankIcon = "🥇";
    } else if (row.rank == 2) {
        rankColor = WHITE;
        rankIcon = "🥈";
    } else if (row.rank == 3) {
        rankColor = YELLOW;
        rankIcon = "🥉";
    } else {
        rankColor = WHITE;
        rankIcon = "  ";
    }
    
    // 제목 자르기 (너무 길면)
    std::string title = row.title;
    if (title.length() > 32) {
        title = title.substr(0, 29) + "...";
    }
    
    // 채널명 자르기
    std::string channel = row.channel;
    if (channel.length() > 12) {
        channel = channel.substr(0, 9) + "...";
    }
    
    // 조회수 포맷팅
    std::string viewsStr;
    if (row.views >= 1000000) {
        viewsStr = std::to_string(row.views / 1000000) + "M";
    } else if (row.views >= 1000) {
        viewsStr = std::to_string(row.views / 1000) + "K";
    } else {
        viewsStr = std::to_string(row.views);
    }
    
    std::ostringstream line;
    line << "  "
         << std::left << std::setw(3) << ("#" + std::to_string(row.rank))
         << rankIcon
         << std::setw(33) << title
         << std::setw(13) << channel
         << std::right << std::setw(8) << row.score
         << std::setw(10) << viewsStr
         << "  " << apply(changeColor, changeIcon);
    
    std::cout << apply(BRIGHT_RED, boxRow(line.str(), totalWidth)) << "\n";
}

/**
 * @brief 랭킹 테이블 하단 출력
 */
inline void printRankingFooter(int showing, int total, int totalWidth = 85) {
    using namespace Color;
    
    std::cout << apply(BRIGHT_RED, boxMiddle(totalWidth)) << "\n";
    
    std::ostringstream footer;
    footer << "  Showing " << apply(BRIGHT_WHITE, std::to_string(showing))
           << " of " << apply(BRIGHT_WHITE, std::to_string(total)) << " videos";
    
    std::cout << apply(BRIGHT_RED, boxRow(footer.str(), totalWidth)) << "\n";
    std::cout << apply(BRIGHT_RED, boxBottom(totalWidth)) << "\n";
}

/**
 * @brief 전체 랭킹 테이블 출력 (통합 함수)
 */
inline void printRankingTable(
    const std::vector<RankingRow>& rows,
    const std::string& sortAlgo = "",
    const std::string& scoreStrategy = "",
    int total = -1
) {
    printRankingHeader(sortAlgo, scoreStrategy);
    
    for (const auto& row : rows) {
        printRankingRow(row);
    }
    
    int showing = static_cast<int>(rows.size());
    if (total < 0) total = showing;
    printRankingFooter(showing, total);
}

// ============================================================================
// ASCII 버전 랭킹 테이블 (호환성)
// ============================================================================

inline void printRankingTableAscii(const std::vector<RankingRow>& rows) {
    std::cout << "\n";
    std::cout << "+-----+-----------------------------------+---------------+--------+----------+-----+\n";
    std::cout << "| Rank|            Title                  |    Channel    | Score  |  Views   |Chng |\n";
    std::cout << "+-----+-----------------------------------+---------------+--------+----------+-----+\n";
    
    for (const auto& row : rows) {
        std::string title = row.title.length() > 33 ? row.title.substr(0, 30) + "..." : row.title;
        std::string channel = row.channel.length() > 13 ? row.channel.substr(0, 10) + "..." : row.channel;
        
        std::string change;
        if (row.rankChange > 0) change = "+" + std::to_string(row.rankChange);
        else if (row.rankChange < 0) change = std::to_string(row.rankChange);
        else change = "-";
        
        std::cout << "| " << std::setw(3) << row.rank << " | "
                  << std::left << std::setw(33) << title << " | "
                  << std::setw(13) << channel << " | "
                  << std::right << std::setw(6) << row.score << " | "
                  << std::setw(8) << row.views << " | "
                  << std::setw(3) << change << " |\n";
    }
    
    std::cout << "+-----+-----------------------------------+---------------+--------+----------+-----+\n";
}

// ============================================================================
// 영상 상세 정보
// ============================================================================

/**
 * @struct VideoDetail
 * @brief 영상 상세 정보
 */
struct VideoDetail {
    int rank;
    std::string videoId;
    std::string title;
    std::string channel;
    std::string uploadDate;
    int64_t views;
    int64_t likes;
    int64_t comments;
    int duration;  // 초
    double score;
};

/**
 * @brief 영상 상세 정보 패널 출력
 */
inline void printVideoDetail(const VideoDetail& video) {
    using namespace Color;
    
    const int W = 65;
    
    std::cout << "\n";
    std::cout << apply(BRIGHT_BLUE, boxTop(W)) << "\n";
    std::cout << apply(BRIGHT_BLUE, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_BLUE, boxRow(
        "   " + std::string(apply(BRIGHT_WHITE, "영상 상세 정보  |  Video Details")), W)) << "\n";
    std::cout << apply(BRIGHT_BLUE, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_BLUE, boxMiddle(W)) << "\n";
    
    // 순위
    std::string rankLine = "  순위:    " + apply(BRIGHT_YELLOW, "#" + std::to_string(video.rank));
    std::cout << apply(BRIGHT_BLUE, boxRow(rankLine, W)) << "\n";
    
    // 제목
    std::string titleLine = "  제목:    " + apply(BRIGHT_WHITE, video.title);
    std::cout << apply(BRIGHT_BLUE, boxRow(titleLine, W)) << "\n";
    
    // 채널
    std::string channelLine = "  채널:    " + apply(CYAN, video.channel);
    std::cout << apply(BRIGHT_BLUE, boxRow(channelLine, W)) << "\n";
    
    std::cout << apply(BRIGHT_BLUE, boxMiddle(W)) << "\n";
    
    // 통계
    std::cout << apply(BRIGHT_BLUE, boxRow(apply(DIM, "  ── 통계 ──────────────────────────────────────"), W)) << "\n";
    
    std::ostringstream stats;
    stats << "  점수:    " << apply(BRIGHT_GREEN, std::to_string(static_cast<int>(video.score)));
    std::cout << apply(BRIGHT_BLUE, boxRow(stats.str(), W)) << "\n";
    
    std::ostringstream views;
    views << "  조회수:  " << apply(WHITE, std::to_string(video.views));
    std::cout << apply(BRIGHT_BLUE, boxRow(views.str(), W)) << "\n";
    
    std::ostringstream likes;
    likes << "  좋아요:  " << apply(BRIGHT_RED, std::to_string(video.likes));
    std::cout << apply(BRIGHT_BLUE, boxRow(likes.str(), W)) << "\n";
    
    std::ostringstream comments;
    comments << "  댓글:    " << apply(YELLOW, std::to_string(video.comments));
    std::cout << apply(BRIGHT_BLUE, boxRow(comments.str(), W)) << "\n";
    
    // 영상 길이
    int hours = video.duration / 3600;
    int mins = (video.duration % 3600) / 60;
    int secs = video.duration % 60;
    std::ostringstream duration;
    duration << "  길이:    ";
    if (hours > 0) duration << hours << ":";
    duration << std::setfill('0') << std::setw(2) << mins << ":" 
             << std::setw(2) << secs;
    std::cout << apply(BRIGHT_BLUE, boxRow(duration.str(), W)) << "\n";
    
    std::cout << apply(BRIGHT_BLUE, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_BLUE, boxBottom(W)) << "\n";
}

// ============================================================================
// 벤치마크 결과
// ============================================================================

/**
 * @struct BenchmarkResult
 * @brief 벤치마크 결과
 */
struct BenchmarkResult {
    std::string algorithm;
    int64_t timeUs;  // 마이크로초
    int dataSize;
};

/**
 * @brief 벤치마크 결과 테이블 출력
 */
inline void printBenchmarkResults(const std::vector<BenchmarkResult>& results) {
    using namespace Color;
    
    const int W = 60;
    
    std::cout << "\n";
    std::cout << apply(BRIGHT_MAGENTA, boxTop(W)) << "\n";
    std::cout << apply(BRIGHT_MAGENTA, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_MAGENTA, boxRow(
        "   " + std::string(apply(BRIGHT_WHITE, "벤치마크 결과  |  Benchmark Results")), W)) << "\n";
    
    if (!results.empty()) {
        std::cout << apply(BRIGHT_MAGENTA, boxRow(
            "   데이터 크기: " + apply(CYAN, std::to_string(results[0].dataSize)) + " 개", W)) << "\n";
    }
    
    std::cout << apply(BRIGHT_MAGENTA, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_MAGENTA, boxMiddle(W)) << "\n";
    
    // 헤더
    std::ostringstream header;
    header << "  " << std::left << std::setw(25) << "알고리즘" 
           << std::right << std::setw(15) << "소요 시간"
           << std::setw(12) << "상대 속도";
    std::cout << apply(BRIGHT_MAGENTA, boxRow(apply(DIM, header.str()), W)) << "\n";
    std::cout << apply(BRIGHT_MAGENTA, boxMiddle(W)) << "\n";
    
    // 최소 시간 찾기
    int64_t minTime = INT64_MAX;
    for (const auto& r : results) {
        if (r.timeUs < minTime) minTime = r.timeUs;
    }
    
    // 결과 출력
    for (const auto& r : results) {
        double ratio = minTime > 0 ? static_cast<double>(r.timeUs) / minTime : 1.0;
        
        // 속도 등급
        const char* speedColor;
        std::string speedBar;
        if (ratio < 1.5) {
            speedColor = BRIGHT_GREEN;
            speedBar = "████";
        } else if (ratio < 3.0) {
            speedColor = YELLOW;
            speedBar = "███░";
        } else if (ratio < 10.0) {
            speedColor = BRIGHT_YELLOW;
            speedBar = "██░░";
        } else {
            speedColor = RED;
            speedBar = "█░░░";
        }
        
        std::ostringstream row;
        row << "  " << std::left << std::setw(25) << r.algorithm
            << std::right << std::setw(10) << r.timeUs << " μs"
            << "  " << apply(speedColor, speedBar);
        
        std::cout << apply(BRIGHT_MAGENTA, boxRow(row.str(), W)) << "\n";
    }
    
    std::cout << apply(BRIGHT_MAGENTA, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_MAGENTA, boxBottom(W)) << "\n";
}

} // namespace UI

