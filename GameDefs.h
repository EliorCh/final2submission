#pragma once
#include "Utils.h"
#include <string>
#include <iostream>
#include <iomanip>

// File: GameDefs.h
// Description:
//   Defines global constants, enums, structs and shared structures used throughout the game.

// SCREEN Constants
static constexpr int MAX_X = 79;
static constexpr int MAX_Y = 24;

static constexpr int LEGEND_HEIGHT = 5;
static constexpr int LEGEND_WIDTH = 23;

static constexpr int NUM_PLAYERS = 2;
static constexpr int KEYBOARD_DELAY = 150;
static constexpr int LOAD_DELAY = 30;
static constexpr int MAX_SUB_STEPS = 10;

// Input Keys Constants
constexpr char HOME = 'H';
constexpr char RESTART = 'R';

enum { ESC = 27 };

enum ScreenID {
    MENU_SCREEN = 0,
    INSTRUCTIONS_SCREEN = 1,
    NUM_SCREENS = 2
};
constexpr int MIN_REQUIRED_ROOMS = 3;
constexpr int MAX_ROOMS_CAPACITY = 8;
static constexpr int ROOM1_SCREEN = 1;
constexpr int FINAL_SCOREBOARD_START_Y = 8;
constexpr int FINAL_SCOREBOARD_WIDTH = 20;

// Menu Constants
constexpr char START = '1';
constexpr char INSTRUCTIONS = '8';
constexpr char EXIT = '9';


// BOARD CHARS
constexpr char LEGEND = 'L';
constexpr char LEGEND_H_BORDER = '-';
constexpr char LEGEND_V_BORDER = '|';
constexpr char LEGEND_CORNER = '+';

constexpr char BOARD_WALL = 'W';
constexpr char WALL_VERT = '|';
constexpr char WALL_HORIZ = '=';

constexpr char DOOR_MIN_CHAR = '1';
constexpr char DOOR_MAX_CHAR = '9';
constexpr char DIGIT_ZERO = '0';
constexpr char BOARD_KEY = 'K';
constexpr char BOARD_BOMB = '@';
constexpr char BOARD_TORCH = '!';
constexpr char BOARD_RIDDLE = '?';
constexpr char BOARD_SPRING = '#';
constexpr char BOARD_OBSTACLE = '*';
constexpr char BOARD_SWITCH_ON = '/';
constexpr char BOARD_SWITCH_OFF = 'o';
constexpr char DARK_CHAR = '.';
constexpr char BOARD_TELEPORT = '^';

// GAME Constants
enum PlayerID { PLAYER_1 = 0, PLAYER_2 = 1 };

enum Direction { RIGHT, DOWN, LEFT, UP, STAY, DISPOSE };

enum ItemType { NONE, KEY, BOMB, TORCH };

struct Item {         
    ItemType type = NONE;
    int Index = -1;
};

inline char itemTypeToChar(ItemType type)
{
    switch (type)
    {
    case KEY:    return 'K';
    case BOMB:   return '@';
    case TORCH:  return '!';
    case NONE:  return ' ';
    default:     return ' ';
    }
}

enum SwitchRule { ALL_ON, ALL_OFF, NO_RULE };  // combinations of switches to open the door:
constexpr int BOMB_BLAST_RADIUS = 3;

// Event enum used for player's score
enum class ScoreEvent{
    UseKey,
    OpenDoor,
    SolveRiddle,
    FinishGameFirst,
    FinishGameSecond
};

constexpr int SCORE_USE_KEY = 10;
constexpr int SCORE_OPEN_DOOR = 20;
constexpr int SCORE_SOLVE_RIDDLE = 10;
constexpr int SCORE_FINISH_FIRST = 100;
constexpr int SCORE_FINISH_SECOND = 50;

inline int scoreValue(ScoreEvent e)
{
    switch (e)
    {
    case ScoreEvent::UseKey:        return SCORE_USE_KEY;
    case ScoreEvent::OpenDoor:          return SCORE_OPEN_DOOR;
    case ScoreEvent::SolveRiddle:         return SCORE_SOLVE_RIDDLE;
    case ScoreEvent::FinishGameFirst:   return SCORE_FINISH_FIRST;
    case ScoreEvent::FinishGameSecond:  return SCORE_FINISH_SECOND;
    }
    return 0;
}

inline bool matchRiddleAnswer(const std::string& expected,
    const std::string& actual)
{
    std::string e = "|" + Utils::toUpperCase(expected) + "|";
    std::string a = "|" + Utils::toUpperCase(actual) + "|";

    return e.find(a) != std::string::npos;
}

