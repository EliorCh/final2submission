#pragma once
#include "Collectible.h"
#include "Point.h"

//  Key class - represents a collectible key that opens a specific door
//  Inherits common functionality from Collectible base class

class Key : public Collectible {
private:
	int doorID;          // Which door does the key open

public:
	// Default constructor
	Key() : Collectible(Item{ItemType::KEY, -1}, Point(0, 0), BOARD_KEY, false), doorID(-1) {}

	// Custom constructor
	Key(Point _pos, int _xDoorID, int _idx)
        : Collectible(Item{ItemType::KEY, _idx}, _pos, BOARD_KEY, true), doorID(_xDoorID) {}

	// DoorID Functions
	void setDoorId(int id) { doorID = id; }
	int getDoorID() const { return doorID; }

};


