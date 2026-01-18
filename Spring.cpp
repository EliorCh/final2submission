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

SpringAction Spring::interact(Player& p) {
    Direction pDir = p.getDir();
    Direction spDir = this->dir;
    Point nextPos = p.getPos().next(pDir);

    int compressed = p.getCompression();

    if (compressed > 0) {
        if (pDir != spDir && pDir != Point::opposite(spDir) && pDir != Direction::STAY) {
            return SpringAction::Blocked; // preventing side movement while in spring
        }
        if (pDir == spDir) {
            return SpringAction::Launch;  // player stopped compressing and moving with spring's dir
        }
    }

    if (nextPos == getTipPos() && Point::areOpposite(pDir, spDir)) {
        if (currSize > 0) {
            decreaseSize();
            return SpringAction::Compressed; // one link compressed
        }
        return SpringAction::Launch;         // currSize==0 => launch!
    }

    if (isSpringBody(nextPos)) {
        return SpringAction::Blocked;        // not compressing and trying to get onto spring
    }

    if (isSpringBody(nextPos) || nextPos == getTipPos()) {
        return SpringAction::Blocked;        // touching the spring without compressing
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

