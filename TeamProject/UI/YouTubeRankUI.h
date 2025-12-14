#pragma once
/**
 * @file YouTubeRankUI.h
 * @brief YouTube Ranking System Integrated UI (ASCII version)
 * 
 * [Usage]
 * #include "UI/YouTubeRankUI.h"
 * 
 * UI::initConsole();
 * UI::showWelcomeScreen();
 * UI::showMainMenu();
 */

#include "ConsoleUI.h"
#include "MenuUI.h"
#include "RankingUI.h"

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace UI {

// ============================================================================
// Screen Display Functions
// ============================================================================

inline void showWelcomeScreen() {
    clearScreen();
    printLogo();
    std::cout << "\n";
    showMessage("Initializing system...", MessageType::Info);
}

inline void showInitComplete(int videoCount) {
    showMessage("Initialization complete! " + std::to_string(videoCount) + " videos loaded", MessageType::Success);
    std::cout << "\n  Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

inline int showMainMenuScreen() {
    clearScreen();
    printMiniLogo();
    printMainMenu();
    printPrompt("Select");
    
    int choice;
    std::cin >> choice;
    return choice;
}

inline int showSortAlgorithmScreen() {
    clearScreen();
    printMiniLogo();
    printSortAlgorithmMenu();
    printPrompt("Select algorithm");
    
    int choice;
    std::cin >> choice;
    return choice;
}

inline int showSelectAlgorithmScreen() {
    clearScreen();
    printMiniLogo();
    printSelectAlgorithmMenu();
    printPrompt("Select algorithm");
    
    int choice;
    std::cin >> choice;
    return choice;
}

inline int showScoringStrategyScreen() {
    clearScreen();
    printMiniLogo();
    printScoringStrategyMenu();
    printPrompt("Select strategy");
    
    int choice;
    std::cin >> choice;
    return choice;
}

inline int showRankingModeScreen() {
    clearScreen();
    printMiniLogo();
    printRankingModeMenu();
    printPrompt("Select mode");
    
    int choice;
    std::cin >> choice;
    return choice;
}

inline int showTopKInputScreen(int maxK) {
    clearScreen();
    printMiniLogo();
    
    const int W = 50;
    std::cout << "\n";
    std::cout << boxTop(W) << "\n";
    std::cout << boxRow("  Enter number of rankings to display", W) << "\n";
    std::cout << boxBottom(W) << "\n";
    
    printNumberPrompt("Input", 1, maxK);
    
    int k;
    std::cin >> k;
    return k;
}

inline void showRankingScreen(
    const std::vector<RankingRow>& rankings,
    const std::string& sortAlgo,
    const std::string& scoreStrategy,
    int total
) {
    clearScreen();
    printMiniLogo();
    printRankingTable(rankings, sortAlgo, scoreStrategy, total);
    
    std::cout << "\n  Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

inline void showVideoDetailScreen(const VideoDetail& video) {
    clearScreen();
    printMiniLogo();
    printVideoDetail(video);
    
    std::cout << "\n  Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

inline void showBenchmarkScreen(const std::vector<BenchmarkResult>& results) {
    clearScreen();
    printMiniLogo();
    printBenchmarkResults(results);
    
    std::cout << "\n  Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

inline void showLoadingScreen(const std::string& message, int durationMs = 1500) {
    showSpinner(message, durationMs);
}

inline void showError(const std::string& message) {
    showMessage(message, MessageType::Error);
    std::cout << "\n  Press Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

inline void showSuccess(const std::string& message) {
    showMessage(message, MessageType::Success);
}

inline void showExitScreen() {
    clearScreen();
    
    std::cout << "\n\n";
    std::cout << "    +============================================+" << "\n";
    std::cout << "    |                                            |" << "\n";
    std::cout << "    |      Thank you for using YouTube Rank!     |" << "\n";
    std::cout << "    |              Exiting program...            |" << "\n";
    std::cout << "    |                                            |" << "\n";
    std::cout << "    +============================================+" << "\n";
    std::cout << "\n";
}

// ============================================================================
// Dynamic Update Utility
// ============================================================================

inline void updateText(int row, int col, const std::string& text) {
    moveCursor(row, col);
    std::cout << text << std::flush;
}

inline void updateRankingRow(int rowIndex, const RankingRow& data) {
    const int TABLE_START_ROW = 12;
    const int ROW_HEIGHT = 1;
    
    int targetRow = TABLE_START_ROW + rowIndex * ROW_HEIGHT;
    moveCursor(targetRow, 1);
    
    printRankingRow(data);
}

// ============================================================================
// UI Test
// ============================================================================

inline void showUITest() {
    initConsole(false, BoxStyle::Ascii);
    
    // 1. Logo
    clearScreen();
    std::cout << "\n[1/6] Logo Test\n";
    printLogo();
    std::cin.get();
    
    // 2. Main Menu
    clearScreen();
    std::cout << "\n[2/6] Main Menu Test\n";
    printMainMenu();
    std::cin.get();
    
    // 3. Sort Algorithm Menu
    clearScreen();
    std::cout << "\n[3/6] Sort Algorithm Menu Test\n";
    printSortAlgorithmMenu();
    std::cin.get();
    
    // 4. Ranking Table
    clearScreen();
    std::cout << "\n[4/6] Ranking Table Test\n";
    std::vector<RankingRow> testData = {
        {1, "Amazing Video #1 - Best Compilation", "PopChannel", 98765, 1234567, 45678, 2},
        {2, "Tutorial: How to Code Like a Pro", "TechReview", 87654, 987654, 34567, -1},
        {3, "Music Video - Summer Vibes 2024", "MusicWorld", 76543, 876543, 23456, 0},
    };
    printRankingTable(testData, "Quick Sort", "Engagement", 1000);
    std::cin.get();
    
    // 5. Video Detail
    clearScreen();
    std::cout << "\n[5/6] Video Detail Test\n";
    VideoDetail testVideo = {1, "abc123", "Test Video Title", "Test Channel", "2024-01-15", 
                            1234567, 45678, 1234, 3661, 98765.5};
    printVideoDetail(testVideo);
    std::cin.get();
    
    // 6. Message Box
    clearScreen();
    std::cout << "\n[6/6] Message Box Test\n";
    showMessage("This is an info message.", MessageType::Info);
    showMessage("Success! Operation completed.", MessageType::Success);
    showMessage("Warning: Attention required.", MessageType::Warning);
    showMessage("Error: A problem occurred.", MessageType::Error);
    std::cin.get();
    
    showExitScreen();
}

} // namespace UI
