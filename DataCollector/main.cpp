#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp> 
#include <windows.h>
#include <sqlite3.h>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <vector>

using json = nlohmann::json;

// --- 유틸리티 함수 ---

// CURL 콜백 (데이터 수신용)
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append((char*)contents, totalSize);
    return totalSize;
}

// 현재 시간 문자열 반환 (YYYY-MM-DD HH:MM:SS)
std::string getCurrentTimeISO() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S"); 
    return ss.str();
}

// API 호출을 수행하는 함수 (URL을 받아서 JSON 문자열 반환)
std::string fetchAPI(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL 요청 실패: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// --- 메인 로직 ---

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "--- 유튜브 Top 100 데이터 수집 시작 ---" << std::endl;

    // 1. Config 읽기
    std::ifstream configFile("config.json");
    if (!configFile.is_open()) {
        std::cerr << "config.json 파일을 찾을 수 없습니다." << std::endl;
        return 1;
    }
    json configJson;
    configFile >> configJson;
    std::string apiKey = configJson["YOUTUBE_API_KEY"];

    // 2. DB 연결 및 테이블 생성
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("youtube_data.db", &db);
    if (rc) {
        std::cerr << "DB 연결 실패: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // 테이블 생성 (이미 있으면 건너뜀)
    const char *sql_create = 
        "CREATE TABLE IF NOT EXISTS VIDEO_STATS("
        "VIDEO_ID TEXT NOT NULL,"
        "TIMESTAMP DATETIME NOT NULL,"
        "VIEWS INTEGER,"
        "LIKES INTEGER,"
        "COMMENTS INTEGER,"
        "TITLE TEXT,"
        "PRIMARY KEY (VIDEO_ID, TIMESTAMP));";

    rc = sqlite3_exec(db, sql_create, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "테이블 생성 오류: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return 1;
    }

    // 3. 반복 API 호출 (페이지네이션)
    std::string nextPageToken = "";
    std::string current_time = getCurrentTimeISO();
    int total_collected = 0;
    int max_videos_to_collect = 100; // 목표: 100개 수집

    // 성능을 위해 트랜잭션 시작 (대량 insert 속도 향상)
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    while (total_collected < max_videos_to_collect) {
        std::cout << ">> API 호출 중... (현재 " << total_collected << "개 수집됨)" << std::endl;

        std::string url = "https://www.googleapis.com/youtube/v3/videos"
                          "?part=snippet,statistics,contentDetails"
                          "&chart=mostPopular"
                          "&regionCode=KR"
                          "&maxResults=50" // 한 번에 최대 50개
                          "&key=" + apiKey;
        
        // 다음 페이지가 있다면 토큰 추가
        if (!nextPageToken.empty()) {
            url += "&pageToken=" + nextPageToken;
        }

        std::string responseString = fetchAPI(url);
        if (responseString.empty()) break;

        try {
            auto responseJson = json::parse(responseString);
            
            if (responseJson.contains("error")) {
                std::cerr << "API 에러 발생: " << responseJson["error"]["message"] << std::endl;
                break;
            }

            auto items = responseJson["items"];
            
            for (auto& video : items) {
                std::string title = video["snippet"]["title"];
                std::string videoId = video["id"];
                
                uint64_t viewCount = 0, likeCount = 0, commentCount = 0;
                if(video["statistics"].contains("viewCount")) viewCount = std::stoull(video["statistics"]["viewCount"].get<std::string>());
                if(video["statistics"].contains("likeCount")) likeCount = std::stoull(video["statistics"]["likeCount"].get<std::string>());
                if(video["statistics"].contains("commentCount")) commentCount = std::stoull(video["statistics"]["commentCount"].get<std::string>());

                // 제목 내 따옴표 처리
                std::string safe_title = title;
                size_t pos = 0;
                while ((pos = safe_title.find("'", pos)) != std::string::npos) {
                    safe_title.replace(pos, 1, "''");
                    pos += 2;
                }

                std::string sql_insert = "INSERT OR IGNORE INTO VIDEO_STATS (VIDEO_ID, TIMESTAMP, VIEWS, LIKES, COMMENTS, TITLE) VALUES ('";
                sql_insert += videoId + "', '" + current_time + "', " + 
                              std::to_string(viewCount) + ", " + 
                              std::to_string(likeCount) + ", " + 
                              std::to_string(commentCount) + ", '" + safe_title + "');";

                rc = sqlite3_exec(db, sql_insert.c_str(), 0, 0, &zErrMsg);
                if (rc != SQLITE_OK) {
                    // 중복이나 오류는 무시하고 진행
                    sqlite3_free(zErrMsg);
                } else {
                    total_collected++;
                }
            }

            // 다음 페이지 토큰 확인
            if (responseJson.contains("nextPageToken")) {
                nextPageToken = responseJson["nextPageToken"];
            } else {
                // 더 이상 페이지가 없으면 종료
                break;
            }

        } catch (const std::exception& e) {
            std::cerr << "JSON 파싱 오류: " << e.what() << std::endl;
            break;
        }
    }

    // 트랜잭션 커밋
    sqlite3_exec(db, "END TRANSACTION;", 0, 0, 0);
    sqlite3_close(db);

    std::cout << "--- 수집 완료! 총 " << total_collected << "개의 데이터를 저장했습니다. ---" << std::endl;
    return 0;
}