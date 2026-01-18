#include "Obstacle.h"

// is p part of body
bool Obstacle::isObBody(const Point& p) const {
    return std::find(body.begin(), body.end(), p) != body.end();
}

// checks if it has enough force to move this obstacle
bool Obstacle::canBePushed(int force) const {
    return force >= getSize();
}


void Obstacle::move(Direction dir) {
    if (dir == STAY || dir == DISPOSE)
        return;

    for (Point& cell : body)
        cell = cell.next(dir);
}

std::vector<Point> Obstacle::getNextBody(Direction dir) const {
    std::vector<Point> result;
    result.reserve(body.size());

    for (const Point& cell : body)
        result.push_back(cell.next(dir));

    return result;
}