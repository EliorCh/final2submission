#include "Spring.h"
#include <fstream>

// Returns the position of the first empty link (the "launch pad")
Point Spring::getTipPos() const {
    if (currSize == 0) return basePos;
    return getLinkPos(currSize - 1);
}

// Returns the coordinate of spring link number 'index'
Point Spring::getLinkPos(int index) const {
    int x = basePos.getX();
    int y = basePos.getY();

    switch (dir) {
    case Direction::RIGHT:  x += index; break;
    case Direction::LEFT:   x -= index; break;
    case Direction::UP:     y -= index; break;
    case Direction::DOWN:   y += index; break;
    default: break;
    }
    return { x,y };
}

// Checks whether point p lies on the spring's extended body
bool Spring::isSpringBody(const Point& p) const {
    if (p == basePos) return true;

    for (int i = 0; i < currSize; i++) {
        if (getLinkPos(i) == p) return true;
    }
    return false;
}

// True if the player is facing the spring (for compression)
bool Spring::isOppositeDir(Direction playerDir) const {
    return (Point::areOpposite(dir, playerDir));
}

// *Logic reviewed with ChatGPT assistance*
SpringAction Spring::interact(Player& player) {
    Point playerPos = player.getPos();
    Point playerNext = playerPos.next(player.getDir());
    Direction playerDir = player.getDir();

    // Player on fully compressed spring base - launch
    if (currSize == 0 && player.getCompression() > 0) {
        if (playerPos == basePos && Point::areOpposite(playerDir, dir)) {
            return SpringAction::Launch;
        }
    }

    // Player approaching spring tip
    if (playerNext == getTipPos()) {
        if (!Point::areOpposite(playerDir, dir)) {
            return SpringAction::Blocked;
        }

        currSize--;
        return SpringAction::Compressed;
    }

    // Player already on spring body - let them exit
    if (isSpringBody(playerPos)) {
        return SpringAction::None;
    }

    // Player trying to enter spring body from side
    if (isSpringBody(playerNext)) {
        return SpringAction::Blocked;
    }

    return SpringAction::None;
}

// Returns the force and resets currSize to fullSize
int Spring::springRelease()
{
    int force = fullSize - currSize;   // force equals number of compressed links
    currSize = fullSize;              // restore spring length
    return force;
}

