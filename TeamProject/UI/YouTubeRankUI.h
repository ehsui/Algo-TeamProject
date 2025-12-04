#pragma once
/**
 * @file YouTubeRankUI.h
 * @brief YouTube 랭킹 시스템 통합 UI
 * 
 * 모든 UI 컴포넌트를 통합하고, 실제 데이터와 연결하는 인터페이스를 제공합니다.
 * 
 * [사용법]
 * #include "UI/YouTubeRankUI.h"
 * 
 * UI::initConsole();
 * UI::showWelcomeScreen();
 * UI::showMainMenu();
 */

#include "ConsoleUI.h"
#include "MenuUI.h"
#include "RankingUI.h"
#include "UITemplates.h"

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace UI {

// ============================================================================
// 콘솔 초기화
// ============================================================================

/**
 * @brief 콘솔 환경 초기화
 */
inline void initConsole(bool enableColor = true, BoxStyle boxStyle = BoxStyle::Unicode) {
    // 색상 설정
    Color::enabled = enableColor;
    currentBoxStyle = boxStyle;
    
    // Windows에서 UTF-8 및 ANSI 색상 활성화
    #ifdef _WIN32
    system("chcp 65001 > nul");
    
    // ANSI 이스케이프 시퀀스 활성화 (Windows 10+)
    if (enableColor) {
        system(""); // 이 빈 호출이 ANSI를 활성화함
    }
    #endif
}

// ============================================================================
// 화면 표시 함수
// ============================================================================

/**
 * @brief 시작 화면 표시
 */
inline void showWelcomeScreen() {
    clearScreen();
    printLogo();
    
    std::cout << "\n";
    showMessage("시스템 초기화 중...", MessageType::Info);
}

/**
 * @brief 초기화 완료 메시지
 */
inline void showInitComplete(int videoCount) {
    showMessage("초기화 완료! " + std::to_string(videoCount) + "개 영상 로드됨", MessageType::Success);
    std::cout << "\n  계속하려면 Enter를 누르세요...";
    std::cin.ignore();
    std::cin.get();
}

/**
 * @brief 메인 메뉴 화면 표시
 */
inline int showMainMenuScreen() {
    clearScreen();
    printMiniLogo();
    printMainMenu();
    printPrompt("선택");
    
    int choice;
    std::cin >> choice;
    return choice;
}

/**
 * @brief 정렬 알고리즘 선택 화면
 */
inline int showSortAlgorithmScreen() {
    clearScreen();
    printMiniLogo();
    printSortAlgorithmMenu();
    printPrompt("알고리즘 선택");
    
    int choice;
    std::cin >> choice;
    return choice;
}

/**
 * @brief 선택 알고리즘 선택 화면
 */
inline int showSelectAlgorithmScreen() {
    clearScreen();
    printMiniLogo();
    printSelectAlgorithmMenu();
    printPrompt("알고리즘 선택");
    
    int choice;
    std::cin >> choice;
    return choice;
}

/**
 * @brief 점수 전략 선택 화면
 */
inline int showScoringStrategyScreen() {
    clearScreen();
    printMiniLogo();
    printScoringStrategyMenu();
    printPrompt("전략 선택");
    
    int choice;
    std::cin >> choice;
    return choice;
}

/**
 * @brief 랭킹 방식 선택 화면
 */
inline int showRankingModeScreen() {
    clearScreen();
    printMiniLogo();
    printRankingModeMenu();
    printPrompt("방식 선택");
    
    int choice;
    std::cin >> choice;
    return choice;
}

/**
 * @brief Top-K 개수 입력 화면
 */
inline int showTopKInputScreen(int maxK) {
    clearScreen();
    printMiniLogo();
    
    using namespace Color;
    std::cout << "\n";
    std::cout << apply(CYAN, boxTop(50)) << "\n";
    std::cout << apply(CYAN, boxRow("  출력할 순위 수를 입력하세요", 50)) << "\n";
    std::cout << apply(CYAN, boxBottom(50)) << "\n";
    
    printNumberPrompt("입력", 1, maxK);
    
    int k;
    std::cin >> k;
    return k;
}

/**
 * @brief 랭킹 결과 화면 표시
 */
inline void showRankingScreen(
    const std::vector<RankingRow>& rankings,
    const std::string& sortAlgo,
    const std::string& scoreStrategy,
    int total
) {
    clearScreen();
    printMiniLogo();
    printRankingTable(rankings, sortAlgo, scoreStrategy, total);
    
    std::cout << "\n  계속하려면 Enter를 누르세요...";
    std::cin.ignore();
    std::cin.get();
}

/**
 * @brief 영상 상세 정보 화면
 */
inline void showVideoDetailScreen(const VideoDetail& video) {
    clearScreen();
    printMiniLogo();
    printVideoDetail(video);
    
    std::cout << "\n  계속하려면 Enter를 누르세요...";
    std::cin.ignore();
    std::cin.get();
}

/**
 * @brief 벤치마크 결과 화면
 */
inline void showBenchmarkScreen(const std::vector<BenchmarkResult>& results) {
    clearScreen();
    printMiniLogo();
    printBenchmarkResults(results);
    
    std::cout << "\n  계속하려면 Enter를 누르세요...";
    std::cin.ignore();
    std::cin.get();
}

/**
 * @brief 로딩 화면 표시
 */
inline void showLoadingScreen(const std::string& message, int durationMs = 1500) {
    showSpinner(message, durationMs);
}

/**
 * @brief 에러 메시지 표시
 */
inline void showError(const std::string& message) {
    showMessage(message, MessageType::Error);
    std::cout << "\n  계속하려면 Enter를 누르세요...";
    std::cin.ignore();
    std::cin.get();
}

/**
 * @brief 성공 메시지 표시
 */
inline void showSuccess(const std::string& message) {
    showMessage(message, MessageType::Success);
}

/**
 * @brief 종료 화면
 */
inline void showExitScreen() {
    clearScreen();
    
    using namespace Color;
    std::cout << "\n\n";
    std::cout << apply(DIM, "    ╔════════════════════════════════════════════╗") << "\n";
    std::cout << apply(DIM, "    ║                                            ║") << "\n";
    std::cout << apply(DIM, "    ║") << apply(WHITE, "      Thank you for using YouTube Rank!    ") << apply(DIM, "║") << "\n";
    std::cout << apply(DIM, "    ║") << apply(WHITE, "           프로그램을 종료합니다.          ") << apply(DIM, "║") << "\n";
    std::cout << apply(DIM, "    ║                                            ║") << "\n";
    std::cout << apply(DIM, "    ╚════════════════════════════════════════════╝") << "\n";
    std::cout << "\n";
}

// ============================================================================
// 동적 업데이트 유틸리티
// ============================================================================

/**
 * @brief 특정 위치의 텍스트만 업데이트
 * @param row 행 번호 (1-indexed)
 * @param col 열 번호 (1-indexed)
 * @param text 새 텍스트
 * 
 * [사용 예시]
 * // 점수 표시 위치만 업데이트
 * updateText(5, 20, "12345");
 */
inline void updateText(int row, int col, const std::string& text) {
    moveCursor(row, col);
    std::cout << text << std::flush;
}

/**
 * @brief 랭킹 행 하나만 업데이트
 * @param rowIndex 업데이트할 행 (0-indexed, 헤더 제외)
 * @param data 새 데이터
 */
inline void updateRankingRow(int rowIndex, const RankingRow& data) {
    // 랭킹 테이블 시작 위치 (로고 + 헤더 등 고려)
    const int TABLE_START_ROW = 12;
    const int ROW_HEIGHT = 1;
    
    int targetRow = TABLE_START_ROW + rowIndex * ROW_HEIGHT;
    moveCursor(targetRow, 1);
    
    // 해당 행만 다시 출력
    printRankingRow(data);
}

// ============================================================================
// 디버그/테스트 화면
// ============================================================================

/**
 * @brief UI 테스트 - 모든 컴포넌트 미리보기
 */
inline void showUITest() {
    initConsole(true, BoxStyle::Unicode);
    
    // 1. 로고
    clearScreen();
    std::cout << "\n[1/6] Logo Test\n";
    printLogo();
    std::cin.get();
    
    // 2. 메인 메뉴
    clearScreen();
    std::cout << "\n[2/6] Main Menu Test\n";
    printMainMenu();
    std::cin.get();
    
    // 3. 정렬 알고리즘 메뉴
    clearScreen();
    std::cout << "\n[3/6] Sort Algorithm Menu Test\n";
    printSortAlgorithmMenu();
    std::cin.get();
    
    // 4. 랭킹 테이블
    clearScreen();
    std::cout << "\n[4/6] Ranking Table Test\n";
    std::vector<RankingRow> testData = {
        {1, "Amazing Video #1 - Best Compilation Ever", "PopChannel", 98765, 1234567, 45678, 2},
        {2, "Tutorial: How to Code Like a Pro", "TechReview", 87654, 987654, 34567, -1},
        {3, "Music Video - Summer Vibes 2024", "MusicWorld", 76543, 876543, 23456, 0},
        {4, "Gaming Highlights - Epic Moments", "GamingPro", 65432, 765432, 12345, 5},
        {5, "Cooking Show: 10 Easy Recipes", "CookMaster", 54321, 654321, 11234, -2},
    };
    printRankingTable(testData, "Quick Sort", "Balanced", 1000);
    std::cin.get();
    
    // 5. 영상 상세 정보
    clearScreen();
    std::cout << "\n[5/6] Video Detail Test\n";
    VideoDetail testVideo = {1, "abc123", "Test Video Title", "Test Channel", "2024-01-15", 
                            1234567, 45678, 1234, 3661, 98765.5};
    printVideoDetail(testVideo);
    std::cin.get();
    
    // 6. 메시지 박스
    clearScreen();
    std::cout << "\n[6/6] Message Box Test\n";
    showMessage("정보 메시지입니다.", MessageType::Info);
    showMessage("성공! 작업이 완료되었습니다.", MessageType::Success);
    showMessage("경고: 주의가 필요합니다.", MessageType::Warning);
    showMessage("에러: 문제가 발생했습니다.", MessageType::Error);
    std::cin.get();
    
    showExitScreen();
}

} // namespace UI

