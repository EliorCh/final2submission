#include "Player.h"

bool Point::checkLimits(const Point& p)
{
	int x = p.getX();
	int y = p.getY();

	// Returns true if the given point lies inside the screen boundaries.
	return (x >= 0 && x < SCREEN_WIDTH &&
		y >= 0 && y < SCREEN_HEIGHT);
}

// Returns true if the two directions are opposite (UP vs DOWN, LEFT vs RIGHT).
bool Point::areOpposite(Direction d1, Direction d2) {
	if (d1 == UP && d2 == DOWN ) return true;
	if (d1 == DOWN && d2 == UP) return true;
	if (d1 == RIGHT && d2 == LEFT) return true;
	if (d1 == LEFT && d2 == RIGHT) return true;
	return false;
}

Direction Point::opposite(Direction dir)
{
	switch (dir)
	{
	case RIGHT: return LEFT;
	case LEFT:  return RIGHT;
	case UP:    return DOWN;
	case DOWN:  return UP;
	default:    return STAY;
	}
}

// Computes a new point one step away from the current position,
// based on the given direction (up/down/left/right).
Point Point::next(Direction dir) const {
	Point nextPoint = *this;
	switch (dir) {
	case RIGHT: nextPoint.x++; break;
	case LEFT: nextPoint.x--; break;
	case UP: nextPoint.y--; break;
	case DOWN: nextPoint.y++; break;
	default: break;
	}
	return nextPoint;
}