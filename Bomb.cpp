#include "Bomb.h"
#include "Screen.h"

// Arms the bomb at a new position and restarts the timer.
// The bomb becomes active and ticking immediately.
void Bomb::arm(const Point& p)
{
    pos = p;
    timer = BOMB_INITIAL_TIMER;
    active = true;
    ticking = true;
}

bool Bomb::tick() {
    if (!active || !ticking)
        return false;         // If the bomb is not active or not ticking, no countdown occurs.

    if (timer > 0) {          // Advances the countdown by one step.
        timer--;
        return (timer == 0);  // Returns true only when the timer reaches zero.
    }
    return false;          
}

void Bomb::manageBombs(std::vector<Bomb>& bombs, Screen& room, Ray& dangerZone) {
    for (auto it = bombs.begin(); it != bombs.end(); ) {
        if (it->tick()) {
            Ray currDanger = it->explode(room);
            dangerZone.insert(dangerZone.end(), currDanger.begin(), currDanger.end());
            room.erasePoint(it->getPos());
            it = bombs.erase(it);
        } else {
            ++it;
        }
    }
}


// *Developed with ChatGPT assistance*
Ray Bomb::explode(Screen& room) const {
    Ray affectedPoints;
    Point center = getPos();
    room.removeBombAt(center);

    // getting all points that are in the bomb explosion area
    // the points are in order: from the center - out
    BlastPattern Pattern = Bomb::getBlastPattern(center, BOMB_BLAST_RADIUS);

    for (const auto& ray : Pattern) {
        for (size_t i = 0; i < ray.size(); i++) {
            Point p = ray[i];

            if (!Point::checkLimits(p)) break; // the point and those after it in this ray are out of limits
            char c = room.charAt(p);

            if (c == BOARD_WALL || c == WALL_HORIZ || c == WALL_VERT) {
                if (i == 0 && (c == WALL_HORIZ || c == WALL_VERT)) { // inner circle
                    // some barriers ('=' or '|') can be destroyed is those are adjacent to bomb
                    room.erasePoint(p);
                    affectedPoints.push_back(p);
                }
                break; // blocked by wall
            }
            if (room.removeBombAt(p)) { // chain explosion
                Bomb chainedBomb(p);
                Ray chainDanger=chainedBomb.explode(room);
                affectedPoints.insert(affectedPoints.end(), chainDanger.begin(), chainDanger.end());
            }

            room.erasePoint(p);
            room.removeObjectsAt(p);
            affectedPoints.push_back(p);
        }
    }
    return affectedPoints;
}

BlastPattern Bomb::getBlastPattern(Point center, int radius) {
    BlastPattern allRays;
    allRays.reserve(9); // saving place in advance for all rays

    Ray centerPoint;
    centerPoint.emplace_back(center);
    allRays.push_back(std::move(centerPoint)); // center point

    static constexpr int directions[8][2] = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0},
        {-1, -1}, {1, -1}, {-1, 1}, {1, 1}
    };

    for (auto& dir : directions) {
        Ray currentRay;
        currentRay.reserve(radius); // saving place in advance for the exact number of points in each ray
        for (int i = 1; i <= radius; i++) {
            int pX = center.getX() + (dir[0] * i);
            int pY = center.getY() + (dir[1] * i);
            currentRay.emplace_back(pX, pY);
        }
        allRays.push_back(std::move(currentRay)); // move ctr instead of deep copy
    }
    return allRays;
}