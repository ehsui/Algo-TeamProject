#pragma once
/**
 * @file Video.h
 * @brief YouTube Video Data Structure
 * 
 * [Unified Data Model]
 * - Primary data structure for RankEngine
 * - Loadable from CSV files
 * - Convertible to/from Domain::VideoMetrics
 */

#include "Utility.h"
#include "Score.h"
#include "Domain/VideoMetrics.h"
#include <fstream>
#include <sstream>

// Forward declaration
struct key;

/**
 * @struct YouTubeVideoInfo
 * @brief Structure containing YouTube video information
 * 
 * [Field Categories]
 * - Basic info: ID, title, channel info
 * - Statistics: views, likes, comments
 * - Details: duration, quality, captions
 * - Status: privacy, embed settings
 * - Meta: fetch timestamp
 */
typedef struct YouTubeVideoInfo {
    // === Basic Info ===
    string videoId;            // Video unique ID (e.g., "dQw4w9WgXcQ")
    string channelId;          // Channel ID
    string channelTitle;       // Channel name
    string title;              // Video title
    string description;        // Video description
    string categoryId;         // Category ID
    vector<string> tags;       // Tag list
    string publishedAt;        // Upload date (ISO 8601)
    
    // === Calculated Score ===
    Score score = 0;

    // === Thumbnail ===
    string thumbnailUrl;

    // === Statistics ===
    long long viewCount = 0;
    long long likeCount = 0;
    long long commentCount = 0;

    // === Details ===
    int durationSeconds = 0;   // Duration in seconds
    string definition;         // "hd" or "sd"
    string dimension;          // "2d" or "3d"
    bool caption = false;      // Has captions
    bool licensedContent = false;
    vector<string> regionBlocked;  // Blocked country codes

    // === Status ===
    string privacyStatus;      // "public", "private", "unlisted"
    bool embeddable = true;

    // === Metadata ===
    string fetchTimestamp;     // Data fetch timestamp

    // === Methods ===
    
    /**
     * @brief Update with another Video's info
     */
    void updateInfo(const YouTubeVideoInfo& v);
    
    /**
     * @brief Create lightweight key for sorting
     */
    key makekey() const;
    
    /**
     * @brief Calculate and assign score
     * @param strategy Scoring strategy to use
     */
    void calculateScore(ScoringStrategy strategy = ScoringStrategy::Engagement);
    
    // === VideoMetrics Conversion ===
    
    /**
     * @brief Convert to VideoMetrics
     */
    Domain::VideoMetrics toVideoMetrics() const;
    
    /**
     * @brief Create Video from VideoMetrics
     */
    static YouTubeVideoInfo fromVideoMetrics(const Domain::VideoMetrics& vm);
    
    // === CSV Loading ===
    
    /**
     * @brief Create Video from CSV row
     */
    static YouTubeVideoInfo fromCsvRow(const string& videoId, const string& timestamp,
                            long long views, long long likes, long long comments,
                            const string& title);
    
} Video;

/**
 * @struct key
 * @brief Lightweight key structure for sorting
 * 
 * Sorting full Video objects is expensive due to memory moves,
 * so we use this lightweight key for sorting operations.
 */
struct key {
    Score Value = 0;       // Score
    string videoId;        // Video ID (for reference)
    string title;          // Title (for tie-breaking)
    
    // Comparison operators (descending: higher score first)
    bool operator<(const key& other) const {
        if (Value != other.Value) {
            return Value > other.Value;  // Higher score comes first
        }
        return title < other.title;      // Alphabetical on tie
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
// CSV Data Loader
// ============================================================================

/**
 * @class CsvDataLoader
 * @brief Utility class to load Video data from CSV files
 * 
 * [CSV Format]
 * VIDEO_ID,TITLE,VIEWS,LIKES,COMMENTS,TIMESTAMP
 * 
 * [Usage Example]
 * auto [initial, refresh] = CsvDataLoader::loadAndSplit("data.csv");
 * engine.build(initial);
 * engine.refresh(refresh);
 */
class CsvDataLoader {
public:
    /**
     * @brief Load all Videos from CSV file
     * @param filePath CSV file path
     * @param strategy Scoring strategy to use
     * @return Vector of Videos
     */
    static vector<Video> loadAll(const string& filePath, 
                                  ScoringStrategy strategy = ScoringStrategy::Engagement);
    
    /**
     * @brief Load Videos from specific timestamp
     * @param filePath CSV file path
     * @param timestamp Timestamp to filter (e.g., "2025-11-26")
     * @param strategy Scoring strategy to use
     * @return Vector of Videos from that timestamp
     */
    static vector<Video> loadByTimestamp(const string& filePath, const string& timestamp,
                                          ScoringStrategy strategy = ScoringStrategy::Engagement);
    
    /**
     * @brief Load CSV and split into two timestamps
     * @param filePath CSV file path
     * @param strategy Scoring strategy to use
     * @return pair<initial data, refresh data>
     */
    static pair<vector<Video>, vector<Video>> loadAndSplit(const string& filePath,
                                                            ScoringStrategy strategy = ScoringStrategy::Engagement);
    
    /**
     * @brief Get unique timestamps from CSV file
     * @param filePath CSV file path
     * @return Sorted vector of timestamp strings
     */
    static vector<string> getTimestamps(const string& filePath);
    
private:
    /**
     * @brief Parse CSV line (comma-separated, handles quotes)
     */
    static vector<string> parseCsvLine(const string& line);
};
