/**
 * @file main.cpp
 * @brief YouTube Data Collector - Main Entry Point
 * 
 * [Features]
 * - Collect popular videos from YouTube API (~100 per run)
 * - Save snapshots to SQLite DB
 * - Export to CSV for RankEngine
 * - All videos in one execution share the same timestamp
 * 
 * [API Key Setup] (priority order)
 * 1. Command line: YouTubeDataCollector.exe API_KEY
 * 2. Config file: config.txt with API key
 * 3. Environment variable: YOUTUBE_API_KEY
 * 
 * [CSV Output Path]
 * Default: ../../../data/  (TeamProject/data/ folder)
 */

#include "YouTubeApiClient.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <filesystem>

// CSV output directory - using absolute path for reliability
const std::string CSV_OUTPUT_DIR = "C:/Users/chois/source/repos/TeamProject/data/";

// ============================================================================
// Database Helper
// ============================================================================

/**
 * @brief Create DB table
 */
bool createTable(sqlite3* db) {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS video_snapshots (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            video_id TEXT NOT NULL,
            title TEXT,
            view_count INTEGER DEFAULT 0,
            like_count INTEGER DEFAULT 0,
            comment_count INTEGER DEFAULT 0,
            timestamp TEXT NOT NULL,
            UNIQUE(video_id, timestamp)
        );
        CREATE INDEX IF NOT EXISTS idx_video_id ON video_snapshots(video_id);
        CREATE INDEX IF NOT EXISTS idx_timestamp ON video_snapshots(timestamp);
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Table creation failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    std::cout << "[DB] Table ready" << std::endl;
    return true;
}

/**
 * @brief Save video snapshots to DB
 */
int saveSnapshots(sqlite3* db, const std::vector<Api::YouTube::VideoSnapshot>& videos) {
    const char* sql = R"(
        INSERT OR REPLACE INTO video_snapshots 
        (video_id, title, view_count, like_count, comment_count, timestamp)
        VALUES (?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Query prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    int savedCount = 0;
    
    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    for (const auto& video : videos) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);

        sqlite3_bind_text(stmt, 1, video.videoId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, video.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, video.viewCount);
        sqlite3_bind_int64(stmt, 4, video.likeCount);
        sqlite3_bind_int64(stmt, 5, video.commentCount);
        sqlite3_bind_text(stmt, 6, video.timestamp.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            ++savedCount;
        } else {
            std::cerr << "[DB] Save failed: " << video.videoId 
                      << " - " << sqlite3_errmsg(db) << std::endl;
        }
    }

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    sqlite3_finalize(stmt);

    return savedCount;
}

/**
 * @brief Print collection statistics
 */
void printStats(sqlite3* db) {
    const char* sql = R"(
        SELECT 
            COUNT(DISTINCT video_id) as unique_videos,
            COUNT(DISTINCT timestamp) as snapshots,
            COUNT(*) as total_rows
        FROM video_snapshots
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::cout << "\n=== DB Statistics ===" << std::endl;
            std::cout << "Unique videos: " << sqlite3_column_int(stmt, 0) << std::endl;
            std::cout << "Snapshots: " << sqlite3_column_int(stmt, 1) << std::endl;
            std::cout << "Total records: " << sqlite3_column_int(stmt, 2) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

// ============================================================================
// CSV Export
// ============================================================================

/**
 * @brief Escape CSV field (handle commas and quotes)
 */
std::string escapeCsvField(const std::string& field) {
    bool needsQuotes = field.find(',') != std::string::npos ||
                       field.find('"') != std::string::npos ||
                       field.find('\n') != std::string::npos;
    
    if (!needsQuotes) {
        return field;
    }
    
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";  // Double quotes
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

/**
 * @brief Export all data from DB to CSV file
 * @param db SQLite database
 * @param filename Output CSV filename
 * @return Number of rows exported
 */
int exportToCsv(sqlite3* db, const std::string& filename) {
    const char* sql = R"(
        SELECT video_id, title, view_count, like_count, comment_count, timestamp
        FROM video_snapshots
        ORDER BY timestamp, view_count DESC
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[CSV] Query failed: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[CSV] Cannot create file: " << filename << std::endl;
        sqlite3_finalize(stmt);
        return 0;
    }

    // Write CSV header
    file << "video_id,title,view_count,like_count,comment_count,timestamp\n";

    int rowCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string videoId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int64_t viewCount = sqlite3_column_int64(stmt, 2);
        int64_t likeCount = sqlite3_column_int64(stmt, 3);
        int64_t commentCount = sqlite3_column_int64(stmt, 4);
        std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        file << escapeCsvField(videoId) << ","
             << escapeCsvField(title) << ","
             << viewCount << ","
             << likeCount << ","
             << commentCount << ","
             << escapeCsvField(timestamp) << "\n";

        ++rowCount;
    }

    sqlite3_finalize(stmt);
    file.close();

    std::cout << "[CSV] Exported " << rowCount << " rows to " << filename << std::endl;
    return rowCount;
}

/**
 * @brief Export latest snapshot only (most recent timestamp)
 */
int exportLatestToCsv(sqlite3* db, const std::string& filename) {
    const char* sql = R"(
        SELECT video_id, title, view_count, like_count, comment_count, timestamp
        FROM video_snapshots
        WHERE timestamp = (SELECT MAX(timestamp) FROM video_snapshots)
        ORDER BY view_count DESC
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[CSV] Query failed: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[CSV] Cannot create file: " << filename << std::endl;
        sqlite3_finalize(stmt);
        return 0;
    }

    file << "video_id,title,view_count,like_count,comment_count,timestamp\n";

    int rowCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string videoId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int64_t viewCount = sqlite3_column_int64(stmt, 2);
        int64_t likeCount = sqlite3_column_int64(stmt, 3);
        int64_t commentCount = sqlite3_column_int64(stmt, 4);
        std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        file << escapeCsvField(videoId) << ","
             << escapeCsvField(title) << ","
             << viewCount << ","
             << likeCount << ","
             << commentCount << ","
             << escapeCsvField(timestamp) << "\n";

        ++rowCount;
    }

    sqlite3_finalize(stmt);
    file.close();

    std::cout << "[CSV] Exported " << rowCount << " rows (latest snapshot) to " << filename << std::endl;
    return rowCount;
}

// ============================================================================
// API Key Load
// ============================================================================

std::string loadApiKeyFromConfig() {
    std::ifstream file("config.txt");
    if (!file.is_open()) {
        return "";
    }
    
    std::string key;
    std::getline(file, key);
    
    key.erase(0, key.find_first_not_of(" \t\r\n"));
    key.erase(key.find_last_not_of(" \t\r\n") + 1);
    
    return key;
}

/**
 * @brief Validate API key format
 * YouTube API keys are typically 39 characters, starting with "AIza"
 */
bool validateApiKeyFormat(const std::string& key) {
    if (key.empty()) return false;
    if (key.length() < 30 || key.length() > 50) {
        std::cerr << "[WARNING] API key length (" << key.length() 
                  << ") seems unusual (expected ~39 chars)" << std::endl;
    }
    if (key.find(' ') != std::string::npos || 
        key.find('\t') != std::string::npos ||
        key.find('\n') != std::string::npos) {
        std::cerr << "[ERROR] API key contains whitespace!" << std::endl;
        return false;
    }
    if (key[0] == '{' || key[0] == '"') {
        std::cerr << "[ERROR] API key looks like JSON! Check config.txt format." << std::endl;
        std::cerr << "        config.txt should contain ONLY the API key, nothing else." << std::endl;
        return false;
    }
    return true;
}

std::string loadApiKey(int argc, char* argv[]) {
    std::string key;
    std::string source;
    
    // Check command line (skip --collect flag)
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg != "--collect" && arg != "-c" && arg[0] != '-') {
            key = arg;
            source = "command line";
            break;
        }
    }
    
    // Check config.txt
    if (key.empty()) {
        key = loadApiKeyFromConfig();
        if (!key.empty()) {
            source = "config.txt";
        }
    }
    
    // Check environment variable
    if (key.empty()) {
        const char* envKey = std::getenv("YOUTUBE_API_KEY");
        if (envKey && envKey[0] != '\0') {
            key = envKey;
            source = "environment variable";
        }
    }
    
    if (key.empty()) {
        std::cerr << "\n[ERROR] No API key found!" << std::endl;
        std::cerr << "Please provide API key via one of:" << std::endl;
        std::cerr << "  1. config.txt file (just the key, nothing else)" << std::endl;
        std::cerr << "  2. Command line: YouTubeDataCollector.exe YOUR_API_KEY" << std::endl;
        std::cerr << "  3. Environment: set YOUTUBE_API_KEY=YOUR_API_KEY" << std::endl;
        return "";
    }
    
    std::cout << "[Config] API key loaded from: " << source << std::endl;
    std::cout << "[Config] API key: " << key.substr(0, 8) << "..." 
              << key.substr(key.length() - 4) << " (length: " << key.length() << ")" << std::endl;
    
    if (!validateApiKeyFormat(key)) {
        std::cerr << "[ERROR] API key format validation failed!" << std::endl;
        return "";
    }
    
    return key;
}

// ============================================================================
// Menu
// ============================================================================

/**
 * @brief Ensure output directory exists
 */
bool ensureOutputDir() {
    try {
        std::filesystem::create_directories(CSV_OUTPUT_DIR);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Cannot create output directory: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Get full path for CSV file
 */
std::string getCsvPath(const std::string& filename) {
    return CSV_OUTPUT_DIR + filename;
}

void printMenu() {
    std::cout << "\n=== YouTube Data Collector ===" << std::endl;
    std::cout << "1. Collect new data from API" << std::endl;
    std::cout << "2. Export all data to CSV" << std::endl;
    std::cout << "3. Export latest snapshot to CSV" << std::endl;
    std::cout << "4. Show DB statistics" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "CSV output: " << CSV_OUTPUT_DIR << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "Select: ";
}

// ============================================================================
// Main
// ============================================================================

/**
 * @brief Non-interactive collection mode (called from TeamProject)
 */
int runAutoCollect(const std::string& apiKey) {
    std::cout << "=== Auto Collect Mode ===" << std::endl;
    
    if (apiKey.empty()) {
        std::cerr << "ERROR: API key required." << std::endl;
        return 1;
    }
    
    // DB connection
    sqlite3* db = nullptr;
    int rc = sqlite3_open("youtube_data.db", &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Connection failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }
    
    if (!createTable(db)) {
        sqlite3_close(db);
        return 1;
    }
    
    // Collect from API
    std::cout << "Collecting data from YouTube API..." << std::endl;
    Api::YouTube::YouTubeApiClient client(apiKey);
    auto videos = client.fetchMostPopular();
    
    if (videos.empty()) {
        std::cerr << "WARNING: No videos collected" << std::endl;
        sqlite3_close(db);
        return 1;
    }
    
    int saved = saveSnapshots(db, videos);
    std::cout << "[DB] Saved " << saved << "/" << videos.size() << " videos" << std::endl;
    
    // Auto-export to CSV
    ensureOutputDir();
    exportLatestToCsv(db, getCsvPath("youtube_latest.csv"));
    exportToCsv(db, getCsvPath("youtube_all.csv"));
    
    sqlite3_close(db);
    std::cout << "=== Collection Complete ===" << std::endl;
    
    return 0;
}

int main(int argc, char* argv[]) {
    std::cout << "=== YouTube Data Collector ===" << std::endl;

    // Check for --collect flag (non-interactive mode)
    bool autoCollect = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--collect" || arg == "-c") {
            autoCollect = true;
        }
    }
    
    // Load API key
    std::string apiKey = loadApiKey(argc, argv);

    // Auto collect mode (called from TeamProject)
    if (autoCollect) {
        return runAutoCollect(apiKey);
    }

    // Interactive mode
    if (apiKey.empty()) {
        std::cout << "\nWARNING: No API key found." << std::endl;
        std::cout << "You can still export existing data.\n" << std::endl;
    } else {
        std::cout << "API key loaded (length: " << apiKey.length() << ")" << std::endl;
    }

    // DB connection
    sqlite3* db = nullptr;
    int rc = sqlite3_open("youtube_data.db", &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Connection failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }
    std::cout << "[DB] Connected" << std::endl;

    // Create table
    if (!createTable(db)) {
        sqlite3_close(db);
        return 1;
    }

    // Menu loop
    int choice;
    bool running = true;

    while (running) {
        printMenu();
        std::cin >> choice;

        switch (choice) {
            case 1: {
                // Collect from API
                if (apiKey.empty()) {
                    std::cerr << "ERROR: API key required for collection." << std::endl;
                    std::cerr << "Setup: config.txt, command line, or YOUTUBE_API_KEY env" << std::endl;
                    break;
                }

                std::cout << "\nCollecting data from YouTube API..." << std::endl;
                Api::YouTube::YouTubeApiClient client(apiKey);
                auto videos = client.fetchMostPopular();

                if (videos.empty()) {
                    std::cerr << "WARNING: No videos collected" << std::endl;
                } else {
                    int saved = saveSnapshots(db, videos);
                    std::cout << "[DB] Saved " << saved << "/" << videos.size() << " videos" << std::endl;
                    
                    // Auto-export to CSV
                    ensureOutputDir();
                    exportLatestToCsv(db, getCsvPath("youtube_latest.csv"));
                    exportToCsv(db, getCsvPath("youtube_all.csv"));
                }
                break;
            }

            case 2: {
                // Export all to CSV
                ensureOutputDir();
                exportToCsv(db, getCsvPath("youtube_all.csv"));
                break;
            }

            case 3: {
                // Export latest to CSV
                ensureOutputDir();
                exportLatestToCsv(db, getCsvPath("youtube_latest.csv"));
                break;
            }

            case 4: {
                // Show stats
                printStats(db);
                break;
            }

            case 0:
                running = false;
                std::cout << "Exiting..." << std::endl;
                break;

            default:
                std::cout << "Invalid selection." << std::endl;
                break;
        }
    }

    sqlite3_close(db);
    std::cout << "\n=== Done ===" << std::endl;
    
    return 0;
}
