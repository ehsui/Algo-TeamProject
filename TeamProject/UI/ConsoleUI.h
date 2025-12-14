#pragma once
/**
 * @file ConsoleUI.h
 * @brief Simple ASCII Console UI Library
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
// Box Style
// ============================================================================
enum class BoxStyle { Unicode, Single, Ascii };
inline BoxStyle currentBoxStyle = BoxStyle::Ascii;

// ============================================================================
// Simple Box Functions (ASCII only for compatibility)
// ============================================================================

inline std::string boxTop(int width) {
    return "+" + std::string(width - 2, '=') + "+";
}

inline std::string boxBottom(int width) {
    return "+" + std::string(width - 2, '=') + "+";
}

inline std::string boxMiddle(int width) {
    return "+" + std::string(width - 2, '-') + "+";
}

inline std::string boxRow(const std::string& content, int width) {
    std::ostringstream oss;
    int contentWidth = width - 4;
    std::string trimmed = content;
    if (trimmed.length() > static_cast<size_t>(contentWidth)) {
        trimmed = trimmed.substr(0, contentWidth - 3) + "...";
    }
    oss << "| " << std::left << std::setw(contentWidth) << trimmed << " |";
    return oss.str();
}

// ============================================================================
// Screen Utilities
// ============================================================================

inline void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

inline void hideCursor() { std::cout << "\033[?25l"; }
inline void showCursor() { std::cout << "\033[?25h"; }

inline void moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

// ============================================================================
// Console Initialization
// ============================================================================

inline void initConsole(bool enableColor = false, BoxStyle style = BoxStyle::Ascii) {
    currentBoxStyle = style;
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif
}

// ============================================================================
// Logo (ASCII Art)
// ============================================================================

inline void printLogo() {
    std::cout << R"(
+=====================================================================+
|  __   __         _______      _            ____             _       |
|  \ \ / /__  _   |__   __|   _| |__   ___  |  _ \ __ _ _ __ | | __   |
|   \ V / _ \| | | | | || | | | '_ \ / _ \ | |_) / _` | '_ \| |/ /   |
|    | | (_) | |_| | | || |_| | |_) |  __/ |  _ < (_| | | | |   <    |
|    |_|\___/ \__,_| |_| \__,_|_.__/ \___| |_| \_\__,_|_| |_|_|\_\   |
|                                                                     |
|              [>]  YouTube Rank  -  Ranking Engine  v1.0             |
+=====================================================================+
)" << std::endl;
}

inline void printMiniLogo() {
    std::cout << "[>] YouTube Ranking Engine v1.0" << std::endl;
}

inline void printAsciiLogo() {
    printLogo();
}

// ============================================================================
// Message Types
// ============================================================================

enum class MessageType { Info, Success, Warning, Error };

inline void showMessage(const std::string& message, MessageType type = MessageType::Info) {
    std::string icon;
    switch (type) {
        case MessageType::Success: icon = "[OK]"; break;
        case MessageType::Warning: icon = "[!]"; break;
        case MessageType::Error:   icon = "[X]"; break;
        case MessageType::Info:
        default:                   icon = "[i]"; break;
    }
    
    int width = static_cast<int>(message.length()) + 10;
    if (width < 40) width = 40;
    
    std::cout << "\n";
    std::cout << boxTop(width) << "\n";
    std::cout << boxRow(icon + " " + message, width) << "\n";
    std::cout << boxBottom(width) << "\n";
}

// ============================================================================
// Loading Animation
// ============================================================================

inline void showSpinner(const std::string& message, int durationMs = 2000) {
    const char* frames[] = {"|", "/", "-", "\\"};
    const int frameCount = 4;
    
    hideCursor();
    auto start = std::chrono::steady_clock::now();
    int frame = 0;
    
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        
        if (elapsed >= durationMs) break;
        
        std::cout << "\r[" << frames[frame] << "] " << message << std::flush;
        frame = (frame + 1) % frameCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\r[OK] " << message << " Done!" << std::endl;
    showCursor();
}

inline void showProgressBar(int current, int total, int width = 40) {
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
