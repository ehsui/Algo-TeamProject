#pragma once
/**
 * @file MenuUI.h
 * @brief Menu and Selection Screen UI (ASCII version)
 */

#include "ConsoleUI.h"
#include <functional>

namespace UI {

// ============================================================================
// Menu Item Struct
// ============================================================================

struct MenuItem {
    int id;
    std::string label;
    std::string shortcut;
    std::string description;
};

// ============================================================================
// Main Menu Screen
// ============================================================================

inline void printMainMenu() {
    const int W = 60;
    
    std::cout << "\n";
    std::cout << boxTop(W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxRow("           MAIN MENU", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxMiddle(W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxRow("  [1] View Rankings", W) << "\n";
    std::cout << boxRow("  [2] Change Sort Algorithm", W) << "\n";
    std::cout << boxRow("  [3] Change Scoring Strategy", W) << "\n";
    std::cout << boxRow("  [4] Refresh Rankings", W) << "\n";
    std::cout << boxRow("  [5] Run Benchmark", W) << "\n";
    std::cout << boxRow("  [6] Video Details", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxMiddle(W) << "\n";
    std::cout << boxRow("  [0] Exit", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxBottom(W) << "\n";
}

inline void printMainMenuAscii() {
    printMainMenu();
}

// ============================================================================
// Algorithm Selection Screens
// ============================================================================

inline void printSortAlgorithmMenu() {
    const int W = 65;
    
    std::cout << "\n";
    std::cout << boxTop(W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxRow("        SELECT SORT ALGORITHM", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxMiddle(W) << "\n";
    std::cout << boxRow("  -- O(n^2) Algorithms --", W) << "\n";
    std::cout << boxRow("  [1] Selection Sort              [*---] Slow", W) << "\n";
    std::cout << boxRow("  [2] Bubble Sort                 [*---] Slow", W) << "\n";
    std::cout << boxRow("  [3] Insertion Sort              [**--] Medium", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxRow("  -- O(n log n) Algorithms --", W) << "\n";
    std::cout << boxRow("  [4] Quick Sort                  [***-] Fast", W) << "\n";
    std::cout << boxRow("  [5] Merge Sort                  [***-] Fast", W) << "\n";
    std::cout << boxRow("  [6] Heap Sort                   [***-] Fast", W) << "\n";
    std::cout << boxRow("  [7] Shell Sort                  [**--] Medium", W) << "\n";
    std::cout << boxRow("  [8] std::sort                   [****] Optimal", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxBottom(W) << "\n";
}

inline void printSelectAlgorithmMenu() {
    const int W = 65;
    
    std::cout << "\n";
    std::cout << boxTop(W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxRow("        SELECT TOP-K ALGORITHM", W) << "\n";
    std::cout << boxRow("        [Algorithm to find Top-K elements]", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxMiddle(W) << "\n";
    std::cout << boxRow("  [1] Sequential (Heap)   Heap-based     O(n log k)", W) << "\n";
    std::cout << boxRow("  [2] Quick Select        QuickSelect    O(n) avg", W) << "\n";
    std::cout << boxRow("  [3] Binary Select       Binary search  O(n log max)", W) << "\n";
    std::cout << boxRow("  [4] std::nth_element    Standard       O(n) avg", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxBottom(W) << "\n";
}

inline void printScoringStrategyMenu() {
    const int W = 65;
    
    std::cout << "\n";
    std::cout << boxTop(W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxRow("        SCORING STRATEGY", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxMiddle(W) << "\n";
    std::cout << boxRow("  [1] Engagement Rate   (Recommended)", W) << "\n";
    std::cout << boxRow("      Views + engagement bonus", W) << "\n";
    std::cout << boxRow("  [2] Weighted Sum", W) << "\n";
    std::cout << boxRow("      Views*1 + Likes*50 + Comments*200", W) << "\n";
    std::cout << boxRow("  [3] Normalized (0-1000 scale)", W) << "\n";
    std::cout << boxRow("      Balanced with caps", W) << "\n";
    std::cout << boxRow("  [4] Legacy", W) << "\n";
    std::cout << boxRow("      Original log-based formula", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxBottom(W) << "\n";
}

inline void printRankingModeMenu() {
    const int W = 65;
    
    std::cout << "\n";
    std::cout << boxTop(W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxRow("        RANKING MODE", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxMiddle(W) << "\n";
    std::cout << boxRow("  [1] BasicSort        Full sort then top K", W) << "\n";
    std::cout << boxRow("  [2] SelectThenSort   Find K then sort", W) << "\n";
    std::cout << boxRow("  [3] AVLTree          AVL Tree based", W) << "\n";
    std::cout << boxRow("  [4] OnlineInsert     Real-time update", W) << "\n";
    std::cout << boxRow("  [5] MultiMetric      Lexicographic", W) << "\n";
    std::cout << boxRow("", W) << "\n";
    std::cout << boxBottom(W) << "\n";
}

// ============================================================================
// Input Prompts
// ============================================================================

inline void printPrompt(const std::string& message = "Select") {
    std::cout << "\n  " << message << ": > ";
}

inline void printNumberPrompt(const std::string& message, int min, int max) {
    std::cout << "\n  " << message << " (" << min << "~" << max << "): > ";
}

} // namespace UI
