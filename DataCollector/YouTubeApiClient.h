#pragma once
/**
 * @file YouTubeApiClient.h
 * @brief YouTube Data API v3 Client
 * 
 * [Specification]
 * - Endpoint: videos.list (mostPopular)
 * - Region: KR (Korea)
 * - Collection: ~100 videos (2 pages)
 * - Retry: up to 3 times with exponential backoff
 * 
 * [Note]
 * - API key is loaded at runtime (never hardcoded)
 * - All videos share the same timestamp
 */

#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace Api {
namespace YouTube {

// ============================================================================
// Data Structures
// ============================================================================

/**
 * @struct VideoSnapshot
 * @brief Video snapshot data at one point in time
 */
struct VideoSnapshot {
    std::string videoId;        // Video ID
    std::string title;          // Title
    int64_t viewCount = 0;      // View count
    int64_t likeCount = 0;      // Like count
    int64_t commentCount = 0;   // Comment count
    std::string timestamp;      // Collection time (UTC "YYYY-MM-DD HH:MM:SS")
};

/**
 * @struct FetchResponse
 * @brief API call result
 */
struct FetchResponse {
    std::vector<VideoSnapshot> videos;
    std::string nextPageToken;
    bool success = false;
    std::string errorMessage;
    int httpStatusCode = 0;
};

// ============================================================================
// YouTube API Client
// ============================================================================

/**
 * @class YouTubeApiClient
 * @brief YouTube Data API v3 Client
 */
class YouTubeApiClient {
public:
    /**
     * @brief Constructor
     * @param apiKey YouTube API key (loaded at runtime)
     */
    explicit YouTubeApiClient(const std::string& apiKey)
        : apiKey_(apiKey), maxRetries_(3), baseDelayMs_(1000) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~YouTubeApiClient() {
        curl_global_cleanup();
    }

    /**
     * @brief Fetch ~100 popular videos (2 pages)
     * @return List of video snapshots
     */
    std::vector<VideoSnapshot> fetchMostPopular() {
        std::vector<VideoSnapshot> allVideos;
        std::string timestamp = getCurrentUtcTimestamp();

        log("=== YouTube API Data Collection Started ===");
        log("Timestamp: " + timestamp);

        // Page 1 request
        FetchResponse page1 = fetchPage("", timestamp);
        if (!page1.success) {
            log("ERROR: Page 1 request failed - " + page1.errorMessage);
            return allVideos;
        }
        log("Page 1: " + std::to_string(page1.videos.size()) + " videos collected");
        allVideos.insert(allVideos.end(), page1.videos.begin(), page1.videos.end());

        // Page 2 request (if nextPageToken exists)
        if (!page1.nextPageToken.empty()) {
            FetchResponse page2 = fetchPage(page1.nextPageToken, timestamp);
            if (page2.success) {
                log("Page 2: " + std::to_string(page2.videos.size()) + " videos collected");
                allVideos.insert(allVideos.end(), page2.videos.begin(), page2.videos.end());
            } else {
                log("WARNING: Page 2 request failed - " + page2.errorMessage);
                // Fallback: try different region code
                log("Fallback: Trying US region");
                FetchResponse fallback = fetchPage("", timestamp, "US");
                if (fallback.success) {
                    log("Fallback: " + std::to_string(fallback.videos.size()) + " videos collected");
                    allVideos.insert(allVideos.end(), fallback.videos.begin(), fallback.videos.end());
                }
            }
        } else {
            // Fallback: no nextPageToken, try different region
            log("Fallback: No nextPageToken, trying US region");
            FetchResponse fallback = fetchPage("", timestamp, "US");
            if (fallback.success) {
                log("Fallback: " + std::to_string(fallback.videos.size()) + " videos collected");
                allVideos.insert(allVideos.end(), fallback.videos.begin(), fallback.videos.end());
            }
        }

        log("=== Total " + std::to_string(allVideos.size()) + " videos collected ===");
        return allVideos;
    }

private:
    std::string apiKey_;
    int maxRetries_;
    int baseDelayMs_;

    // ========================================================================
    // HTTP Request
    // ========================================================================

    /**
     * @brief Single page request
     */
    FetchResponse fetchPage(const std::string& pageToken, 
                            const std::string& timestamp,
                            const std::string& regionCode = "KR") {
        FetchResponse response;
        
        // Build URL
        std::string url = buildUrl(pageToken, regionCode);
        
        // Retry logic
        for (int attempt = 1; attempt <= maxRetries_; ++attempt) {
            log("Request attempt " + std::to_string(attempt) + "/" + std::to_string(maxRetries_));
            
            std::string responseBody;
            int httpCode = performRequest(url, responseBody);
            response.httpStatusCode = httpCode;

            log("HTTP status code: " + std::to_string(httpCode));

            if (httpCode == 200) {
                // Success - parse JSON
                if (parseResponse(responseBody, timestamp, response)) {
                    response.success = true;
                    return response;
                } else {
                    response.errorMessage = "JSON parsing failed";
                    return response;
                }
            } else if (httpCode == 403) {
                // Quota exceeded or API key invalid
                log("HTTP 403 - Checking error details...");
                parseErrorResponse(responseBody, response);
                
                if (response.errorMessage.find("quotaExceeded") != std::string::npos) {
                    log("*** QUOTA EXCEEDED ***");
                    log("Your daily YouTube API quota has been exhausted.");
                    log("The quota resets at midnight Pacific Time (PT).");
                    log("Check: https://console.cloud.google.com/apis/dashboard");
                } else if (response.errorMessage.find("keyInvalid") != std::string::npos) {
                    log("*** INVALID API KEY ***");
                    log("Check your API key in config.txt");
                } else if (response.errorMessage.find("accessNotConfigured") != std::string::npos) {
                    log("*** API NOT ENABLED ***");
                    log("Enable 'YouTube Data API v3' in Google Cloud Console");
                } else if (response.errorMessage.find("forbidden") != std::string::npos) {
                    log("*** ACCESS FORBIDDEN ***");
                    log("Check API key restrictions in Google Cloud Console");
                }
                return response;
            } else if (httpCode == 400) {
                // Bad request - likely API key format issue
                log("HTTP 400 - Bad Request");
                parseErrorResponse(responseBody, response);
                if (response.errorMessage.find("keyInvalid") != std::string::npos) {
                    log("*** INVALID API KEY FORMAT ***");
                    log("API key may be malformed or contain invalid characters");
                }
                return response;
            } else if (httpCode == 429 || httpCode >= 500) {
                // Retryable error
                if (attempt < maxRetries_) {
                    int delayMs = baseDelayMs_ * (1 << (attempt - 1));  // Exponential backoff
                    log("Waiting... " + std::to_string(delayMs) + "ms");
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
                }
            } else if (httpCode == 0) {
                // Network error
                log("*** NETWORK ERROR ***");
                log("Check your internet connection");
                response.errorMessage = "Network error (no response)";
                return response;
            } else {
                // Non-retryable error
                response.errorMessage = "HTTP error: " + std::to_string(httpCode);
                parseErrorResponse(responseBody, response);
                return response;
            }
        }

        response.errorMessage = "Max retry count exceeded";
        return response;
    }

    /**
     * @brief Build API URL
     */
    std::string buildUrl(const std::string& pageToken, const std::string& regionCode) {
        std::ostringstream url;
        url << "https://www.googleapis.com/youtube/v3/videos?"
            << "part=snippet,statistics"
            << "&chart=mostPopular"
            << "&maxResults=50"
            << "&regionCode=" << regionCode
            << "&fields=items(id,snippet(title),statistics(viewCount,likeCount,commentCount)),nextPageToken"
            << "&key=" << apiKey_;
        
        if (!pageToken.empty()) {
            url << "&pageToken=" << pageToken;
        }
        
        return url.str();
    }

    /**
     * @brief curl callback function
     */
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    }

    /**
     * @brief Perform HTTP GET request
     * @return HTTP status code (0 = network error)
     */
    int performRequest(const std::string& url, std::string& responseBody) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            log("ERROR: curl_easy_init() failed");
            return 0;
        }

        // Debug: print URL without API key
        std::string debugUrl = url.substr(0, url.find("&key="));
        log("Request URL: " + debugUrl + "&key=***");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        
        long httpCode = 0;
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            log("Response size: " + std::to_string(responseBody.size()) + " bytes");
        } else {
            log("curl error: " + std::string(curl_easy_strerror(res)));
            log("curl error code: " + std::to_string(static_cast<int>(res)));
        }

        curl_easy_cleanup(curl);
        return static_cast<int>(httpCode);
    }

    // ========================================================================
    // JSON Parsing
    // ========================================================================

    /**
     * @brief Parse error response for detailed error info
     */
    void parseErrorResponse(const std::string& jsonStr, FetchResponse& response) {
        try {
            auto json = nlohmann::json::parse(jsonStr);
            
            if (json.contains("error")) {
                std::string errMsg = "";
                
                if (json["error"].contains("code")) {
                    errMsg += "Code " + std::to_string(json["error"]["code"].get<int>()) + ": ";
                }
                if (json["error"].contains("message")) {
                    errMsg += json["error"]["message"].get<std::string>();
                }
                
                // Check for specific error reasons
                if (json["error"].contains("errors") && json["error"]["errors"].is_array()) {
                    for (const auto& err : json["error"]["errors"]) {
                        if (err.contains("reason")) {
                            std::string reason = err["reason"].get<std::string>();
                            errMsg += " [" + reason + "]";
                            log("Error reason: " + reason);
                        }
                        if (err.contains("domain")) {
                            log("Error domain: " + err["domain"].get<std::string>());
                        }
                    }
                }
                
                response.errorMessage = errMsg;
                log("API Error Detail: " + errMsg);
            }
        } catch (const std::exception& e) {
            response.errorMessage = "Error parsing failed: " + std::string(e.what());
        }
    }

    /**
     * @brief Parse JSON response
     */
    bool parseResponse(const std::string& jsonStr, 
                       const std::string& timestamp,
                       FetchResponse& response) {
        try {
            // Debug: show first 500 chars of response
            if (jsonStr.size() < 500) {
                log("Response: " + jsonStr);
            } else {
                log("Response (first 500 chars): " + jsonStr.substr(0, 500) + "...");
            }

            auto json = nlohmann::json::parse(jsonStr);

            // Check for error response
            if (json.contains("error")) {
                std::string errMsg = "API Error";
                if (json["error"].contains("message")) {
                    errMsg = json["error"]["message"].get<std::string>();
                }
                if (json["error"].contains("code")) {
                    errMsg = "Code " + std::to_string(json["error"]["code"].get<int>()) + ": " + errMsg;
                }
                response.errorMessage = errMsg;
                log("API Error: " + errMsg);
                return false;
            }

            // nextPageToken
            if (json.contains("nextPageToken")) {
                response.nextPageToken = json["nextPageToken"].get<std::string>();
            }

            // Parse items
            if (!json.contains("items") || !json["items"].is_array()) {
                return true;  // Empty response is valid
            }

            for (const auto& item : json["items"]) {
                VideoSnapshot video;
                video.timestamp = timestamp;

                // videoId
                if (item.contains("id")) {
                    video.videoId = item["id"].get<std::string>();
                }

                // title
                if (item.contains("snippet") && item["snippet"].contains("title")) {
                    video.title = item["snippet"]["title"].get<std::string>();
                }

                // statistics
                if (item.contains("statistics")) {
                    const auto& stats = item["statistics"];
                    video.viewCount = safeGetInt64(stats, "viewCount");
                    video.likeCount = safeGetInt64(stats, "likeCount");
                    video.commentCount = safeGetInt64(stats, "commentCount");
                }

                response.videos.push_back(std::move(video));
            }

            return true;
        } catch (const std::exception& e) {
            response.errorMessage = std::string("JSON parsing exception: ") + e.what();
            return false;
        }
    }

    /**
     * @brief Safely extract int64 from JSON
     */
    int64_t safeGetInt64(const nlohmann::json& obj, const std::string& key) {
        if (!obj.contains(key)) return 0;
        
        const auto& val = obj[key];
        if (val.is_string()) {
            try {
                return std::stoll(val.get<std::string>());
            } catch (...) {
                return 0;
            }
        } else if (val.is_number()) {
            return val.get<int64_t>();
        }
        return 0;
    }

    // ========================================================================
    // Utility
    // ========================================================================

    /**
     * @brief Generate current UTC timestamp
     * @return "YYYY-MM-DD HH:MM:SS" format
     */
    std::string getCurrentUtcTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        
        std::tm tm_utc;
#ifdef _WIN32
        gmtime_s(&tm_utc, &time_t_now);
#else
        gmtime_r(&time_t_now, &tm_utc);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm_utc, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    /**
     * @brief Log output (API key excluded)
     */
    void log(const std::string& message) {
        std::cout << "[YouTubeApi] " << message << std::endl;
    }
};

} // namespace YouTube
} // namespace Api
