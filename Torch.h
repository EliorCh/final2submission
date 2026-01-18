#pragma once
#include "Point.h"

class Torch{
private:
	Point pos;          // Position on the board
	char figure = '!';
	bool active;        // True if torch is on board

public:
	Torch() : pos(0, 0), figure('!'), active(false)   // default ctor 
	{
	}
	Torch(Point _pos)                // custom ctor 
		: pos(_pos), active(true)
	{
	}

	// Get Functions
	Point getPos() const { return pos; }
	char getFigure() const { return figure; }
	bool isActive() const { return active; }
	void setPos(Point pos) { this->pos = pos; }

	void activate() { active = true; }          // Marks the torch as available (visible on screen).
	void deactivate() { active = false; }       // Marks the torch as collected (no longer visible).

};

