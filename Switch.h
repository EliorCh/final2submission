#pragma once
#include "Point.h"

//  Switch class - represents a switch that can toggle to affect door states
//  Inherits from DoorElement for common door-related functionality

class Switch : public DoorElement {
private:
	bool state;        // True = ON, False = OFF

	void updateFigure() {
		figure = state ? BOARD_SWITCH_ON : BOARD_SWITCH_OFF;
	}

public:
	enum class SwitchRule { ALL_ON, ALL_OFF, NO_RULE };  // combinations of switches to open the door:
	// Default constructor
	Switch()
		: DoorElement(Point(0, 0), -1, BOARD_SWITCH_OFF),
		  state(false) {}

	// Custom constructor
	Switch(Point _pos, int _doorID, bool _state)
		: DoorElement(_pos, _doorID, _state ? BOARD_SWITCH_ON : BOARD_SWITCH_OFF),
		  state(_state) {}

	// State management
	bool getState() const { return state; }

	// Override getFigure to return current state character
	char getFigure() const override {
		return state ? BOARD_SWITCH_ON : BOARD_SWITCH_OFF;
	}

	// Toggles the switch state
	void toggle() {
		state = !state;
		updateFigure();
	}
};
