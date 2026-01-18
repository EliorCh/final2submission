#pragma once
#include "Point.h"
#include "BoardChars.h"

// Base class for door-related elements (Door, Switch)
// Provides common position and door linkage

class DoorElement {
protected:
    Point pos;
    int doorID;    // The door this element is linked to
    char figure;

public:
    DoorElement() : pos(0, 0), doorID(-1), figure(' ') {}
    DoorElement(Point _pos, int _doorID, char _figure)
        : pos(_pos), doorID(_doorID), figure(_figure) {}

    virtual ~DoorElement() = default;

    // Position
    Point getPos() const { return pos; }
    
    // Door linkage
    int getDoorID() const { return doorID; }
    void setDoorID(int id) { doorID = id; }
    
    // Display character
    virtual char getFigure() const { return figure; }
};
