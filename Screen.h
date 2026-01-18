#pragma once
#include "Utils.h"
#include "GameDefs.h"
#include "Point.h"
#include "Door.h"
#include "Key.h"
#include "Switch.h"
#include "Spring.h"
#include "Bomb.h"
#include "Obstacle.h"
#include "Torch.h"
#include "Riddle.h"
#include "Maps.h"
#include "Templates.h"
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <stdexcept>

struct LegendArea{
	Point topLeft;
	Point bottomRight;
	bool exists = false;
};

struct DarkArea {
	Point topLeft;
	Point bottomRight;
};

struct TeleportPair {
	Point p1;
	Point p2;
};

class Screen {
private:
	char board[SCREEN_WIDTH][SCREEN_HEIGHT]; // 2D buffer storing the room's characters 

	std::vector<DarkArea> darkAreas;       // Stores all predefined dark regions in the room.
	LegendArea legend;

	bool illuminated[SCREEN_WIDTH][SCREEN_HEIGHT];      // Marks which cells are currently illuminated by torches.
	bool validationMask[SCREEN_HEIGHT][SCREEN_WIDTH];  	// Used for object placement validation during screen loading
	std::string sourceFile = "";

	std::vector<Door> doors;
	std::vector<Key> keys;
	std::vector<Bomb> bombs;
	std::vector<Spring> springs;
	std::vector<Switch> switches;
	std::vector<Torch> torches;
	std::vector<Riddle> riddles;
	std::vector<Obstacle> obstacles;
	std::vector<TeleportPair> teleporters;

public:
	Screen() = default;                 // default ctor 

	void setMap(const char* map[SCREEN_HEIGHT]);
	bool loadScreenFromFile(const std::string& filename, std::string& errorMsg, std::string& warningMsg);
	bool loadMapFromFile(std::ifstream& file, const std::string& filename, std::string& errorMsg, std::string& warningMsg);
	bool buildObjectsFromBoard(std::string& errorMsg);
	void handleChar(char c, int x, int y);
	bool readDataFromFile(std::ifstream& file, const std::string& filename, std::string& errorMsg);
	bool parseDataLine(const std::string& line, std::string& error);
	bool addTeleporterPair(Point p1, Point p2, std::string& errorMsg);

	// Helpers 
	bool buildSpringsFromBoard(std::string& error);
	bool isSpringBase(int x, int y, Direction& dir) const;
	void buildObstaclesFromBoard();
	void collectObstacleDFS(int x, int y, bool visited[][SCREEN_HEIGHT], std::vector<Point>& body);
	bool isValidBoardChar(char c) const;
	bool validateLegendPlacement(std::string& errorMsg) const;
	bool validateDoors(int numRooms, std::string& errorMsg) const;
	const std::string& getSourceFile() const { return sourceFile; }

	void clearRoom();
	void resetObjects();

	void addDarkArea(const Point& topLeft, const Point& bottomRight);
	void addDoor(const Door& d) { doors.push_back(d); }
	void addKey(const Key& k) { keys.push_back(k); }
	void addBomb(const Bomb& b) { bombs.push_back(b); }
	void addSpring(const Spring& s) { springs.push_back(s); }
	void addSwitch(const Switch& sw) { switches.push_back(sw); }
	void addTorch(const Torch& t) { torches.push_back(t); }
	void addRiddle(const Riddle& r) { riddles.push_back(r); }
	void addObstacle(const Obstacle& ob);

	// Display Functions
	void drawChar(const Point& p, char c); // draws specific char at point in screen
	void erase(const Point& p);    // erases specific char from point in screen
	bool isCellFree(const Point& pos) const;
	void drawScreen();
	void drawBase();
	void drawItems();
	char charAt(const Point& p) const {   // returns the character stored at the given screen position.
		return board[p.getX()][p.getY()];
	}
	bool isWall(const Point& p) const;
	bool isItem(const Point& p) const;
	bool isDoor(const Point& p) const;
	bool isSwitch(const Point& p) const;
	bool isObstacle(const Point& p) const;
	bool isSpring(const Point& p) const;

	// Get Functions

	std::vector<Bomb>& getBombs() { return bombs; }
	const std::vector<Bomb>& getBomb() const { return bombs; }

	const std::vector<Switch>& getSwitches() const { return switches; }
	const std::vector<Spring>& getSprings() const { return springs; }

	// Get Objects Functions 
	Door* getDoorAt(const Point& p) { return getItemAt(doors, p); }
	Key* getKeyAt(const Point& p) { return getItemAt(keys, p); }
	Bomb* getBombAt(const Point& p) { return getItemAt(bombs, p); }
	Switch* getSwitchAt(const Point& p) { return getItemAt(switches, p); }
	Riddle* getRiddleAt(const Point& p) { return getItemAt(riddles, p); }
	Spring* getSpringAt(Point p);  // theres a new version
	Obstacle* getObstacleAt(Point p);

	ItemType getItemType(const Point& p) const;
	Door& getDoorById(int id);        // used in func updateDoorBySwitches
	Point getTeleportDest(const Point& p) const;

	// Helper Functions
	void collectKey(Player& player, const Point& p);
	void collectBomb(Player& player, const Point& p);
	void collectTorch(Player& player, const Point& p);

	// helps get the key\bomb\torch object from player's inventory
	Key& getStoredKey(int index) { return keys[index]; }
	Bomb& getStoredBomb(int index) { return bombs[index]; }
	Torch& getStoredTorch(int index) { return torches[index]; }

	void pushObstacle(Obstacle& ob, Direction dir);

	// Legend helpers
	void setLegendAnchor(int x, int y);
	const LegendArea& getLegend() const { return legend; }
	bool isLegendCell(const Point& p) const;
	void clearLegendAreaFromBoard();

	// Dark & Torch helpers

	bool isVisible(const Point& p) const;
	bool isInDarkArea(const Point& p) const;
	bool isIlluminated(const Point& p) const;
	void illuminateMap(const Point& center);
	void clearIllumination();

	// Bomb helpers
	bool removeObjectsAt(const Point& p);
	bool removeSpringAt(const Point& p);
	void removeObstacleAt(const Point& p);
	bool removeTeleporterAt(const Point& p);
	void removeRiddleAt(const Point& p) { removeItemAt(riddles, p); }
	bool removeBombAt(const Point& p) { return removeItemAt(bombs, p); }


};
