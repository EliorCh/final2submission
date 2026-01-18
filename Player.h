#pragma once
#include "Utils.h"
#include "Point.h"
#include "GameDefs.h"

class Player {
private:
	char figure = '$';    // Character used to draw the player
	char arrowKeys[6] = { 0, 0, 0, 0, 0, 0 };  // Key mapping for movement and actions

	Point pos;              // Current position on the board
	Point startPos;         // Initial spawn position
	Direction dir = STAY;   // Current movement direction

	int speed = 1;         // Steps per movement action
	int accelTimer = 0;      // Remaining forced-movement duration
	Direction forcedDir = STAY;  // Direction used during acceleration

	bool isDead = false;         // True if player is in dead state
	int respawnTimer = 20;        // Countdown until respawn

	bool afterDispose = false;    // True if the player disposed an item
	int compressedLinks = 0;      // compressed links counter (spring)
	bool pushing = false;   // player movement is constrained by obstacle push
	Point teleportPos;

	int score = 0;   
	int life = 3;
	Item inventory;               // Currently held item (if any)

public:
	static const int MAX_SUB_STEPS = 10;

	// Set Functions
	void setPlayer(const Point& _pos, char _figure,const char keys[6]);
	void setPos(const Point& p) { pos = p; }
	void setStartPos(const Point& p) { startPos = pos = p; }
	void setDirection(Direction _dir) { dir = _dir; }
	void setDir(const char ch);
	void setArrowKeys(const char* arrowKeysFig);
	bool isMoveKey(char c) const;
	void setDead() { isDead = true; }
	void setDisposeFlag(bool val) { afterDispose = val; } 
	void setTeleportPos(const Point& p) { teleportPos = p; }
	bool isPushing() const { return pushing; }
	void setPushing(bool v) { pushing = v; }

	// Get Functions
	Point& getPos() { return pos; }
	const Point& getPos() const { return pos; }   // Non-const access to player's position
	Point getNextPos() const;
	char getFigure() const { return figure; }
	int getSpeed() const { return speed; }
	Direction getDir() const { return dir; }
	bool getDead() const { return isDead; }
	bool getDisposeFlag() const { return afterDispose; } 
	Point& getTeleportPos() { return teleportPos; };

	// Action Functions
	void draw() const;
	void erase() const;
	void move();
	void accel(int force, Direction spDir);

	bool isAccelerating() const { return accelTimer > 0; }
	void stopAcceleration() {
		accelTimer = 0;
		forcedDir = STAY;
		speed = 1;
	}

	void bumpedInto(Player& other) const;
	int getAccelerationSubSteps(Point subSteps[MAX_SUB_STEPS]) const;
	void tickAcceleration();
	void respawn();
	char getInventoryChar() const;

	void resetForRoom();

	// Inventory Functions
	bool inventoryEmpty() const { return inventory.type == NONE; }
	ItemType checkItem() const { return inventory.type; }
	int getIndex() const { return inventory.Index; }
	void clearInventory() // Removes any item the player carries.
	{
		inventory.type = NONE;
		inventory.Index = -1;
	}      
	void collectItem(const ItemType& newItem, const int index = -1) { // Stores a new item in player's inventory  
		inventory.type = newItem; 
		inventory.Index = index;   // remembers its source index
	}
	bool isDisposeKey(char c) const { return c == arrowKeys[DISPOSE]; }  // Returns True if player pressed the Dispose key

	// Helper Functions for Spring handling
	void addCompression() { compressedLinks++; }
	int getCompression() const { return compressedLinks; }
	void resetCompression() { compressedLinks = 0; }


	// Legend Panel Functions
	void addScore(const int i) { score += i; }
	int getScore() const { return score; }
	int getLife() const { return life; }

	bool lowerLife() { // no more set dead func
		life--;
		isDead = true;
		respawnTimer = 20;
		return life > 0;
	}



};