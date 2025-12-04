#pragma once
/**
 * @file MenuUI.h
 * @brief 메뉴 및 선택 화면 UI
 */

#include "ConsoleUI.h"
#include <functional>

namespace UI {

// ============================================================================
// 메뉴 아이템 구조체
// ============================================================================

struct MenuItem {
    int id;
    std::string label;
    std::string shortcut;      // 단축키 (예: "1", "Q")
    std::string description;   // 추가 설명
};

// ============================================================================
// 메인 메뉴 화면
// ============================================================================

/**
 * @brief 메인 메뉴 출력
 */
inline void printMainMenu() {
    using namespace Color;
    
    const int W = 60;  // 박스 너비
    
    std::cout << "\n";
    std::cout << apply(BRIGHT_CYAN, boxTop(W)) << "\n";
    std::cout << apply(BRIGHT_CYAN, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_CYAN, boxRow("   " + std::string(apply(BRIGHT_WHITE, "메인 메뉴  |  Main Menu")), W)) << "\n";
    std::cout << apply(BRIGHT_CYAN, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_CYAN, boxMiddle(W)) << "\n";
    
    // 메뉴 항목
    std::vector<std::pair<std::string, std::string>> items = {
        {"1", "전체 랭킹 조회         View Rankings"},
        {"2", "정렬 알고리즘 변경     Change Sort Algorithm"},
        {"3", "점수 계산 전략 변경    Change Scoring Strategy"},
        {"4", "랭킹 새로고침          Refresh Rankings"},
        {"5", "알고리즘 벤치마크      Run Benchmark"},
        {"6", "영상 상세 조회         Video Details"},
        {"─", "─────────────────────────────────────────"},
        {"0", "종료                   Exit"}
    };
    
    for (const auto& [key, label] : items) {
        if (key == "─") {
            std::cout << apply(BRIGHT_CYAN, boxMiddle(W)) << "\n";
        } else {
            std::string line = "  [" + apply(BRIGHT_YELLOW, key) + "] " + label;
            std::cout << apply(BRIGHT_CYAN, boxRow(line, W)) << "\n";
        }
    }
    
    std::cout << apply(BRIGHT_CYAN, boxRow("", W)) << "\n";
    std::cout << apply(BRIGHT_CYAN, boxBottom(W)) << "\n";
}

/**
 * @brief ASCII 버전 메인 메뉴
 */
inline void printMainMenuAscii() {
    const int W = 60;
    
    std::cout << "\n";
    std::cout << "+----------------------------------------------------------+\n";
    std::cout << "|                                                          |\n";
    std::cout << "|              Main Menu  |  메인 메뉴                     |\n";
    std::cout << "|                                                          |\n";
    std::cout << "+----------------------------------------------------------+\n";
    std::cout << "|                                                          |\n";
    std::cout << "|   [1] 전체 랭킹 조회         View Rankings               |\n";
    std::cout << "|   [2] 정렬 알고리즘 변경     Change Sort Algorithm       |\n";
    std::cout << "|   [3] 점수 계산 전략 변경    Change Scoring Strategy     |\n";
    std::cout << "|   [4] 랭킹 새로고침          Refresh Rankings            |\n";
    std::cout << "|   [5] 알고리즘 벤치마크      Run Benchmark               |\n";
    std::cout << "|   [6] 영상 상세 조회         Video Details               |\n";
    std::cout << "|                                                          |\n";
    std::cout << "+----------------------------------------------------------+\n";
    std::cout << "|   [0] 종료                   Exit                        |\n";
    std::cout << "+----------------------------------------------------------+\n";
}

// ============================================================================
// 알고리즘 선택 화면
// ============================================================================

/**
 * @brief 정렬 알고리즘 선택 메뉴
 */
inline void printSortAlgorithmMenu() {
    using namespace Color;
    
    const int W = 65;
    
    std::cout << "\n";
    std::cout << apply(MAGENTA, boxTop(W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("", W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("   " + std::string(apply(BRIGHT_WHITE, "정렬 알고리즘 선택  |  Select Sort Algorithm")), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("", W)) << "\n";
    std::cout << apply(MAGENTA, boxMiddle(W)) << "\n";
    
    // O(n²) 알고리즘
    std::cout << apply(MAGENTA, boxRow(apply(DIM, "  ── O(n²) 알고리즘 ──────────────────────────────"), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [1] Selection Sort    선택 정렬      " + std::string(apply(RED, "느림")), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [2] Bubble Sort       버블 정렬      " + std::string(apply(RED, "느림")), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [3] Insertion Sort    삽입 정렬      " + std::string(apply(YELLOW, "보통")), W)) << "\n";
    
    std::cout << apply(MAGENTA, boxRow("", W)) << "\n";
    
    // O(n log n) 알고리즘
    std::cout << apply(MAGENTA, boxRow(apply(DIM, "  ── O(n log n) 알고리즘 ─────────────────────────"), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [4] Quick Sort        퀵 정렬        " + std::string(apply(GREEN, "빠름")), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [5] Merge Sort        병합 정렬      " + std::string(apply(GREEN, "빠름")), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [6] Heap Sort         힙 정렬        " + std::string(apply(GREEN, "빠름")), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [7] Shell Sort        셸 정렬        " + std::string(apply(YELLOW, "보통")), W)) << "\n";
    std::cout << apply(MAGENTA, boxRow("  [8] std::sort         표준 정렬      " + std::string(apply(BRIGHT_GREEN, "최적")), W)) << "\n";
    
    std::cout << apply(MAGENTA, boxRow("", W)) << "\n";
    std::cout << apply(MAGENTA, boxBottom(W)) << "\n";
}

/**
 * @brief 선택 알고리즘 선택 메뉴
 */
inline void printSelectAlgorithmMenu() {
    using namespace Color;
    
    const int W = 65;
    
    std::cout << "\n";
    std::cout << apply(BLUE, boxTop(W)) << "\n";
    std::cout << apply(BLUE, boxRow("", W)) << "\n";
    std::cout << apply(BLUE, boxRow("   " + std::string(apply(BRIGHT_WHITE, "선택 알고리즘 선택  |  Select Top-K Algorithm")), W)) << "\n";
    std::cout << apply(BLUE, boxRow("", W)) << "\n";
    std::cout << apply(BLUE, boxMiddle(W)) << "\n";
    
    std::cout << apply(BLUE, boxRow("  [1] Sequential (Heap)    힙 기반        O(n log k)", W)) << "\n";
    std::cout << apply(BLUE, boxRow("  [2] Quick Select         퀵 셀렉트      O(n) 평균", W)) << "\n";
    std::cout << apply(BLUE, boxRow("  [3] Binary Select        이진 탐색      O(n log max)", W)) << "\n";
    std::cout << apply(BLUE, boxRow("  [4] std::nth_element     표준 함수      O(n) 평균", W)) << "\n";
    
    std::cout << apply(BLUE, boxRow("", W)) << "\n";
    std::cout << apply(BLUE, boxBottom(W)) << "\n";
}

/**
 * @brief 점수 계산 전략 선택 메뉴
 */
inline void printScoringStrategyMenu() {
    using namespace Color;
    
    const int W = 65;
    
    std::cout << "\n";
    std::cout << apply(GREEN, boxTop(W)) << "\n";
    std::cout << apply(GREEN, boxRow("", W)) << "\n";
    std::cout << apply(GREEN, boxRow("   " + std::string(apply(BRIGHT_WHITE, "점수 계산 전략  |  Scoring Strategy")), W)) << "\n";
    std::cout << apply(GREEN, boxRow("", W)) << "\n";
    std::cout << apply(GREEN, boxMiddle(W)) << "\n";
    
    std::cout << apply(GREEN, boxRow("  [1] View Weighted     조회수 중심     views × 1.0", W)) << "\n";
    std::cout << apply(GREEN, boxRow("  [2] Engagement        참여도 중심     likes + comments", W)) << "\n";
    std::cout << apply(GREEN, boxRow("  [3] Trending          트렌딩          최근성 + 성장률", W)) << "\n";
    std::cout << apply(GREEN, boxRow("  [4] Balanced          균형            모든 지표 종합", W)) << "\n";
    
    std::cout << apply(GREEN, boxRow("", W)) << "\n";
    std::cout << apply(GREEN, boxBottom(W)) << "\n";
}

/**
 * @brief 랭킹 방식 선택 메뉴
 */
inline void printRankingModeMenu() {
    using namespace Color;
    
    const int W = 65;
    
    std::cout << "\n";
    std::cout << apply(YELLOW, boxTop(W)) << "\n";
    std::cout << apply(YELLOW, boxRow("", W)) << "\n";
    std::cout << apply(YELLOW, boxRow("   " + std::string(apply(BRIGHT_WHITE, "랭킹 방식 선택  |  Ranking Mode")), W)) << "\n";
    std::cout << apply(YELLOW, boxRow("", W)) << "\n";
    std::cout << apply(YELLOW, boxMiddle(W)) << "\n";
    
    std::cout << apply(YELLOW, boxRow("  [1] BasicSort         전체 정렬 후 상위 K개 추출", W)) << "\n";
    std::cout << apply(YELLOW, boxRow("  [2] SelectThenSort    선택 알고리즘으로 K개 찾고 정렬", W)) << "\n";
    std::cout << apply(YELLOW, boxRow("  [3] AVLTree           AVL 트리 기반 (개발중)", W)) << "\n";
    std::cout << apply(YELLOW, boxRow("  [4] OnlineInsert      실시간 삽입 (개발중)", W)) << "\n";
    std::cout << apply(YELLOW, boxRow("  [5] RadixSort         기수 정렬 기반 (개발중)", W)) << "\n";
    
    std::cout << apply(YELLOW, boxRow("", W)) << "\n";
    std::cout << apply(YELLOW, boxBottom(W)) << "\n";
}

// ============================================================================
// 입력 프롬프트
// ============================================================================

/**
 * @brief 선택 입력 프롬프트
 */
inline void printPrompt(const std::string& message = "선택") {
    using namespace Color;
    std::cout << "\n" << apply(BRIGHT_WHITE, "  " + message + ": ") << apply(BRIGHT_YELLOW, "▶ ");
}

/**
 * @brief 숫자 입력 프롬프트
 */
inline void printNumberPrompt(const std::string& message, int min, int max) {
    using namespace Color;
    std::cout << "\n  " << message << " (" 
              << apply(BRIGHT_CYAN, std::to_string(min)) << "~" 
              << apply(BRIGHT_CYAN, std::to_string(max)) << "): "
              << apply(BRIGHT_YELLOW, "▶ ");
}

} // namespace UI

