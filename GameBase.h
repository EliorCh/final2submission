#pragma once
#include "Utils.h"
#include "GameDefs.h"
#include "Screen.h"
#include "Player.h"
#include "Door.h"
#include "Spring.h"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "Steps.h"
#include "Results.h"


class GameBase {
private:
    std::vector<Screen> screens;
    int currRoomID;

    Player players[NUM_PLAYERS];
    int playerRoom[NUM_PLAYERS];
    int roomsDone[NUM_PLAYERS];
    bool playerFinished[NUM_PLAYERS];
    Point prevPos[NUM_PLAYERS];

    Steps* steps;
    Results* results;

protected:
    bool isRunning;
    bool gameOver;
    size_t gameCycles = 0;

    // Getters 
    bool isFinalRoom(int dest) const { return dest == static_cast<int>(screens.size()) - 1; }
    Steps* getSteps() const { return steps; }
    Results* getResults() const { return results; }

    // Setters 
    void setGame();
    void setResults(Results* r){
        if (results)
            delete results;
        results = r;
    }
    void setSteps(Steps* s) {
        if (steps)
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
    bool loadRiddles();
    bool loadRiddles(int loadRoomID);

    // --Used in Derived Classes--
    bool restartCurrentRoom();
    bool reloadRoom(int roomID);
    void showError(const std::string& msg);
    virtual void showMessage(const std::string& msg);

    bool processKey(char ch);
    bool handleRiddles(Player& player);
    virtual bool getRiddleAnswer(Riddle* riddle, bool& outSolved) = 0;
    std::vector<std::string> getScreenSourceFiles() const;
    bool isGameInFinalPhase() const {
        return playerFinished[0] || playerFinished[1];
    }
private:
    // ----- Display Functions -----
    void displayLegend(const Screen& room) const;
    void displayFinalScoreboard() const;
    void drawPlayers() const;

    int getTotalScore() const;

    // ----- Game Logic Functions -----
    void moveRoom(Player& p, int dest);
    Point getStartPoint(Player& player, int idx, int dest) const;
    bool playersCollide(int currPlayerIndex, const Point& nextPos);
    void applyLifeLoss(Player& player);

    // ----- Handle Functions -----
    void handleDoor(Player& player);
    void handleSwitch(Player& player);
    bool handleSprings(Player& p);
    void handleBombs();
    bool handleObstacles(Player& player, const Point& nextPos);
    void handleTorch(Player& player);
    void handleCollectibles(Player& player);
    bool handleTeleports(Player& player);
    bool handleDispose(Player& p);
    bool handleAcceleratedMovement(Player& player, int playerIndex);

    // ----- Helper Functions -----
    
    bool isMatchingKey(const Player& player, Screen& room, const Door* door);
    void updateDoorBySwitches(int id);
    void explodeBomb(Point center);
    Spring* findAdjacentSpring(const Point& pos);
    bool compressSpring(Player& player, Spring& sp);
    static void launchPlayer(Player& player, Spring& sp);
    int calcForce(const Player& pusher, const Obstacle* ob, Direction dir) const;
    bool canMoveObstacle(const std::vector<Point>& nextBody, const Obstacle* currOb);
    bool chainPushSuccess(int idx, Direction dir, const Point& obstaclePos);

 public:
    GameBase();
    virtual ~GameBase();

    void run();

};