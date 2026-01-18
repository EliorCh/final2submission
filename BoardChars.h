#pragma once

// BoardChars.h - All board character constants used in the game

// Empty cell
constexpr char EMPTY_CELL = ' ';

// Legend characters
constexpr char LEGEND = 'L';
constexpr char LEGEND_H_BORDER = '-';
constexpr char LEGEND_V_BORDER = '|';
constexpr char LEGEND_CORNER = '+';

// Wall characters
constexpr char BOARD_WALL = 'W';
constexpr char WALL_VERT = '|';
constexpr char WALL_HORIZ = '=';

// Door characters
constexpr char DOOR_MIN_CHAR = '1';
constexpr char DOOR_MAX_CHAR = '9';
constexpr char DIGIT_ZERO = '0';

// Collectible characters
constexpr char BOARD_KEY = 'K';
constexpr char BOARD_BOMB = '@';
constexpr char BOARD_TORCH = '!';

// Interactive object characters
constexpr char BOARD_RIDDLE = '?';
constexpr char BOARD_SPRING = '#';
constexpr char BOARD_OBSTACLE = '*';
constexpr char BOARD_SWITCH_ON = '/';
constexpr char BOARD_SWITCH_OFF = 'o';
constexpr char BOARD_TELEPORT = '^';

// Dark area character
constexpr char DARK_CHAR = '.';
