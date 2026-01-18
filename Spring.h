#pragma once
#include "Point.h"
#include "Player.h"

class Spring {
private:
    Point basePos;      // Base position of the spring (first cell)
    char figure = '#';
    int fullSize;       // Maximum number of links the spring can extend
    int currSize;       // How many links are currently extended
    Direction dir;      // Direction in which the spring extends
    

public:
    Spring() : basePos({ 0, 0 }), fullSize(0), currSize(0), dir(UP)  // default ctor  
    {
    }
    Spring(const Point& p, int s, const Direction& d)     // custom ctor 
        : basePos(p), fullSize(s), currSize(s), dir(d) 
    {
    }

    int getCurrSize() const { return currSize; }
    int getFullSize() const { return fullSize; }
    Point getPos() { return basePos; }
    Direction getDir() const { return dir; }
    char getFigure() const { return figure; }
    Point getTipPos() const;
    Point getLinkPos(int index) const;
    bool isSpringBody(const Point& p) const;
    bool isOppositeDir(Direction playerDir) const;
    void decreaseSize(int amount = 1) { // Removes one link when compressed / more when exploded
        currSize -= amount;
        if (currSize < 0) currSize = 0;
    }    
    int springRelease();

};