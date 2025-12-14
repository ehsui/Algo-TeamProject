#pragma once
/**
 * @file RankingUI.h
 * @brief Ranking Table and Detail Info UI (Simple format for Korean support)
 */

#include "ConsoleUI.h"
#include <vector>
#include <iomanip>

namespace UI {

// ============================================================================
// Ranking Table
// ============================================================================

struct RankingRow {
    int rank;
    std::string title;
    std::string channel;
    int64_t score;
    int64_t views;
    int64_t likes;
    int rankChange;  // +: up, -: down, 0: same
};

inline void printRankingHeader(const std::string& sortAlgo = "", const std::string& scoreStrategy = "") {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "                            [>] Live Rankings\n";
    
    if (!sortAlgo.empty() || !scoreStrategy.empty()) {
        std::cout << "    Sort: " << sortAlgo << "  |  Score: " << scoreStrategy << "\n";
    }
    
    std::cout << "================================================================================\n";
    std::cout << "\n";
    std::cout << "  Rank   Score    Title\n";
    std::cout << "  ----   ------   ----------------------------------------------------------\n";
}

inline void printRankingRow(const RankingRow& row, int totalWidth = 80) {
    // Simple format: Rank   Score   Title
    std::cout << "  #" << std::left << std::setw(4) << row.rank
              << "  " << std::right << std::setw(6) << row.score
              << "   " << row.title << "\n";
}

inline void printRankingFooter(int showing, int total, int totalWidth = 80) {
    std::cout << "\n";
    std::cout << "  ------------------------------------------------------------------------\n";
    std::cout << "  Showing " << showing << " of " << total << " videos\n";
    std::cout << "================================================================================\n";
}

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

inline void printRankingTableAscii(const std::vector<RankingRow>& rows) {
    printRankingTable(rows);
}

// ============================================================================
// Video Details
// ============================================================================

struct VideoDetail {
    int rank;
    std::string videoId;
    std::string title;
    std::string channel;
    std::string uploadDate;
    int64_t views;
    int64_t likes;
    int64_t comments;
    int duration;
    double score;
};

inline void printVideoDetail(const VideoDetail& video) {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "                            Video Details\n";
    std::cout << "================================================================================\n";
    std::cout << "\n";
    std::cout << "  Rank:      #" << video.rank << "\n";
    std::cout << "  Title:     " << video.title << "\n";
    std::cout << "  Channel:   " << video.channel << "\n";
    std::cout << "\n";
    std::cout << "  -- Statistics --\n";
    std::cout << "  Score:     " << static_cast<int>(video.score) << "\n";
    std::cout << "  Views:     " << video.views << "\n";
    std::cout << "  Likes:     " << video.likes << "\n";
    std::cout << "  Comments:  " << video.comments << "\n";
    
    int hours = video.duration / 3600;
    int mins = (video.duration % 3600) / 60;
    int secs = video.duration % 60;
    std::cout << "  Duration:  ";
    if (hours > 0) std::cout << hours << ":";
    std::cout << std::setfill('0') << std::setw(2) << mins << ":" 
              << std::setw(2) << secs << std::setfill(' ') << "\n";
    std::cout << "\n";
    std::cout << "================================================================================\n";
}

// ============================================================================
// Benchmark Results
// ============================================================================

struct BenchmarkResult {
    std::string algorithm;
    int64_t timeUs;
    int dataSize;
};

inline void printBenchmarkResults(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "                           Benchmark Results\n";
    
    if (!results.empty()) {
        std::cout << "    Data Size: " << results[0].dataSize << " items\n";
    }
    
    std::cout << "================================================================================\n";
    std::cout << "\n";
    std::cout << "  Algorithm                    Time           Speed\n";
    std::cout << "  -------------------------    ----------     ------\n";
    
    int64_t minTime = INT64_MAX;
    for (const auto& r : results) {
        if (r.timeUs < minTime) minTime = r.timeUs;
    }
    
    for (const auto& r : results) {
        double ratio = minTime > 0 ? static_cast<double>(r.timeUs) / minTime : 1.0;
        
        std::string speedBar;
        if (ratio < 1.5) {
            speedBar = "[****]";
        } else if (ratio < 3.0) {
            speedBar = "[***-]";
        } else if (ratio < 10.0) {
            speedBar = "[**--]";
        } else {
            speedBar = "[*---]";
        }
        
        std::cout << "  " << std::left << std::setw(25) << r.algorithm
                  << "    " << std::right << std::setw(8) << r.timeUs << " us"
                  << "     " << speedBar << "\n";
    }
    
    std::cout << "\n";
    std::cout << "================================================================================\n";
}

} // namespace UI
