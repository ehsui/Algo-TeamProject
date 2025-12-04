#pragma once
/**
 * @file ConsoleUI.h
 * @brief ASCII 기반 콘솔 UI 라이브러리
 * 
 * YouTube 실시간 랭킹 프로젝트를 위한 콘솔 UI 시스템입니다.
 * 
 * [테마]
 * - YouTube 데이터 분석 느낌
 * - 깔끔하고 직관적인 레이아웃
 * - Windows Terminal / VS Code 터미널 호환
 * 
 * [지원 기능]
 * - 로고 및 헤더
 * - 메뉴 박스
 * - 랭킹 테이블
 * - 상세 정보 패널
 * - 로딩 애니메이션
 * - 메시지 박스 (성공/경고/에러)
 * - ANSI 색상 지원 (선택적)
 */

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

namespace UI {

// ============================================================================
// ANSI 색상 코드
// ============================================================================
namespace Color {
    // 색상 활성화 플래그
    inline bool enabled = true;
    
    // 리셋
    inline const char* RESET   = "\033[0m";
    
    // 기본 색상
    inline const char* BLACK   = "\033[30m";
    inline const char* RED     = "\033[31m";
    inline const char* GREEN   = "\033[32m";
    inline const char* YELLOW  = "\033[33m";
    inline const char* BLUE    = "\033[34m";
    inline const char* MAGENTA = "\033[35m";
    inline const char* CYAN    = "\033[36m";
    inline const char* WHITE   = "\033[37m";
    
    // 밝은 색상
    inline const char* BRIGHT_RED     = "\033[91m";
    inline const char* BRIGHT_GREEN   = "\033[92m";
    inline const char* BRIGHT_YELLOW  = "\033[93m";
    inline const char* BRIGHT_BLUE    = "\033[94m";
    inline const char* BRIGHT_MAGENTA = "\033[95m";
    inline const char* BRIGHT_CYAN    = "\033[96m";
    inline const char* BRIGHT_WHITE   = "\033[97m";
    
    // 배경색
    inline const char* BG_RED    = "\033[41m";
    inline const char* BG_GREEN  = "\033[42m";
    inline const char* BG_YELLOW = "\033[43m";
    inline const char* BG_BLUE   = "\033[44m";
    
    // 스타일
    inline const char* BOLD      = "\033[1m";
    inline const char* DIM       = "\033[2m";
    inline const char* UNDERLINE = "\033[4m";
    
    // 색상 적용 헬퍼
    inline std::string apply(const char* color, const std::string& text) {
        if (!enabled) return text;
        return std::string(color) + text + RESET;
    }
}

// ============================================================================
// 박스 드로잉 문자 세트
// ============================================================================
namespace BoxChars {
    // Unicode 박스 (Windows Terminal, VS Code 호환)
    struct UnicodeBox {
        static constexpr const char* TL = "╔";  // Top-Left
        static constexpr const char* TR = "╗";  // Top-Right
        static constexpr const char* BL = "╚";  // Bottom-Left
        static constexpr const char* BR = "╝";  // Bottom-Right
        static constexpr const char* H  = "═";  // Horizontal
        static constexpr const char* V  = "║";  // Vertical
        static constexpr const char* LT = "╠";  // Left-T
        static constexpr const char* RT = "╣";  // Right-T
        static constexpr const char* TT = "╦";  // Top-T
        static constexpr const char* BT = "╩";  // Bottom-T
        static constexpr const char* X  = "╬";  // Cross
    };
    
    // 단선 Unicode 박스
    struct SingleBox {
        static constexpr const char* TL = "┌";
        static constexpr const char* TR = "┐";
        static constexpr const char* BL = "└";
        static constexpr const char* BR = "┘";
        static constexpr const char* H  = "─";
        static constexpr const char* V  = "│";
        static constexpr const char* LT = "├";
        static constexpr const char* RT = "┤";
        static constexpr const char* TT = "┬";
        static constexpr const char* BT = "┴";
        static constexpr const char* X  = "┼";
    };
    
    // 순수 ASCII 박스 (호환성 최고)
    struct AsciiBox {
        static constexpr const char* TL = "+";
        static constexpr const char* TR = "+";
        static constexpr const char* BL = "+";
        static constexpr const char* BR = "+";
        static constexpr const char* H  = "-";
        static constexpr const char* V  = "|";
        static constexpr const char* LT = "+";
        static constexpr const char* RT = "+";
        static constexpr const char* TT = "+";
        static constexpr const char* BT = "+";
        static constexpr const char* X  = "+";
    };
}

// 현재 사용할 박스 스타일 (기본: Unicode)
enum class BoxStyle { Unicode, Single, Ascii };
inline BoxStyle currentBoxStyle = BoxStyle::Unicode;

// ============================================================================
// 로고 (ASCII Art)
// ============================================================================

/**
 * @brief 메인 로고 출력
 */
inline void printLogo() {
    using namespace Color;
    
    // YouTube 스타일 로고 (한글 버전)
    std::cout << apply(BRIGHT_RED, R"(
    ╔═══════════════════════════════════════════════════════════════════╗
    ║   __   __        _______    _                ____             _   ║
    ║   \ \ / /__  ___|_   _| |  | |__   ___      |  _ \ __ _ _ __ | | _║
    ║    \ V / _ \/ _ \ | | | |  | '_ \ / _ \     | |_) / _` | '_ \| |/ /║
    ║     | | (_) | __/ | | | |__| |_) |  __/     |  _ < (_| | | | |   < ║
    ║     |_|\___/\___| |_|_|____|_.__/ \___|     |_| \_\__,_|_| |_|_|\_\║
    ║                                                                   ║
    ║            ▶  실시간 랭킹 엔진  |  Real-time Ranking Engine       ║
    ╚═══════════════════════════════════════════════════════════════════╝
)") << std::endl;
}

/**
 * @brief 미니 로고 (상단 헤더용)
 */
inline void printMiniLogo() {
    using namespace Color;
    std::cout << apply(BRIGHT_RED, "▶ YouTube") 
              << apply(WHITE, " Ranking Engine v1.0") << std::endl;
}

/**
 * @brief 순수 ASCII 로고 (호환성 최고)
 */
inline void printAsciiLogo() {
    std::cout << R"(
    +===================================================================+
    |   __   __        _______    _                ____             _   |
    |   \ \ / /__  ___|_   _| |  | |__   ___      |  _ \ __ _ _ __ | | _|
    |    \ V / _ \/ _ \ | | | |  | '_ \ / _ \     | |_) / _` | '_ \| |/ /|
    |     | | (_) | __/ | | | |__| |_) |  __/     |  _ < (_| | | | |   < |
    |     |_|\___/\___| |_|_|____|_.__/ \___|     |_| \_\__,_|_| |_|_|\_\|
    |                                                                   |
    |            [>] Real-time Ranking Engine                           |
    +===================================================================+
)" << std::endl;
}

// ============================================================================
// 박스 생성 함수
// ============================================================================

/**
 * @brief 수평선 생성
 */
inline std::string horizontalLine(int width, const char* left, const char* mid, const char* right) {
    std::string line = left;
    for (int i = 0; i < width - 2; ++i) line += mid;
    line += right;
    return line;
}

/**
 * @brief 박스 상단 테두리
 */
inline std::string boxTop(int width) {
    switch (currentBoxStyle) {
        case BoxStyle::Unicode:
            return horizontalLine(width, BoxChars::UnicodeBox::TL, BoxChars::UnicodeBox::H, BoxChars::UnicodeBox::TR);
        case BoxStyle::Single:
            return horizontalLine(width, BoxChars::SingleBox::TL, BoxChars::SingleBox::H, BoxChars::SingleBox::TR);
        case BoxStyle::Ascii:
        default:
            return horizontalLine(width, BoxChars::AsciiBox::TL, BoxChars::AsciiBox::H, BoxChars::AsciiBox::TR);
    }
}

/**
 * @brief 박스 하단 테두리
 */
inline std::string boxBottom(int width) {
    switch (currentBoxStyle) {
        case BoxStyle::Unicode:
            return horizontalLine(width, BoxChars::UnicodeBox::BL, BoxChars::UnicodeBox::H, BoxChars::UnicodeBox::BR);
        case BoxStyle::Single:
            return horizontalLine(width, BoxChars::SingleBox::BL, BoxChars::SingleBox::H, BoxChars::SingleBox::BR);
        case BoxStyle::Ascii:
        default:
            return horizontalLine(width, BoxChars::AsciiBox::BL, BoxChars::AsciiBox::H, BoxChars::AsciiBox::BR);
    }
}

/**
 * @brief 박스 중간 구분선
 */
inline std::string boxMiddle(int width) {
    switch (currentBoxStyle) {
        case BoxStyle::Unicode:
            return horizontalLine(width, BoxChars::UnicodeBox::LT, BoxChars::UnicodeBox::H, BoxChars::UnicodeBox::RT);
        case BoxStyle::Single:
            return horizontalLine(width, BoxChars::SingleBox::LT, BoxChars::SingleBox::H, BoxChars::SingleBox::RT);
        case BoxStyle::Ascii:
        default:
            return horizontalLine(width, BoxChars::AsciiBox::LT, BoxChars::AsciiBox::H, BoxChars::AsciiBox::RT);
    }
}

/**
 * @brief 박스 내용 행
 */
inline std::string boxRow(const std::string& content, int width) {
    const char* V;
    switch (currentBoxStyle) {
        case BoxStyle::Unicode: V = BoxChars::UnicodeBox::V; break;
        case BoxStyle::Single:  V = BoxChars::SingleBox::V; break;
        case BoxStyle::Ascii:
        default:                V = BoxChars::AsciiBox::V; break;
    }
    
    std::ostringstream oss;
    oss << V << " " << std::left << std::setw(width - 4) << content << " " << V;
    return oss.str();
}

// ============================================================================
// 화면 유틸리티
// ============================================================================

/**
 * @brief 화면 지우기
 */
inline void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

/**
 * @brief 커서 숨기기/보이기
 */
inline void hideCursor() { std::cout << "\033[?25l"; }
inline void showCursor() { std::cout << "\033[?25h"; }

/**
 * @brief 커서 위치 이동
 */
inline void moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

// ============================================================================
// 메시지 박스
// ============================================================================

enum class MessageType { Info, Success, Warning, Error };

/**
 * @brief 메시지 박스 출력
 */
inline void showMessage(const std::string& message, MessageType type = MessageType::Info) {
    using namespace Color;
    
    const char* color;
    std::string icon;
    
    switch (type) {
        case MessageType::Success:
            color = BRIGHT_GREEN;
            icon = "[✓]";
            break;
        case MessageType::Warning:
            color = BRIGHT_YELLOW;
            icon = "[!]";
            break;
        case MessageType::Error:
            color = BRIGHT_RED;
            icon = "[✗]";
            break;
        case MessageType::Info:
        default:
            color = BRIGHT_CYAN;
            icon = "[i]";
            break;
    }
    
    int width = static_cast<int>(message.length()) + 10;
    width = std::max(width, 40);
    
    std::cout << "\n";
    std::cout << apply(color, boxTop(width)) << "\n";
    std::cout << apply(color, boxRow(icon + " " + message, width)) << "\n";
    std::cout << apply(color, boxBottom(width)) << "\n";
}

// ============================================================================
// 로딩 애니메이션
// ============================================================================

/**
 * @brief 스피너 애니메이션
 */
inline void showSpinner(const std::string& message, int durationMs = 2000) {
    const char* frames[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    const int frameCount = 10;
    
    hideCursor();
    auto start = std::chrono::steady_clock::now();
    int frame = 0;
    
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        
        if (elapsed >= durationMs) break;
        
        std::cout << "\r" << Color::apply(Color::BRIGHT_CYAN, frames[frame]) 
                  << " " << message << std::flush;
        
        frame = (frame + 1) % frameCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
    
    std::cout << "\r" << Color::apply(Color::BRIGHT_GREEN, "✓") 
              << " " << message << " 완료!" << std::endl;
    showCursor();
}

/**
 * @brief 프로그레스 바
 */
inline void showProgressBar(int current, int total, int width = 40) {
    float progress = static_cast<float>(current) / total;
    int filled = static_cast<int>(progress * width);
    
    std::cout << "\r[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) std::cout << Color::apply(Color::BRIGHT_GREEN, "█");
        else if (i == filled) std::cout << Color::apply(Color::BRIGHT_YELLOW, "▓");
        else std::cout << Color::apply(Color::DIM, "░");
    }
    std::cout << "] " << std::setw(3) << static_cast<int>(progress * 100) << "%" << std::flush;
}

/**
 * @brief ASCII 프로그레스 바 (호환성)
 */
inline void showAsciiProgressBar(int current, int total, int width = 40) {
    float progress = static_cast<float>(current) / total;
    int filled = static_cast<int>(progress * width);
    
    std::cout << "\r[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) std::cout << "#";
        else std::cout << "-";
    }
    std::cout << "] " << std::setw(3) << static_cast<int>(progress * 100) << "%" << std::flush;
}

} // namespace UI

