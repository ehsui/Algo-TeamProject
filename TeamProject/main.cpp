/**
 * @file main.cpp
 * @brief YouTube Ranking System Main Entry Point
 */

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <cstdlib>
#include <ctime>

// UI Module
#include "UI/YouTubeRankUI.h"

// Main Ranking Engine (5 algorithms)
#include "RankEngine.h"

// Benchmark History
#include "BenchmarkHistory.h"

using namespace std;

// ============================================================================
// DataCollector Path & API Key
// ============================================================================

const string DATA_COLLECTOR_PATH = "C:/Users/chois/source/repos/TeamProject/DataCollector/out/build/x64-Debug/YouTubeDataCollector.exe";
const string API_KEY_CONFIG_PATH = "C:/Users/chois/source/repos/TeamProject/DataCollector/config.txt";

/**
 * @brief Load API key from config file
 */
string loadApiKey() {
    ifstream file(API_KEY_CONFIG_PATH);
    if (!file.is_open()) {
        return "";
    }
    
    string key;
    getline(file, key);
    
    // Trim whitespace
    size_t start = key.find_first_not_of(" \t\r\n");
    size_t end = key.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    
    return key.substr(start, end - start + 1);
}

bool callDataCollector() {
    UI::showMessage("Calling DataCollector...", UI::MessageType::Info);
    
    // Check if executable exists
    ifstream exeCheck(DATA_COLLECTOR_PATH);
    if (!exeCheck.is_open()) {
        UI::showMessage("DataCollector not found!", UI::MessageType::Error);
        cout << "  Path: " << DATA_COLLECTOR_PATH << endl;
        return false;
    }
    exeCheck.close();
    
    // Load API key
    string apiKey = loadApiKey();
    if (apiKey.empty()) {
        UI::showMessage("API key not found!", UI::MessageType::Error);
        cout << "  Config path: " << API_KEY_CONFIG_PATH << endl;
        cout << "  Please add your YouTube API key to config.txt" << endl;
        return false;
    }
    
    // Show masked API key
    cout << "\n  API Key: " << apiKey.substr(0, 8) << "..." 
         << apiKey.substr(apiKey.length() - 4) << endl;
    
    // Build command with API key (Windows requires special quoting)
    // Format: cmd /c "full_path" arg1 arg2
    string command = "cmd /c \"\"" + DATA_COLLECTOR_PATH + "\" " + apiKey + " --collect\"";
    
    cout << "  Executing: DataCollector --collect\n";
    cout << "  " << string(50, '-') << endl;
    
    int result = system(command.c_str());
    
    cout << "  " << string(50, '-') << endl;
    
    if (result == 0) {
        UI::showMessage("DataCollector completed successfully!", UI::MessageType::Success);
        return true;
    } else {
        UI::showMessage("DataCollector failed (code: " + to_string(result) + ")", UI::MessageType::Error);
        return false;
    }
}

// ============================================================================
// Dummy Data Generator
// ============================================================================

vector<Video> generateDummyData(int count) {
    if (count < 1) count = 1;
    if (count > 2000) count = 2000;
    
    vector<Video> videos;
    videos.reserve(count);
    
    vector<string> titlePrefixes = {
        "Amazing", "Ultimate", "Best", "Top", "Epic", 
        "Incredible", "Awesome", "Must Watch", "Trending", "Viral"
    };
    
    vector<string> titleSuffixes = {
        "Compilation", "Tutorial", "Review", "Highlights", "Guide",
        "Tips & Tricks", "Challenge", "Reaction", "Vlog", "Unboxing"
    };
    
    vector<string> channelNames = {
        "TechMaster", "GameZone", "MusicHub", "CookingPro", "TravelWorld",
        "FitnessFan", "ComedyKing", "NewsDaily", "ScienceNow", "ArtCreative",
        "MovieBuff", "SportsFan", "LifeHacks", "BeautyTips", "PetLovers"
    };
    
    srand(static_cast<unsigned>(time(nullptr)));
    
    for (int i = 0; i < count; i++) {
        Video v;
        v.videoId = "vid_" + to_string(i + 1) + "_" + to_string(rand() % 10000);
        v.title = titlePrefixes[rand() % titlePrefixes.size()] + " " +
                  titleSuffixes[rand() % titleSuffixes.size()] + " #" + 
                  to_string(i + 1);
        
        int channelIdx = rand() % channelNames.size();
        v.channelId = "ch_" + to_string(channelIdx);
        v.channelTitle = channelNames[channelIdx];
        v.fetchTimestamp = "2024-01-15 12:00:00";
        
        v.viewCount = (rand() % 10000 + 1) * 1000;
        double likeRatio = (rand() % 30 + 20) / 1000.0;
        v.likeCount = static_cast<int64_t>(v.viewCount * likeRatio);
        double commentRatio = (rand() % 4 + 1) / 1000.0;
        v.commentCount = static_cast<int64_t>(v.viewCount * commentRatio);
        
        v.calculateScore();
        
        videos.push_back(v);
    }
    
    return videos;
}

pair<vector<Video>, vector<Video>> generateDummyDataWithRefresh(int count) {
    if (count < 1) count = 1;
    if (count > 2000) count = 2000;
    
    vector<Video> initial = generateDummyData(count);
    vector<Video> refreshed;
    refreshed.reserve(count);
    
    srand(static_cast<unsigned>(time(nullptr)) + 1);
    
    for (const auto& v : initial) {
        Video updated = v;
        updated.fetchTimestamp = "2024-01-15 18:00:00";
        
        double viewIncrease = 1.0 + (rand() % 15 + 5) / 100.0;
        updated.viewCount = static_cast<int64_t>(v.viewCount * viewIncrease);
        
        double likeRatio = (rand() % 30 + 20) / 1000.0;
        updated.likeCount = static_cast<int64_t>(updated.viewCount * likeRatio);
        
        double commentRatio = (rand() % 4 + 1) / 1000.0;
        updated.commentCount = static_cast<int64_t>(updated.viewCount * commentRatio);
        
        updated.calculateScore();
        
        refreshed.push_back(updated);
    }
    
    return {initial, refreshed};
}

// ============================================================================
// Benchmark History Display
// ============================================================================

void showBenchmarkHistory() {
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 60;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("        ALGORITHM BENCHMARK HISTORY", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("  Compare performance of 5 ranking algorithms", W) << "\n";
    cout << UI::boxRow("  Records are saved each time you run ranking.", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    // Get benchmark history
    BenchmarkHistory& history = BenchmarkHistory::getInstance();
    
    if (history.isEmpty()) {
        cout << "\n";
        cout << "  +----------------------------------------------------------+\n";
        cout << "  |                                                          |\n";
        cout << "  |     No benchmark records yet!                            |\n";
        cout << "  |                                                          |\n";
        cout << "  |     Run some ranking operations first:                   |\n";
        cout << "  |       [1] Real Data Mode                                 |\n";
        cout << "  |       [2] Dummy Data Mode                                |\n";
        cout << "  |                                                          |\n";
        cout << "  |     Each ranking operation will be recorded here.        |\n";
        cout << "  |                                                          |\n";
        cout << "  +----------------------------------------------------------+\n";
    } else {
        history.printHistory();
    }
    
    cout << "\n";
    cout << UI::boxTop(45) << "\n";
    cout << UI::boxRow("  [1] Clear all records", 45) << "\n";
    cout << UI::boxRow("  [0] Back to main menu", 45) << "\n";
    cout << UI::boxBottom(45) << "\n";
    
    UI::printPrompt("Select");
    int choice;
    cin >> choice;
    
    if (choice == 1) {
        history.clear();
        UI::showMessage("All records cleared!", UI::MessageType::Success);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
}

// ============================================================================
// Real Data Mode (CSV)
// ============================================================================

const string DEFAULT_CSV_DIR = "C:/Users/chois/source/repos/TeamProject/data/";

void runRealDataMode() {
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 60;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("  Real Data Mode (CSV from YouTube API)", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    string latestPath = DEFAULT_CSV_DIR + "youtube_latest.csv";
    string allPath = DEFAULT_CSV_DIR + "youtube_all.csv";
    
    ifstream testFile(latestPath);
    if (!testFile.is_open()) {
        UI::showMessage("No CSV data found", UI::MessageType::Warning);
        
        cout << "\n";
        cout << UI::boxTop(50) << "\n";
        cout << UI::boxRow("  [1] Fetch new data from YouTube API", 50) << "\n";
        cout << UI::boxRow("  [0] Back to main menu", 50) << "\n";
        cout << UI::boxBottom(50) << "\n";
        
        UI::printPrompt("Select");
        int choice;
        cin >> choice;
        
        if (choice == 1) {
            if (!callDataCollector()) {
                cout << "\n  Press Enter to continue...";
                cin.ignore();
                cin.get();
                return;
            }
        } else {
            return;
        }
    } else {
        testFile.close();
        
        UI::showMessage("Existing data found: youtube_latest.csv", UI::MessageType::Success);
        
        cout << "\n";
        cout << UI::boxTop(50) << "\n";
        cout << UI::boxRow("  [1] Use existing data", 50) << "\n";
        cout << UI::boxRow("  [2] Fetch new data (refresh)", 50) << "\n";
        cout << UI::boxRow("  [0] Back to main menu", 50) << "\n";
        cout << UI::boxBottom(50) << "\n";
        
        UI::printPrompt("Select");
        int choice;
        cin >> choice;
        
        if (choice == 2) {
            if (!callDataCollector()) {
                cout << "\n  Press Enter to continue...";
                cin.ignore();
                cin.get();
                return;
            }
        } else if (choice == 0) {
            return;
        }
    }
    
    UI::showSpinner("Loading CSV data", 500);
    
    vector<Video> videos = CsvDataLoader::loadAll(latestPath);
    
    if (videos.empty()) {
        UI::showMessage("No data in CSV file", UI::MessageType::Error);
        cout << "\n  Press Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }
    
    UI::showMessage("Loaded " + to_string(videos.size()) + " videos", UI::MessageType::Success);
    
    // Check for refresh data
    ifstream allFile(allPath);
    bool hasRefreshData = false;
    vector<Video> refreshVideos;
    
    if (allFile.is_open()) {
        allFile.close();
        vector<string> timestamps = CsvDataLoader::getTimestamps(allPath);
        
        if (timestamps.size() >= 2) {
            cout << "\n  [+] Refresh data available! ("
                 << timestamps.size() << " timestamps)\n";
            
            auto [initial, refresh] = CsvDataLoader::loadAndSplit(allPath);
            if (!refresh.empty()) {
                refreshVideos = refresh;
                hasRefreshData = true;
            }
        }
    }
    
    cout << endl;
    
    RankPolicy policy;
    RankEngine engine(policy);
    engine.setDataSourceType(DataSourceType::Real);  // Set data source type
    engine.interface(videos);
}

// ============================================================================
// Dummy Data Mode
// ============================================================================

void runDummyDataMode() {
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 60;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("  Dummy Data Mode (Test Data Generator)", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    UI::printNumberPrompt("Number of videos to generate", 1, 2000);
    int count;
    cin >> count;
    
    if (count < 1) count = 1;
    if (count > 2000) count = 2000;
    
    cout << "\n";
    cout << UI::boxTop(55) << "\n";
    cout << UI::boxRow("  Generate refresh data? (for AVL/OnlineInsert)", 55) << "\n";
    cout << UI::boxMiddle(55) << "\n";
    cout << UI::boxRow("  [1] Yes (two timestamps)", 55) << "\n";
    cout << UI::boxRow("  [2] No (single timestamp)", 55) << "\n";
    cout << UI::boxBottom(55) << "\n";
    
    UI::printPrompt("Select");
    int refreshChoice;
    cin >> refreshChoice;
    
    UI::showSpinner("Generating dummy data", 800);
    
    RankPolicy policy;
    RankEngine engine(policy);
    engine.setDataSourceType(DataSourceType::Dummy);  // Set data source type
    
    if (refreshChoice == 1) {
        auto [initial, refreshed] = generateDummyDataWithRefresh(count);
        UI::showMessage("Generated " + to_string(initial.size()) + " videos with refresh data", UI::MessageType::Success);
        engine.interface(initial);
    } else {
        vector<Video> dummyData = generateDummyData(count);
        UI::showMessage("Generated " + to_string(dummyData.size()) + " videos", UI::MessageType::Success);
        engine.interface(dummyData);
    }
}

// ============================================================================
// Main Function
// ============================================================================

int main() {
    // Initialize UI
    UI::initConsole(false, UI::BoxStyle::Ascii);
    
    // Welcome screen
    UI::showWelcomeScreen();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    int choice;
    bool running = true;
    
    while (running) {
        UI::clearScreen();
        UI::printLogo();
        
        // Main menu
        const int W = 60;
        cout << "\n";
        cout << UI::boxTop(W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxRow("               MAIN MENU", W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxMiddle(W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxRow("  [1] Real Data   - Load from YouTube API CSV", W) << "\n";
        cout << UI::boxRow("  [2] Dummy Data  - Generate test data (up to 2000)", W) << "\n";
        cout << UI::boxRow("  [3] Benchmark   - View algorithm performance history", W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxMiddle(W) << "\n";
        cout << UI::boxRow("  [0] Exit", W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxBottom(W) << "\n";
        
        UI::printPrompt("Select");
        cin >> choice;
        
        switch (choice) {
            case 1:
                runRealDataMode();
                break;
            
            case 2:
                runDummyDataMode();
                break;
            
            case 3:
                showBenchmarkHistory();
                break;
            
            case 0:
                running = false;
                UI::showExitScreen();
                break;
            
            default:
                UI::showMessage("Invalid selection", UI::MessageType::Warning);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                break;
        }
    }
    
    return 0;
}
