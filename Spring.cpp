#include "Spring.h"

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
    case RIGHT: x += index; break;
    case LEFT: x -= index; break;
    case UP: y -= index; break;
    case DOWN: y += index; break;
    default: break;
    }
    return { x,y };
}

// Checks whether point p lies on the spring's extended body
bool Spring::isSpringBody(const Point& p) const {
    for (int i = 0; i < currSize; i++) {
        if (getLinkPos(i) == p) return true;
    }
    return false;
}

// True if the player is facing the spring (for compression)
bool Spring::isOppositeDir(Direction playerDir) const {
    return (Point::areOpposite(dir, playerDir));
}

// Returns the force and resets currSize to fullSize
int Spring::springRelease()
{
    int force = fullSize - currSize;   // force equals number of compressed links

    currSize = fullSize;              // restore spring length

    return force;
}

