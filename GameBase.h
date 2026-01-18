#pragma once
#include "Utils.h"
#include "GameDefs.h"
#include "Screen.h"
#include "Player.h"
#include "Door.h"
#include "Spring.h"
#include "Steps.h"
#include "Results.h"

#include <string>
#include <vector>
#include <algorithm>
#include <random>


// Score System
enum class ScoreEvent {
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

class ScoreUtils {
public:
    static int getValue(ScoreEvent e) {
        switch (e) {
            case ScoreEvent::UseKey:            return SCORE_USE_KEY;
            case ScoreEvent::OpenDoor:          return SCORE_OPEN_DOOR;
            case ScoreEvent::SolveRiddle:       return SCORE_SOLVE_RIDDLE;
            case ScoreEvent::FinishGameFirst:   return SCORE_FINISH_FIRST;
            case ScoreEvent::FinishGameSecond:  return SCORE_FINISH_SECOND;
        }
        return 0;
    }
};

class GameBase {
    enum class PlayerID {
        Player1 = 0,
        Player2 = 1,
        PlayersNum   = 2
    };

private:
    std::vector<Screen> screens;
    int currRoomID;

    std::vector<Player> players;

    Steps* steps;
    Results* results;

    unsigned int gameSeed=0;

protected:
    bool isRunning;
    bool gameOver;
    size_t gameCycles = 0;

    // Getters 
    Steps* getSteps() const { return steps; }
    Results* getResults() const { return results; }
    unsigned int getGameSeed() const { return gameSeed; }
    bool isFinalRoom(int dest) const { return dest == static_cast<int>(screens.size()) - 1; }


    // Setters 
    void setGame();
    void setResults(Results* r){
        delete results;
        results = r;
    }
    void setSteps(Steps* s) {
        delete steps;
        steps = s; 
    }

    // ----- Pure Virtual -----
    virtual void handleInput() = 0;
    virtual int getDelay() const = 0;
    virtual void onGameEnd() = 0;
    virtual void onPlayerDeath() = 0;

    // ----- Core Game Loop -----
    void update();
    virtual void render();

    // ----- Init Functions -----
    void initGame();
    bool loadGameFiles();
    bool initGameFiles(const std::vector<std::string> &foundFiles);

public:
    void setGameSeed(unsigned int seed) { gameSeed = seed; }
    bool loadRiddles(int filterRoomID = -1);

    // --Used in Derived Classes--
    bool restartCurrentRoom();
    bool reloadRoom(int roomID);

    static void displayPopup(const std::string &msg, const std::string &title);
    static void showError(const std::string& msg);
    static void showMessage(const std::string& msg);

    bool processKey(char ch);
    bool handleRiddles(Player& player);
    virtual bool getRiddleAnswer(Riddle* riddle, bool& outSolved) = 0;
    std::vector<std::string> getScreenSourceFiles() const;
    Player& getPlayer(PlayerID id) {
        auto index = static_cast<size_t>(id);
        return players[index];
    }

private:
    // ----- Display Functions -----
    void displayLegend(const Screen& room) const;
    void displayFinalScoreboard() const;
    void drawPlayers() const;
    virtual void showDoorStatus(const Door& d);

    // ----- Game Logic Functions -----
    void moveRoom(Player& p, int dest);
    Point getStartPoint(Player &player) const;
    bool playersCollide(Player &current, const Point &nextPos);
    void applyLifeLoss(Player& player);

    // ----- Handle Functions -----
    void handleDoor(Player& player);
    void handleSwitch(Player& player);
    bool handleSprings(Player& p);

    bool compressSpring(Player &player, Spring &sp);

    void handleBombs();
    bool handleObstacles(Player& player, const Point& nextPos);
    void handleTorch(Player& player);
    void handleCollectibles(Player& player);
    bool handleTeleports(Player& player);
    bool handleDispose(Player& p);
    bool handleAcceleratedMovement(Player &player);

    // ----- Helper Functions -----
    static bool isMatchingKey(const Player &player, const Door *door);
    void updateDoorBySwitches(int id);
    Spring* findAdjacentSpring(const Point& pos);
    void launchPlayer(Player& player, Spring& sp);
    int calcForce(const Player& pusher, const Obstacle* ob, Direction dir) const;
    bool canMoveObstacle(const std::vector<Point>& nextBody, const Obstacle* currOb);
    bool chainPushSuccess(const Player &pusher, Direction dir, const Point &obstaclePos);

 public:
    GameBase();
    virtual ~GameBase();

    void run();

};