#pragma once
#include "Point.h"
#include "BoardChars.h"

// Base class for all collectible items (Key, Bomb, Torch).
// Provides common position, figure, and active state management.

// ItemType - Types of collectible items a player can hold
enum class ItemType {
    NONE,
    KEY,
    BOMB,
    TORCH
};

struct Item {
    ItemType type = ItemType::NONE;
    int Index = -1;
};

// ItemTypeUtils - Static utility class for ItemType operations
class ItemTypeUtils {
public:
    static char toChar(ItemType type) {
        switch (type) {
            case ItemType::KEY:   return BOARD_KEY;
            case ItemType::BOMB:  return BOARD_BOMB;
            case ItemType::TORCH: return BOARD_TORCH;
            default:              return EMPTY_CELL;
        }
    }
};

class Collectible {
protected:
    Item id;
    Point pos;
    char figure;
    bool active;

public:
    Collectible() : pos(0, 0), figure(' '), active(false) {}
    Collectible(Item _id, Point _pos, char _figure, bool _active = true)
        :  id(_id), pos(_pos), figure(_figure), active(_active) {}

    virtual ~Collectible() = default;

    // Get
    Item getItemID() const { return id; }

    // Position management
    void setPos(const Point& p) { pos = p; }
    Point getPos() const { return pos; }

    // Figure/display character
    char getFigure() const { return figure; }

    // Active state management
    bool isActive() const { return active; }
    void activate() { active = true; }
    void deactivate() { active = false; }
    virtual void dispose(const Point& p) {
        pos = p;
        activate();
    }

    virtual bool canPickUp() const { return true; }
};
