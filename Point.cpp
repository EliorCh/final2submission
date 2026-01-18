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
	if (d1 == Direction::UP		&& d2 == Direction::DOWN )	return true;
	if (d1 == Direction::DOWN	&& d2 == Direction::UP)		return true;
	if (d1 == Direction::RIGHT  && d2 == Direction::LEFT)	return true;
	if (d1 == Direction::LEFT	&& d2 == Direction::RIGHT)	return true;
	return false;
}

Direction Point::opposite(Direction dir)
{
	switch (dir)
	{
	case Direction::RIGHT: return Direction::LEFT;
	case Direction::LEFT:  return Direction::RIGHT;
	case Direction::UP:    return Direction::DOWN;
	case Direction::DOWN:  return Direction::UP;
	default:    return Direction::STAY;
	}
}

// Computes a new point one step away from the current position,
// based on the given direction (up/down/left/right).
Point Point::next(Direction dir) const {
	Point nextPoint = *this;
	switch (dir) {
	case Direction::RIGHT: nextPoint.x++; break;
	case Direction::LEFT: nextPoint.x--; break;
	case Direction::UP: nextPoint.y--; break;
	case Direction::DOWN: nextPoint.y++; break;
	default: break;
	}
	return nextPoint;
}