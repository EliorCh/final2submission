#include "Utils.h"
#include <iostream>
using namespace std;

#ifndef _WIN32
#include <unistd.h>
#include <termios.h>
#include <sys/select.h> 
#include <fcntl.h>

static struct termios oldSettings;
#endif

void Utils::gotoxy(int x, int y) {
#ifdef _WIN32
	HANDLE hConsoleOutput;
	COORD dwCursorPosition;
	cout << flush;
	dwCursorPosition.X = x;
	dwCursorPosition.Y = y;
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hConsoleOutput, dwCursorPosition);
#else
	cout << "\033[" << y + 1 << ";" << x + 1 << "H";
#endif
}

// functions by Google Gemini works both win and mac
bool Utils::hasInput() {
#ifdef _WIN32
	return _kbhit();
#else
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
#endif
}

char Utils::getChar() {
#ifdef _WIN32
	return _getch();
#else
	char ch = 0;
	if (read(STDIN_FILENO, &ch, 1) < 0) return 0;
	return ch;
#endif
}

std::string Utils::toUpperCase(std::string str) {
	for (auto& c : str) {
		c = toupper(c);
	}
	return str;
}

void Utils::delay(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
};

void Utils::hideCursor() {
#ifdef _WIN32
	HANDLE myconsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CURSOR;
	CURSOR.dwSize = 1;
	CURSOR.bVisible = FALSE;
	SetConsoleCursorInfo(myconsole, &CURSOR); 

#else
	// ?25l = Cursor Invisible (l = low/reset)
	std::cout << "\033[?25l";
#endif
}

void Utils::showCursor()
{
#ifdef _WIN32
	HANDLE myconsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO CURSOR;
	CURSOR.dwSize = 1;
	CURSOR.bVisible = TRUE;
	SetConsoleCursorInfo(myconsole, &CURSOR);

#else
	// ?25h = Cursor Visible (h = high/set)
	std::cout << "\033[?25h";
#endif
}

void Utils::clearScreen() {
#ifdef _WIN32
	system("cls");
#else
	std::cout << "\033[2J\033[1;1H";
#endif
}

void Utils::initConsole() {
#ifndef _WIN32
	tcgetattr(STDIN_FILENO, &oldSettings);
	struct termios newSettings = oldSettings;
	newSettings.c_lflag &= ~(ICANON | ECHO); // כיבוי Enter וכיבוי הדפסה
	tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
#endif
	hideCursor();
	clearScreen();
}

void Utils::restoreConsole() {
#ifndef _WIN32
	tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
#endif
	showCursor();
}