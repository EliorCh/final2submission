#pragma once
#include "GameDefs.h"

// Represents a 2D position on the screen/board.
class Point
{
private:
	int x, y;   // coordinates

public:
	Point() { x = y = 0; };

	Point(const int _x, const int _y) : x(_x), y(_y) {};

	bool operator==(const Point& other) const {
		return x == other.x && y == other.y;
	}
	bool operator!=(const Point& other) const {
		return !(*this == other);
	}
	int getX() const { return x; }
	int getY() const { return y; }

	bool operator<(const Point& other) const {
		if (y != other.y) return y < other.y;
		return x < other.x;
	}

	bool isAt(const Point& p) const {   // Comparing positions
		return x == p.getX() &&
			y == p.getY();
	}

	Point next(Direction dir) const;             // Returns the next position when moving one step in the given direction.
	static bool checkLimits(const Point& p);     // Checks whether a point is inside the screen limits. 
	static bool areOpposite(Direction d1, Direction d2);   // Returns true if two directions are opposite to each other.
	static Direction opposite(Direction dir);

	
};
