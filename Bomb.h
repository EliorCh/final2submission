#pragma once
#include "Point.h"
#include <vector>

class Bomb {
private:
    Point pos;               // Position of the bomb of the board
    char figure = '@'; 
    int timer = 5;           // Countdown until explosion
    bool active;             // True if the bomb exists on the board
    bool ticking = false;    // True if the timer is currently decreasin

public:
    Bomb() : pos(0, 0), figure('@'), timer(5), active(false), ticking(false)   // default ctor 
    {
    }
    explicit Bomb(Point _pos)                    // custom ctor     
        : pos(_pos), timer(5), active(true), ticking(false)
    {
    }    

    // Get / Set Functions
    void setPos(const Point& p) { pos = p; }
    Point getPos() const { return pos; }
    char getFigure() const { return figure; }

    void activate() { active = true; }
    void deactivate() { active = false; }
    bool isActive() const { return active; }
    bool isTicking() const { return ticking; }
    void setTicking() { ticking = true; }
    void arm(const Point& p);
    bool tick();
    static std::vector<std::vector<Point>> getBlastPattern(Point center, int radius);
};
