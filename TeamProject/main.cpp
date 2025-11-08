/*#include "Utility.h"

int main(void) {

}*/

#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp> // nlohmann/json
#include <windows.h>

using json = nlohmann::json;

// CURL callback to capture response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append((char*)contents, totalSize);
    return totalSize;
}

int main() {

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    
    // 1?? config.json 읽기
    std::ifstream configFile("config.json");
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        return 1;
    }

    json configJson;
    configFile >> configJson;
    std::string apiKey = configJson["YOUTUBE_API_KEY"];

    // 2?? CURL 초기화
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl_easy_init() failed" << std::endl;
        return 1;
    }

    std::string readBuffer;
    std::string url = "https://www.googleapis.com/youtube/v3/videos"
                      "?part=snippet,statistics,contentDetails"
                      "&chart=mostPopular"
                      "&regionCode=KR"
                      "&maxResults=5"
                      "&key=" + apiKey;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return 1;
    }
    curl_easy_cleanup(curl);

    // 3?? JSON 파싱
    try {
        auto responseJson = json::parse(readBuffer);
        auto items = responseJson["items"];

        for (auto& video : items) {
            std::string title = video["snippet"]["title"];
            std::string videoId = video["id"];
            std::string categoryId = video["snippet"]["categoryId"];

            uint64_t viewCount = std::stoull((std::string)video["statistics"]["viewCount"].get<std::string>());
            uint64_t likeCount = 0;
            uint64_t commentCount = 0;

            // 좋아요, 댓글은 일부 영상에서 제한될 수 있음
            if (video["statistics"].contains("likeCount"))
                likeCount = std::stoull((std::string)video["statistics"]["likeCount"].get<std::string>());
            if (video["statistics"].contains("commentCount"))
                commentCount = std::stoull((std::string)video["statistics"]["commentCount"].get<std::string>());

            std::string duration = video["contentDetails"]["duration"]; // ISO 8601 형식, 예: PT5M33S

            std::cout << "Title: " << title << "\n"
                      << "Video ID: " << videoId << "\n"
                      << "Category ID: " << categoryId << "\n"
                      << "Views: " << viewCount << "\n"
                      << "Likes: " << likeCount << "\n"
                      << "Comments: " << commentCount << "\n"
                      << "Duration: " << duration << "\n"
                      << "-----------------------------\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
