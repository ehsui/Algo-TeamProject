/**
 * @file Video.cpp
 * @brief YouTube 영상 정보 구조체 구현
 */

#include "Video.h"
#include <algorithm>
#include <set>

void Video::updateInfo(const Video& v) {
    // 통계 정보 업데이트
    viewCount = v.viewCount;
    likeCount = v.likeCount;
    commentCount = v.commentCount;
    
    // 메타 정보 업데이트
    fetchTimestamp = v.fetchTimestamp;
    
    // 점수 재계산
    calculateScore();
}

key Video::makekey() const {
    return key{ score, videoId, title };
}

void Video::calculateScore() {
    // Score.h의 함수 사용
    score = CalculateScore(viewCount, likeCount, commentCount);
}

// ============================================================================
// VideoMetrics 변환
// ============================================================================

Domain::VideoMetrics Video::toVideoMetrics() const {
    Domain::VideoMetrics vm;
    vm.id = videoId;
    vm.title = title;
    vm.channelId = channelId;
    vm.channelName = channelTitle;
    vm.viewCount = viewCount;
    vm.likeCount = likeCount;
    vm.commentCount = commentCount;
    vm.durationSeconds = durationSeconds;
    vm.score = static_cast<double>(score);
    return vm;
}

Video Video::fromVideoMetrics(const Domain::VideoMetrics& vm) {
    Video v;
    v.videoId = vm.id;
    v.title = vm.title;
    v.channelId = vm.channelId;
    v.channelTitle = vm.channelName;
    v.viewCount = vm.viewCount;
    v.likeCount = vm.likeCount;
    v.commentCount = vm.commentCount;
    v.durationSeconds = vm.durationSeconds;
    v.score = static_cast<Score>(vm.score);
    return v;
}

Video Video::fromCsvRow(const string& videoId, const string& timestamp,
                        long long views, long long likes, long long comments,
                        const string& title) {
    Video v;
    v.videoId = videoId;
    v.title = title;
    v.viewCount = views;
    v.likeCount = likes;
    v.commentCount = comments;
    v.fetchTimestamp = timestamp;
    v.calculateScore();
    return v;
}

// ============================================================================
// CsvDataLoader 구현
// ============================================================================

vector<string> CsvDataLoader::parseCsvLine(const string& line) {
    vector<string> result;
    string current;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(current);
    
    return result;
}

vector<Video> CsvDataLoader::loadAll(const string& filePath) {
    vector<Video> videos;
    ifstream file(filePath);
    
    if (!file.is_open()) {
        cerr << "CSV 파일을 열 수 없습니다: " << filePath << endl;
        return videos;
    }
    
    string line;
    // 헤더 스킵
    getline(file, line);
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        vector<string> fields = parseCsvLine(line);
        if (fields.size() < 6) continue;
        
        try {
            string videoId = fields[0];
            string timestamp = fields[1];
            long long views = stoll(fields[2]);
            long long likes = stoll(fields[3]);
            long long comments = stoll(fields[4]);
            string title = fields[5];
            
            videos.push_back(Video::fromCsvRow(videoId, timestamp, views, likes, comments, title));
        } catch (const exception& e) {
            cerr << "CSV 파싱 오류: " << e.what() << endl;
            continue;
        }
    }
    
    return videos;
}

vector<Video> CsvDataLoader::loadByTimestamp(const string& filePath, const string& timestamp) {
    vector<Video> all = loadAll(filePath);
    vector<Video> filtered;
    
    for (const Video& v : all) {
        // timestamp가 해당 날짜로 시작하는지 확인
        if (v.fetchTimestamp.find(timestamp) != string::npos) {
            filtered.push_back(v);
        }
    }
    
    return filtered;
}

vector<string> CsvDataLoader::getTimestamps(const string& filePath) {
    set<string> uniqueTimestamps;
    ifstream file(filePath);
    
    if (!file.is_open()) {
        return {};
    }
    
    string line;
    getline(file, line);  // 헤더 스킵
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        vector<string> fields = parseCsvLine(line);
        if (fields.size() >= 2) {
            // 날짜 부분만 추출 (예: "2025-11-26 19:40:48" → "2025-11-26")
            string timestamp = fields[1];
            size_t spacePos = timestamp.find(' ');
            if (spacePos != string::npos) {
                timestamp = timestamp.substr(0, spacePos);
            }
            uniqueTimestamps.insert(timestamp);
        }
    }
    
    return vector<string>(uniqueTimestamps.begin(), uniqueTimestamps.end());
}

pair<vector<Video>, vector<Video>> CsvDataLoader::loadAndSplit(const string& filePath) {
    vector<Video> all = loadAll(filePath);
    
    // 시점 목록 추출
    vector<string> timestamps = getTimestamps(filePath);
    
    if (timestamps.size() < 2) {
        // 시점이 하나뿐이면 전체를 초기 데이터로
        return { all, {} };
    }
    
    // 첫 번째 시점 = 초기, 두 번째 시점 = 갱신
    string firstTimestamp = timestamps[0];
    string secondTimestamp = timestamps[1];
    
    vector<Video> initial, refresh;
    
    for (const Video& v : all) {
        if (v.fetchTimestamp.find(firstTimestamp) != string::npos) {
            initial.push_back(v);
        } else if (v.fetchTimestamp.find(secondTimestamp) != string::npos) {
            refresh.push_back(v);
        }
    }
    
    return { initial, refresh };
}