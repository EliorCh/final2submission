#pragma once
#include "Point.h"
#include "Collectible.h"

class Torch : public Collectible {
public:
	// Default constructor
	Torch() : Collectible(Item{ItemType::TORCH, -1},Point(0, 0), BOARD_TORCH, false) {}   // default ctor {}

	// Custom constructor
	explicit Torch(Point _pos, int _idx)
		: Collectible(Item{ItemType::TORCH, _idx}, _pos, BOARD_TORCH, true) {}
};