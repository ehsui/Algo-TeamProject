/**
 * @file RankEngine.cpp
 * @brief Ranking Engine Implementation with UI
 */

#include "RankEngine.h"
#include "BasicSelect.hpp"
#include "UI/YouTubeRankUI.h"
#include <iomanip>

using namespace std;

RankEngine::RankEngine(RankPolicy policy) : P(policy) {}

void RankEngine::sortInterface() {
    UI::clearScreen();
    UI::printMiniLogo();
    UI::printSortAlgorithmMenu();
    
    while (true) {
        UI::printPrompt("Select sort algorithm");
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 8) {
            P.s = static_cast<SortType>(a - 1);
            break;
        }
        
        UI::showMessage("Invalid input. Please try again.", UI::MessageType::Warning);
    }
}

void RankEngine::selectInterface() {
    UI::clearScreen();
    UI::printMiniLogo();
    UI::printSelectAlgorithmMenu();
    
    while (true) {
        UI::printPrompt("Select algorithm");
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 4) {
            P.sel = static_cast<SelectType>(a - 1);
            break;
        }
        
        UI::showMessage("Invalid input. Please try again.", UI::MessageType::Warning);
    }
}

void RankEngine::scoringInterface() {
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 65;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("        SCORING STRATEGY SELECTION", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxMiddle(W) << "\n";
    cout << UI::boxRow("  [1] Engagement Rate  (Recommended)", W) << "\n";
    cout << UI::boxRow("      Views + engagement bonus (like/comment ratio)", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("  [2] Weighted Sum", W) << "\n";
    cout << UI::boxRow("      Views*1 + Likes*50 + Comments*200", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("  [3] Normalized (0-1000 scale)", W) << "\n";
    cout << UI::boxRow("      Balanced metrics with caps", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("  [4] Legacy (Original formula)", W) << "\n";
    cout << UI::boxRow("      log(views)*100 + log(likes)*200 + log(comments)*300", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    while (true) {
        UI::printPrompt("Select scoring strategy");
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 4) {
            P.scoring = static_cast<ScoringStrategy>(a - 1);
            UI::showMessage(string("Selected: ") + ScoringStrategyName[static_cast<int>(P.scoring)], UI::MessageType::Success);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            break;
        }
        
        UI::showMessage("Invalid input. Please try again.", UI::MessageType::Warning);
    }
}

void RankEngine::multiMetricInterface() {
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 65;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("        MULTI-METRIC RANKING CONFIGURATION", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("  Lexicographic: if 1st metric is equal, compare 2nd...", W) << "\n";
    cout << UI::boxMiddle(W) << "\n";
    cout << UI::boxRow("  [1] Default     Views > Likes > Comments", W) << "\n";
    cout << UI::boxRow("  [2] Trending    DeltaViews > DeltaLikes > DeltaComments", W) << "\n";
    cout << UI::boxRow("  [3] Engagement  Likes > Comments > Views", W) << "\n";
    cout << UI::boxRow("  [4] Custom      Set your own priority", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    while (true) {
        UI::printPrompt("Select configuration");
        int a;
        cin >> a;
        
        switch (a) {
            case 1:
                P.metricConfig = MultiMetricConfig::defaultConfig();
                return;
            case 2:
                P.metricConfig = MultiMetricConfig::trendingConfig();
                return;
            case 3:
                P.metricConfig = MultiMetricConfig::engagementConfig();
                return;
            case 4: {
                P.metricConfig.priority.clear();
                
                cout << "\n";
                cout << UI::boxTop(60) << "\n";
                cout << UI::boxRow("  Custom Priority (max 5, enter 0 to finish)", 60) << "\n";
                cout << UI::boxMiddle(60) << "\n";
                cout << UI::boxRow("  1: Views    2: Likes     3: Comments", 60) << "\n";
                cout << UI::boxRow("  4: DViews   5: DLikes    6: DComments", 60) << "\n";
                cout << UI::boxBottom(60) << "\n";
                
                for (int i = 0; i < 5; i++) {
                    cout << "  Metric " << (i + 1) << " (0=finish): ";
                    int m;
                    cin >> m;
                    
                    if (m == 0) break;
                    
                    MetricType type;
                    switch (m) {
                        case 1: type = MetricType::AbsoluteViews; break;
                        case 2: type = MetricType::AbsoluteLikes; break;
                        case 3: type = MetricType::AbsoluteComments; break;
                        case 4: type = MetricType::DeltaViews; break;
                        case 5: type = MetricType::DeltaLikes; break;
                        case 6: type = MetricType::DeltaComments; break;
                        default: continue;
                    }
                    P.metricConfig.priority.push_back(type);
                }
                
                if (P.metricConfig.priority.empty()) {
                    P.metricConfig = MultiMetricConfig::defaultConfig();
                }
                return;
            }
            default:
                UI::showMessage("Invalid input. Please try again.", UI::MessageType::Warning);
        }
    }
}

void RankEngine::mapping(string videoId, int rank) {
    pos[videoId] = rank;
}

void RankEngine::setData(vector<key>& Data, vector<Video>& Src) {
    Data.clear();
    Data.reserve(Src.size());
    
    for (const Video& v : Src) {
        Data.push_back(v.makekey());
    }
}

void RankEngine::build() {
    switch (P.a) {
        case AlgorithmType::BasicSort:
            build_sortAll();
            break;
        case AlgorithmType::SelectThenSort:
            build_selectThenSort();
            break;
        case AlgorithmType::AVLTreeRank:
            build_AVLTree();
            break;
        case AlgorithmType::OnlineInsert:
            build_onlineInsert();
            break;
        case AlgorithmType::MultiMetric:
            build_MultiMetric();
            break;
    }
    
    for (size_t i = 0; i < cur.size(); i++) {
        pos[cur[i].videoId] = static_cast<int>(i);
    }
}

void RankEngine::updateScore(const string& videoId, Score newScore) {
    auto it = pos.find(videoId);
    if (it != pos.end()) {
        int idx = it->second;
        cur[idx].Value = newScore;
        adjust(idx);
    }
}

vector<key> RankEngine::getTopK() const {
    int k = min(P.k, static_cast<int>(cur.size()));
    return vector<key>(cur.begin(), cur.begin() + k);
}

void RankEngine::savePrevRanking() {
    prevCur = cur;
    prevMulti = curMulti;
}

void RankEngine::printRanking() {
    // Build ranking rows for UI
    vector<UI::RankingRow> rows;
    
    string algoName = AlgoName[P.a];
    string scoreInfo;
    
    if (P.a == AlgorithmType::BasicSort) {
        scoreInfo = sortname[P.s];
    } else if (P.a == AlgorithmType::SelectThenSort) {
        scoreInfo = string(selectname[P.sel]) + " + " + sortname[P.s];
    } else if (P.a == AlgorithmType::MultiMetric) {
        for (size_t i = 0; i < P.metricConfig.priority.size() && i < 3; i++) {
            if (i > 0) scoreInfo += ">";
            scoreInfo += getMetricName(P.metricConfig.priority[i]);
        }
    }
    
    if (P.a == AlgorithmType::MultiMetric && !curMulti.empty()) {
        for (size_t i = 0; i < curMulti.size() && i < static_cast<size_t>(P.k); i++) {
            UI::RankingRow row;
            row.rank = static_cast<int>(i + 1);
            row.title = curMulti[i].title;
            row.channel = "";
            row.score = 0;
            row.views = 0;
            row.likes = 0;
            row.rankChange = 0;
            
            if (!curMulti[i].metrics.empty()) {
                row.score = curMulti[i].metrics[0];
            }
            
            rows.push_back(row);
        }
    } else {
        auto topK = getTopK();
        for (size_t i = 0; i < topK.size(); i++) {
            UI::RankingRow row;
            row.rank = static_cast<int>(i + 1);
            row.title = topK[i].title.empty() ? "(No Title)" : topK[i].title;
            row.channel = "";
            row.score = static_cast<int64_t>(topK[i].Value);
            row.views = 0;
            row.likes = 0;
            row.rankChange = 0;
            
            rows.push_back(row);
        }
    }
    
    UI::clearScreen();
    UI::printMiniLogo();
    UI::printRankingTable(rows, algoName, scoreInfo, static_cast<int>(cur.size()));
}

void RankEngine::printPrevRanking() {
    if (prevCur.empty() && prevMulti.empty()) {
        UI::showMessage("No previous ranking available. Please refresh first.", UI::MessageType::Warning);
        cout << "\n  Press Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }
    
    vector<UI::RankingRow> rows;
    
    if (P.a == AlgorithmType::MultiMetric && !prevMulti.empty()) {
        for (size_t i = 0; i < prevMulti.size() && i < static_cast<size_t>(P.k); i++) {
            UI::RankingRow row;
            row.rank = static_cast<int>(i + 1);
            row.title = prevMulti[i].title;
            row.channel = "";
            row.score = 0;
            row.views = 0;
            row.likes = 0;
            row.rankChange = 0;
            
            if (!prevMulti[i].metrics.empty()) {
                row.score = prevMulti[i].metrics[0];
            }
            
            rows.push_back(row);
        }
    } else {
        int k = min(P.k, static_cast<int>(prevCur.size()));
        for (int i = 0; i < k; i++) {
            UI::RankingRow row;
            row.rank = i + 1;
            row.title = prevCur[i].title.empty() ? "(No Title)" : prevCur[i].title;
            row.channel = "";
            row.score = static_cast<int64_t>(prevCur[i].Value);
            row.views = 0;
            row.likes = 0;
            row.rankChange = 0;
            
            rows.push_back(row);
        }
    }
    
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 90;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("  PREVIOUS RANKING (before refresh)", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    UI::printRankingTable(rows, AlgoName[P.a], "Previous", static_cast<int>(prevCur.size()));
    
    cout << "\n  Press Enter to continue...";
    cin.ignore();
    cin.get();
}

void RankEngine::printTimeStats() {
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 55;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("        TIME STATISTICS", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxMiddle(W) << "\n";
    
    ostringstream build;
    build << "  Initial Build:    " << fixed << setprecision(2) << buildTimeMs << " ms";
    cout << UI::boxRow(build.str(), W) << "\n";
    
    if (refreshCount > 0) {
        ostringstream refresh;
        refresh << "  Last Refresh:     " << fixed << setprecision(2) << refreshTimeMs << " ms";
        cout << UI::boxRow(refresh.str(), W) << "\n";
        
        ostringstream count;
        count << "  Total Refreshes:  " << refreshCount;
        cout << UI::boxRow(count.str(), W) << "\n";
    } else {
        cout << UI::boxRow("  No refresh history", W) << "\n";
    }
    
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    cout << "\n  Press Enter to continue...";
    cin.ignore();
    cin.get();
}

void RankEngine::resultMenu() {
    while (true) {
        UI::clearScreen();
        UI::printMiniLogo();
        
        const int W = 55;
        cout << "\n";
        cout << UI::boxTop(W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxRow("        RESULT MENU", W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxMiddle(W) << "\n";
        cout << UI::boxRow("  [1] View Current Ranking", W) << "\n";
        
        if (hasRefreshData) {
            cout << UI::boxRow("  [2] Refresh (update with new data)", W) << "\n";
        } else {
            cout << UI::boxRow("  [2] Refresh (re-process same data)", W) << "\n";
        }
        
        cout << UI::boxRow("  [3] View Previous Ranking", W) << "\n";
        cout << UI::boxRow("  [4] Time Statistics", W) << "\n";
        cout << UI::boxMiddle(W) << "\n";
        cout << UI::boxRow("  [0] Exit to Main Menu", W) << "\n";
        cout << UI::boxRow("", W) << "\n";
        cout << UI::boxBottom(W) << "\n";
        
        UI::printPrompt("Select");
        int ch;
        cin >> ch;
        
        switch (ch) {
            case 1:
                printRanking();
                cout << "\n  Press Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            case 2: {
                UI::showSpinner("Refreshing data", 500);
                
                if (hasRefreshData) {
                    refresh(refreshData);
                } else {
                    refresh(srcData);
                }
                
                // Record benchmark for refresh
                addBenchmarkRecord(
                    dataSourceType,
                    static_cast<int>(srcData.size()),
                    P.k,
                    static_cast<int>(P.a),
                    static_cast<int>(P.s),
                    static_cast<int>(P.sel),
                    static_cast<int>(P.scoring),
                    buildTimeMs,
                    refreshTimeMs,
                    true  // is refresh
                );
                
                UI::showMessage("Refresh complete! Time: " + to_string(static_cast<int>(refreshTimeMs)) + " ms", UI::MessageType::Success);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                printRanking();
                cout << "\n  Press Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            }
            case 3:
                printPrevRanking();
                break;
            case 4:
                printTimeStats();
                break;
            case 0:
                return;
            default:
                UI::showMessage("Invalid input", UI::MessageType::Warning);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void RankEngine::interface(vector<Video>& Src) {
    srcData = Src;
    hasRefreshData = false;
    
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 60;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("        YOUTUBE RANKING ENGINE", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("   Data loaded: " + to_string(Src.size()) + " videos", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxMiddle(W) << "\n";
    cout << UI::boxRow("  [1] Start Ranking", W) << "\n";
    cout << UI::boxRow("  [0] Exit", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    while (true) {
        UI::printPrompt("Select");
        int ch;
        cin >> ch;
        
        if (ch == 0) return;
        if (ch == 1) break;
        
        UI::showMessage("Invalid input", UI::MessageType::Warning);
    }
    
    // Ranking method selection
    UI::clearScreen();
    UI::printMiniLogo();
    UI::printRankingModeMenu();
    
    while (true) {
        UI::printPrompt("Select ranking method");
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 5) {
            P.a = static_cast<AlgorithmType>(a - 1);
            break;
        }
        
        UI::showMessage("Invalid input", UI::MessageType::Warning);
    }
    
    // Scoring strategy selection (except MultiMetric which has its own)
    if (P.a != AlgorithmType::MultiMetric) {
        scoringInterface();
        
        // Recalculate scores with selected strategy
        for (Video& v : srcData) {
            v.calculateScore(P.scoring);
        }
    }
    
    // Top-K input
    int maxK = static_cast<int>(srcData.size());
    int k = UI::showTopKInputScreen(maxK);
    if (k < 1) k = 1;
    if (k > maxK) k = maxK;
    P.k = k;
    
    setData(cur, srcData);
    
    // Algorithm-specific settings
    if (P.a == AlgorithmType::BasicSort) {
        sortInterface();
    } 
    else if (P.a == AlgorithmType::SelectThenSort) {
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::OnlineInsert) {
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::MultiMetric) {
        multiMetricInterface();
        
        curMulti.clear();
        curMulti.reserve(srcData.size());
        for (const Video& v : srcData) {
            MultiMetricKey mmk = createMultiMetricKey(
                v.videoId,
                v.title,
                v.viewCount,
                v.likeCount,
                v.commentCount,
                P.metricConfig
            );
            curMulti.push_back(mmk);
        }
    }
    
    // Build
    UI::clearScreen();
    UI::printMiniLogo();
    UI::showSpinner("Building ranking", 1000);
    
    auto startBuild = chrono::high_resolution_clock::now();
    build();
    auto endBuild = chrono::high_resolution_clock::now();
    buildTimeMs = chrono::duration<double, milli>(endBuild - startBuild).count();
    
    // Record benchmark
    addBenchmarkRecord(
        dataSourceType,
        static_cast<int>(srcData.size()),
        P.k,
        static_cast<int>(P.a),
        static_cast<int>(P.s),
        static_cast<int>(P.sel),
        static_cast<int>(P.scoring),
        buildTimeMs,
        0.0,
        false  // is build, not refresh
    );
    
    UI::showMessage("Build complete! Time: " + to_string(static_cast<int>(buildTimeMs)) + " ms", UI::MessageType::Success);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    printRanking();
    cout << "\n  Press Enter to continue...";
    cin.ignore();
    cin.get();
    
    resultMenu();
}

// ============================================================================
// CSV File Load Interface
// ============================================================================

void RankEngine::interface() {
    UI::clearScreen();
    UI::printMiniLogo();
    
    const int W = 60;
    cout << "\n";
    cout << UI::boxTop(W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxRow("        CSV FILE LOAD MODE", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxMiddle(W) << "\n";
    cout << UI::boxRow("  [1] Start", W) << "\n";
    cout << UI::boxRow("  [0] Exit", W) << "\n";
    cout << UI::boxRow("", W) << "\n";
    cout << UI::boxBottom(W) << "\n";
    
    while (true) {
        UI::printPrompt("Select");
        int ch;
        cin >> ch;
        
        if (ch == 0) return;
        if (ch == 1) break;
        
        UI::showMessage("Invalid input", UI::MessageType::Warning);
    }
    
    cout << "\n";
    cout << UI::boxTop(55) << "\n";
    cout << UI::boxRow("  Enter CSV file path", 55) << "\n";
    cout << UI::boxRow("  (e.g., C:/data/youtube.csv)", 55) << "\n";
    cout << UI::boxBottom(55) << "\n";
    
    cin.ignore();
    string csvPath;
    cout << "\n  File path: ";
    getline(cin, csvPath);
    
    vector<string> timestamps = CsvDataLoader::getTimestamps(csvPath);
    
    if (timestamps.empty()) {
        UI::showMessage("Cannot read CSV file or no data", UI::MessageType::Error);
        cout << "\n  Press Enter to continue...";
        cin.get();
        return;
    }
    
    UI::showMessage("Found " + to_string(timestamps.size()) + " timestamp(s)", UI::MessageType::Success);
    
    // Ranking method selection
    UI::clearScreen();
    UI::printMiniLogo();
    UI::printRankingModeMenu();
    
    while (true) {
        UI::printPrompt("Select ranking method");
        int a;
        cin >> a;
        
        if (a >= 1 && a <= 5) {
            P.a = static_cast<AlgorithmType>(a - 1);
            break;
        }
        
        UI::showMessage("Invalid input", UI::MessageType::Warning);
    }
    
    // Scoring strategy selection (except MultiMetric)
    if (P.a != AlgorithmType::MultiMetric) {
        scoringInterface();
    }
    
    bool useTwoTimestamps = (P.a == AlgorithmType::AVLTreeRank || P.a == AlgorithmType::OnlineInsert);
    
    if (useTwoTimestamps && timestamps.size() >= 2) {
        UI::showSpinner("Loading two-timestamp data", 800);
        
        auto [initial, refresh] = CsvDataLoader::loadAndSplit(csvPath, P.scoring);
        srcData = initial;
        refreshData = refresh;
        hasRefreshData = true;
        
        UI::showMessage("Loaded! Initial: " + to_string(srcData.size()) + ", Refresh: " + to_string(refreshData.size()), UI::MessageType::Success);
    } else {
        UI::showSpinner("Loading data", 500);
        srcData = CsvDataLoader::loadAll(csvPath, P.scoring);
        hasRefreshData = false;
        
        UI::showMessage("Loaded " + to_string(srcData.size()) + " videos", UI::MessageType::Success);
    }
    
    if (srcData.empty()) {
        UI::showMessage("No data available", UI::MessageType::Error);
        cout << "\n  Press Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }
    
    // Top-K input
    int maxK = static_cast<int>(srcData.size());
    int k = UI::showTopKInputScreen(maxK);
    if (k < 1) k = 1;
    if (k > maxK) k = maxK;
    P.k = k;
    
    setData(cur, srcData);
    
    // Algorithm-specific settings
    if (P.a == AlgorithmType::BasicSort) {
        sortInterface();
    } 
    else if (P.a == AlgorithmType::SelectThenSort) {
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::AVLTreeRank) {
        // No additional settings
    }
    else if (P.a == AlgorithmType::OnlineInsert) {
        selectInterface();
        sortInterface();
    }
    else if (P.a == AlgorithmType::MultiMetric) {
        multiMetricInterface();
        
        curMulti.clear();
        curMulti.reserve(srcData.size());
        for (const Video& v : srcData) {
            MultiMetricKey mmk = createMultiMetricKey(
                v.videoId,
                v.title,
                v.viewCount,
                v.likeCount,
                v.commentCount,
                P.metricConfig
            );
            curMulti.push_back(mmk);
        }
    }
    
    // Build
    UI::clearScreen();
    UI::printMiniLogo();
    UI::showSpinner("Building ranking", 1000);
    
    auto startBuild = chrono::high_resolution_clock::now();
    build();
    auto endBuild = chrono::high_resolution_clock::now();
    buildTimeMs = chrono::duration<double, milli>(endBuild - startBuild).count();
    
    // Record benchmark (CSV mode = Real data)
    dataSourceType = DataSourceType::Real;
    addBenchmarkRecord(
        dataSourceType,
        static_cast<int>(srcData.size()),
        P.k,
        static_cast<int>(P.a),
        static_cast<int>(P.s),
        static_cast<int>(P.sel),
        static_cast<int>(P.scoring),
        buildTimeMs,
        0.0,
        false  // is build, not refresh
    );
    
    UI::showMessage("Build complete! Time: " + to_string(static_cast<int>(buildTimeMs)) + " ms", UI::MessageType::Success);
    
    if (hasRefreshData) {
        cout << "\n  [+] Second timestamp data ready for refresh!" << endl;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    
    printRanking();
    cout << "\n  Press Enter to continue...";
    cin.ignore();
    cin.get();
    
    resultMenu();
}
