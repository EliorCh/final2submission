#pragma once
#include <thread>
#include <chrono>
#include <iostream>
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
#include "GameDefs.h"
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

    void printCentered(const std::string& text, int y);
    static int getCenteredX(size_t width, int containerWidth = SCREEN_WIDTH) {
        return (containerWidth - static_cast<int>(width)) / 2;
    }

    inline void print(int x, int y, const std::string& text) {
        gotoxy(x, y);
        std::cout << text;
    };

}
