#pragma once
#include "Point.h"
#include <vector>
#include "Collectible.h"

// Bomb Constants
constexpr int BOMB_BLAST_RADIUS = 3;
constexpr int BOMB_INITIAL_TIMER = 5;
class Screen;
using Ray = std::vector<Point>;
using BlastPattern = std::vector<Ray>;


//  Bomb class - represents a collectible/placeable bomb with explosion mechanics
//  Inherits common functionality from Collectible base class

class Bomb : public Collectible {
private:
    int timer;                  // Countdown until explosion
    bool ticking;               // True if the timer is currently decreasing

public:
    // Default constructor
    Bomb() : Collectible(Item{ItemType::BOMB, -1},Point(0, 0), BOARD_BOMB, false),
    timer(BOMB_INITIAL_TIMER),
    ticking(false) {}

    // Constructor for temp bombs
    explicit Bomb(Point _pos)
    : Collectible(Item{ItemType::BOMB, -1},_pos, BOARD_BOMB, true),
      timer(BOMB_INITIAL_TIMER),
      ticking(false) {}

    // Custom constructor
    Bomb(Point _pos, int _idx)
    : Collectible(Item{ItemType::BOMB, _idx},_pos, BOARD_BOMB, true),
      timer(BOMB_INITIAL_TIMER),
      ticking(false) {}

    // Timer and ticking state
    bool isTicking() const { return ticking; }
    void setTicking() { ticking = true; }
    bool canPickUp() const override { return !ticking; }

    // Bomb actions
    void dispose(const Point& p) override { arm(p);}
    void arm(const Point& p);
    bool tick();

    // Static bomb management
    static void manageBombs(std::vector<Bomb>& bombs, Screen& room, Ray& dangerZone);

    // Explosion mechanics
    Ray explode(Screen& room) const;
    static BlastPattern getBlastPattern(Point center, int radius);

};
