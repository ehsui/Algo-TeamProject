/**
 * @file Video.cpp
 * @brief YouTube Video Data Structure Implementation
 */

#include "Video.h"
#include <algorithm>
#include <set>

void Video::updateInfo(const Video& v) {
    // Update statistics
    viewCount = v.viewCount;
    likeCount = v.likeCount;
    commentCount = v.commentCount;
    
    // Update metadata
    fetchTimestamp = v.fetchTimestamp;
    
    // Recalculate score (using default strategy)
    calculateScore();
}

key Video::makekey() const {
    return key{ score, videoId, title };
}

void Video::calculateScore(ScoringStrategy strategy) {
    score = CalculateScore(viewCount, likeCount, commentCount, strategy);
}

// ============================================================================
// VideoMetrics Conversion
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

YouTubeVideoInfo Video::fromVideoMetrics(const Domain::VideoMetrics& vm) {
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

YouTubeVideoInfo Video::fromCsvRow(const string& videoId, const string& timestamp,
                        long long views, long long likes, long long comments,
                        const string& title) {
    Video v;
    v.videoId = videoId;
    v.title = title;
    v.viewCount = views;
    v.likeCount = likes;
    v.commentCount = comments;
    v.fetchTimestamp = timestamp;
    v.calculateScore();  // Uses default strategy
    return v;
}

// ============================================================================
// CsvDataLoader Implementation
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

vector<Video> CsvDataLoader::loadAll(const string& filePath, ScoringStrategy strategy) {
    vector<Video> videos;
    ifstream file(filePath);
    
    if (!file.is_open()) {
        cerr << "Cannot open CSV file: " << filePath << endl;
        return videos;
    }
    
    string line;
    // Skip header
    getline(file, line);
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        vector<string> fields = parseCsvLine(line);
        if (fields.size() < 6) continue;
        
        try {
            // CSV format: video_id,title,view_count,like_count,comment_count,timestamp
            string videoId = fields[0];
            string title = fields[1];
            long long views = stoll(fields[2]);
            long long likes = stoll(fields[3]);
            long long comments = stoll(fields[4]);
            string timestamp = fields[5];
            
            Video v;
            v.videoId = videoId;
            v.title = title;
            v.viewCount = views;
            v.likeCount = likes;
            v.commentCount = comments;
            v.fetchTimestamp = timestamp;
            v.calculateScore(strategy);
            
            videos.push_back(v);
        } catch (const exception& e) {
            cerr << "CSV parsing error: " << e.what() << endl;
            continue;
        }
    }
    
    return videos;
}

vector<Video> CsvDataLoader::loadByTimestamp(const string& filePath, const string& timestamp,
                                              ScoringStrategy strategy) {
    vector<Video> all = loadAll(filePath, strategy);
    vector<Video> filtered;
    
    for (const Video& v : all) {
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
    getline(file, line);  // Skip header
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        vector<string> fields = parseCsvLine(line);
        if (fields.size() >= 6) {
            // CSV format: video_id,title,view_count,like_count,comment_count,timestamp
            string timestamp = fields[5];
            size_t spacePos = timestamp.find(' ');
            if (spacePos != string::npos) {
                timestamp = timestamp.substr(0, spacePos);
            }
            uniqueTimestamps.insert(timestamp);
        }
    }
    
    return vector<string>(uniqueTimestamps.begin(), uniqueTimestamps.end());
}

pair<vector<Video>, vector<Video>> CsvDataLoader::loadAndSplit(const string& filePath,
                                                                ScoringStrategy strategy) {
    vector<Video> all = loadAll(filePath, strategy);
    
    vector<string> timestamps = getTimestamps(filePath);
    
    if (timestamps.size() < 2) {
        return { all, {} };
    }
    
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
