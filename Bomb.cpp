#include "Bomb.h"

// Arms the bomb at a new position and restarts the timer.
// The bomb becomes active and ticking immediately.
void Bomb::arm(const Point& p)
{
    pos = p;
    timer = 5;     
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

std::vector<std::vector<Point>> Bomb::getBlastPattern(Point center, int radius) {
    std::vector<std::vector<Point>> allRays;
    allRays.reserve(9); // saving place in advance for all rays

    std::vector<Point> centerPoint;
    centerPoint.emplace_back(center);
    allRays.push_back(std::move(centerPoint)); // center point

    int directions[8][2] = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0},
        {-1, -1}, {1, -1}, {-1, 1}, {1, 1}
    };

    for (auto& dir : directions) {
        std::vector<Point> currentRay;
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