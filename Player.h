#pragma once
#include "Collectible.h"
#include "Point.h"
#include "Utils.h"
#include "GameDefs.h"

// Player Constants
constexpr int PLAYER_INITIAL_LIVES = 3;
constexpr int PLAYER_RESPAWN_TIMER = 20;
constexpr int PLAYER_DEFAULT_SPEED = 1;

class Player {
private:
	char figure;					   // Character used to draw the player
	char arrowKeys[6] = { 0, 0, 0, 0, 0, 0 };  // Key mapping for movement and actions

	Point pos;					       // Current position on the board
	Point startPos;					   // Initial spawn position
	Direction dir = Direction::STAY;   // Current movement direction

	int currRoomID{};
	int roomsDoneCount{};;
	bool finishedGame{};
	Point prevPos;

	int speed = PLAYER_DEFAULT_SPEED;       // Steps per movement action
	int accelTimer = 0;						// Remaining forced-movement duration
	Direction forcedDir = Direction::STAY;  // Direction used during acceleration

	bool isDead = false;							// True if player is in dead state
	int respawnTimer = PLAYER_RESPAWN_TIMER;        // Countdown until respawn

	bool afterDispose = false;    // True if the player disposed an item
	int compressedLinks = 0;      // compressed links counter (spring)
	bool pushing = false;   // player movement is constrained by obstacle push
	Point teleportPos;

	int score = 0;   
	int life = PLAYER_INITIAL_LIVES;
	Item inventory;               // Currently held item (if any)

public:
	static constexpr auto MAX_SUB_STEPS = 10;

	Player() : figure(' '), arrowKeys{0} {
		initForNewGame();
	}
	// initializing Functions
	void initForNewGame();
	void setControls(char _figure, const char _keys[6]);
	void setArrowKeys(const char* arrowKeysFig);
	void resetForRoom();
	void resetState();

	// Set Functions
	void setStartPos(const Point& p) { prevPos = startPos = pos = p; }
	void setPos(const Point& p) { pos = p; }
	void setPrevPos(const Point& p) { prevPos = p; }

	void setPushing(bool v) { pushing = v; }
	void setDisposeFlag(bool val) { afterDispose = val; }
	void setTeleportPos(const Point& p) { teleportPos = p; }

	void setDirection(Direction _dir) { dir = _dir; }
	void setDir(char ch);

	void setRoomID(int id) { currRoomID = id; }
	void setDead() { isDead = true; }
	void setFinished(bool state) { finishedGame = state; }

	void incrementRoomsDone() { roomsDoneCount++; }

	// Get Functions
	bool isMoveKey(char c) const;
	bool isPushing() const { return pushing; }
	char getFigure() const { return figure; }

	Point& getPos() { return pos; }
	const Point& getPos() const { return pos; }   // Non-const access to player's position
	Point getNextPos() const;
	Point getPrevPos() const { return prevPos; }
	Direction getForcedDir() const { return forcedDir; }

	int getSpeed() const { return speed; }
	Direction getDir() const { return dir; }
	bool getDisposeFlag() const { return afterDispose; }

	bool getDead() const { return isDead; }
	Point& getTeleportPos() { return teleportPos; };

	int getRoomID() const { return currRoomID; }
	int getRoomsDone() const { return roomsDoneCount; }
	bool isFinished() const { return finishedGame; }

	// Action Functions
	void draw() const;
	void erase() const;
	void move();
	void accel(int force, Direction spDir);

	bool isAccelerating() const { return accelTimer > 0; }
	void stopAcceleration() {
		accelTimer = 0;
		forcedDir = Direction::STAY;
		speed = PLAYER_DEFAULT_SPEED;
	}

	void bumpedInto(Player& other) const;
	int getAccelerationSubSteps(Point subSteps[MAX_SUB_STEPS]) const;
	void tickAcceleration();
	void respawn();

	// Inventory Functions
	bool inventoryEmpty() const { return inventory.type == ItemType::NONE; }
	const Item& getInventory() const { return inventory; }
	ItemType checkItem() const { return inventory.type; }
	int getIndex() const { return inventory.Index; }
	void clearInventory() // Removes any item the player carries.
	{
		inventory.type = ItemType::NONE;
		inventory.Index = -1;
	}      
	void collectItem(const Item& newItem) { // Stores a new item in player's inventory
		inventory = newItem;
	}
	bool isDisposeKey(char c) const { // Returns True if player pressed the Dispose key
		return c == arrowKeys[static_cast<int>(Direction::DISPOSE)];
	}

	// Helper Functions for Spring handling
	void addCompression() { compressedLinks++; }
	int getCompression() const { return compressedLinks; }
	void resetCompression() { compressedLinks = 0; }

	// Legend Panel Functions
	void addScore(const int i) { score += i; }
	int getScore() const { return score; }
	int getLife() const { return life; }

	bool lowerLife() {
		life--;
		isDead = true;
		respawnTimer = PLAYER_RESPAWN_TIMER;
		return life > 0; // does the player has more lives
	}
};