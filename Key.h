#pragma once
#include "Point.h"

class Key {
private:
	Point pos;           // Position on the board
	int DoorID;          // Which door does the key open
	char figure = 'K';
	bool active;         // True if key is on board
public:
	Key() : pos(0, 0), DoorID(-1), figure('K'), active(false)   // default ctor 
	{
	}

	Key(Point _pos, int _DoorID)                // custom ctor 
		: pos(_pos), DoorID(_DoorID), active(true)
	{
	}

	// Set Functions
	void setPos(Point pos) {
		this->pos = pos;
	}
	void setDoorId(int id) { DoorID = id; }

	// Get Functions
	bool isActive() const { return active; }
	Point getPos() const { return pos; }
	int getDoorID() const { return DoorID; }
	char getFigure() const { return figure; }
	
	void activate() { active = true; }          // Marks the key as available (visible on screen).
	void deactivate() { active = false; }       // Marks the key as collected (no longer visible).
};


