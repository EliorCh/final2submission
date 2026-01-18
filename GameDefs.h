#pragma once
#include "BoardChars.h"

// Defines global constants, enums and shared elements used throughout the game.

// Screen/Display Constants
constexpr int SCREEN_WIDTH = 80;
constexpr int SCREEN_HEIGHT = 25;

static constexpr int MAX_X = SCREEN_WIDTH-1;
static constexpr int MAX_Y = SCREEN_HEIGHT-1;

static constexpr int LEGEND_HEIGHT = 5;
static constexpr int LEGEND_WIDTH = 23;

// Game Configuration
static constexpr int NUM_PLAYERS = 2;
static constexpr int KEYBOARD_DELAY = 150;
static constexpr int LOAD_DELAY = 30;
static constexpr int MAX_SUB_STEPS = 10;

constexpr int MIN_REQUIRED_ROOMS = 3;
constexpr int MAX_ROOMS_CAPACITY = 8;
static constexpr int ROOM1_SCREEN = 1;
constexpr int FINAL_SCOREBOARD_START_Y = 8;

// Input Keys Constants
constexpr char HOME = 'H';
constexpr char RESTART = 'R';
constexpr char ESC_KEY = 27;

// Menu Constants
constexpr char START = '1';
constexpr char INSTRUCTIONS = '8';
constexpr char EXIT = '9';

// Direction - Used by Player, Spring, Obstacle
enum class Direction { RIGHT, DOWN, LEFT, UP, STAY, DISPOSE };
