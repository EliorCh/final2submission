#pragma once
#include <thread>
#include <chrono>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#endif

namespace Utils {
    void gotoxy(int x, int y);
    bool hasInput();
    char getChar();
    std::string toUpperCase(std::string str);
    void delay(int ms);

    // Screen Functions
    void hideCursor();
    void showCursor();
    void clearScreen();
    void initConsole();
    void restoreConsole();

}

constexpr int SCREEN_WIDTH = 80;
constexpr int SCREEN_HEIGHT = 25;
