#include "Player.h"
#include <iostream>

// Set Functions
void Player::initForNewGame() {
	life = 3;
	score = 0;
	currRoomID = -1;
	roomsDoneCount = 0;
	finishedGame = false;
	prevPos = Point(0,0);
	clearInventory();

	resetState();
}

void Player::resetState() {
	dir = Direction::STAY;
	forcedDir = Direction::STAY;

	speed = 1;
	accelTimer = 0;
	pushing = false;

	isDead = false;
	respawnTimer = 5;
	prevPos = pos = startPos;
}

void Player::setControls(char _figure, const char _keys[6]) {
	figure = _figure;
	setArrowKeys(_keys);
}

// Copies the 6 control keys into local array.
void Player::setArrowKeys(const char* arrowKeysFig)
{
	arrowKeys[static_cast<int>(Direction::RIGHT)]   = arrowKeysFig[0];
	arrowKeys[static_cast<int>(Direction::DOWN)]    = arrowKeysFig[1];
	arrowKeys[static_cast<int>(Direction::LEFT)]    = arrowKeysFig[2];
	arrowKeys[static_cast<int>(Direction::UP)]      = arrowKeysFig[3];
	arrowKeys[static_cast<int>(Direction::STAY)]    = arrowKeysFig[4];
	arrowKeys[static_cast<int>(Direction::DISPOSE)] = arrowKeysFig[5];
}

// Returns true if character matches one of the movement keys.
bool Player::isMoveKey(char c) const
{
	for (Direction d : { Direction::RIGHT, Direction::LEFT, Direction::UP, Direction::DOWN, Direction::STAY })
	{
		if (c == arrowKeys[static_cast<int>(d)])
			return true;
	}
	return false;
}

// Converts the input key into a movement direction
void Player::setDir(const char ch) 
{
	Direction requestDir = Direction::STAY;
	bool found = false;

	for (Direction d : { Direction::RIGHT, Direction::LEFT, Direction::UP, Direction::DOWN, Direction::STAY })
	{
		if (ch == arrowKeys[static_cast<int>(d)])
		{
			requestDir = d;
			found = true;
			break;
		}
	}

	if (!found) return;

	if (accelTimer > 0)
	{
		// STAY is forbidden
		if (requestDir == Direction::STAY)
			return;

		// Opposite direction is forbidden
		if (Point::areOpposite(requestDir, forcedDir))
			return;

		// only side move and forced direction is allowed
		if (requestDir == forcedDir)
			return;
	}

	dir = requestDir;
}

// Computes the next position based on forced direction and current movement direction.
Point Player::getNextPos() const
{
	Point next = pos;

	// forced movement (spring launch)
	if (accelTimer > 0 && forcedDir != Direction::STAY)
	{
		next = next.next(forcedDir);
	}

	// sideways movement allowed (but not opposite)
	if (accelTimer > 0)
	{
		if (dir != Direction::STAY && !Point::areOpposite(dir, forcedDir))
			next = next.next(dir);
	}
	else
	{
		// normal movement when not accelerating
		if (dir != Direction::STAY)
			next = next.next(dir);
	}

	return next;
}

// Action Functions

void Player::draw() const
{
	Utils::gotoxy(pos.getX(), pos.getY());
	std::cout << figure;
}

void Player::erase() const
{
	Utils::gotoxy(pos.getX(), pos.getY());
	std::cout << ' ';
}

void Player::move()
{
	Point next = getNextPos();

	if (next != pos)
		afterDispose = false;

	pos = next;
}

void Player::accel(int force, Direction spDir) {  // Sets forced movement

	speed = force;       // speed boost and direction for several steps.
	accelTimer = force * force;
	forcedDir = spDir;
	dir = spDir;

}

// Handles collision between players.
void Player::bumpedInto(Player& other) const {// *Logic reviewed with ChatGPT assistance*
	// If the moving player is under spring acceleration:
	if (this->isAccelerating())
	{                              // transfer the same speed, direction and acceleration to the other player.
		other.speed = this->speed;  
		other.accelTimer = this->accelTimer;
		other.forcedDir = this->forcedDir;
		other.dir = this->forcedDir;   // ignore previous direction

		// The moving player continues normally.
		return;
	}
}


int Player::getAccelerationSubSteps(Point subSteps[MAX_SUB_STEPS]) const
{
	int count = 0;

	// 1) forced movement (always first)
	if (accelTimer > 0 && forcedDir != Direction::STAY)
	{
		subSteps[count] = pos.next(forcedDir);
		count++;
	}

	// 2) sideways movement (if allowed)
	if (accelTimer > 0)
	{
		if (dir != Direction::STAY && !Point::areOpposite(dir, forcedDir))
		{
			Point afterSide = subSteps[count - 1].next(dir);
			subSteps[count] = afterSide;
			count++;
		}
	}

	return count; // can be 0, 1 or 2
}

void Player::tickAcceleration()
{
	if (accelTimer > 0)
	{
		accelTimer--;
		if (accelTimer == 0)
		{
			speed = 1;
			forcedDir = Direction::STAY;
		}
	}
}

// Counts down respawn timer; restores player to start position when finished.
void Player::respawn() {
	if (isDead) {
		if (respawnTimer > 0) {
			respawnTimer--;
			return;
		}

		pos = startPos;
		isDead = false;
		respawnTimer = 20;
	}
}

void Player::resetForRoom()
{
	pos = startPos;  
	dir = Direction::STAY;

	speed = 1;       
	accelTimer = 0;      
	forcedDir = Direction::STAY;

	isDead = false;     
	respawnTimer = 5;   
	pushing = false;

	clearInventory();

	afterDispose = false;   
}

