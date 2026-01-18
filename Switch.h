#pragma once
#include "Point.h"
#include "Door.h"

class Switch
{
private:
	Point pos;         // Position of the switch on the board  
	int doorID;        // The door this switch is linked to
	bool state;        // True = ON, False = OFF
	char figure;       // Character drawn on screen ('/' or 'o')

public:
	Switch(): pos(0, 0), doorID(-1), state(false),figure('o')     // default ctor 
	{
	}
	Switch(Point _pos, int _doorID, bool _state)      // custom ctor     
		: pos(_pos), doorID(_doorID), state(_state)
	{
		figure = _state ? '/' : 'o';
	}

	void setDoorId(int id) { doorID = id; }

	// Get Functions
	Point getPos() const { return pos; }
	int getDoorID() const { return doorID; }
	bool getState() const { return state; }
	char getFigure() const { return state ? '/' : 'o'; }\

	void toggle()  // Toggles the switch state. If it was ON, it becomes OFF, and vice-versa
	{
		state = !state;
	}

};
