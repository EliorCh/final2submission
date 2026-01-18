#pragma once
#include "Point.h"
#include "GameDefs.h"

class Door {
private:
	Point pos;             // Position on the board
	int doorID;            // Logical ID used to match keys and switches to this door
	int destRoom;          // ID of the room the door leads to
	bool isOpen=false;     // Indicates whether the door is open   
	int neededKeys;        // Number of keys required to open the door
	char figure;

	SwitchRule rule;       // Type of switch interaction required (if any)
	bool keyOK;            // True if all required keys have been used
	bool switchOK;         // True if switch condition is satisfied

public:
	Door() : pos(0, 0), doorID(0), destRoom(0), isOpen(false), neededKeys(0),  // default ctor 
		figure('0'), rule(NO_RULE), keyOK(false),switchOK(false)
	{
	}
	Door(Point _pos, int _doorID, int _destRoom, int _neededKeys, SwitchRule _rule,bool _keyOK, bool _switchOK) :      // custom ctor 
	pos(_pos), doorID(_doorID), destRoom(_destRoom), neededKeys(_neededKeys), rule(_rule), keyOK(_keyOK), switchOK(_switchOK)
	{
		figure = '0' + destRoom;
	}
	Door(Point _pos, int _dest) : pos(_pos), doorID(0), destRoom(_dest), isOpen(false), neededKeys(0),
		figure('0' + destRoom), rule(NO_RULE), keyOK(false), switchOK(false)
	{
	}
	// Set Functions
	void setFlags(bool _keyOK, bool _switchOK)
	{
		keyOK = _keyOK;           // keyOK: true if the player has used all required keys.
		switchOK = _switchOK;     // switchOK: true if the required switch is active.
	}

	void applyRules(int id, int keys, int ruleCode)
	{
		doorID = id;
		neededKeys = keys;
		if (neededKeys == 0)
			keyOK = true;

		// translate int -> enum
		switch (ruleCode)
		{
		case 0:
			rule = ALL_ON;
			break;
		case 1:
			rule = ALL_OFF;
			break;
		case 2:
			rule = NO_RULE;
			switchOK = true;		
			break;
		default:
			break;
		}

		if (keyOK && switchOK){
			isOpen = true;
		}
	}

	// Get Functions
	Point getPos() const { return pos; }
	int getDoorID() const { return doorID; }
	int getDestination() const { return destRoom; }
	int getNeededKeys() const { return neededKeys; }
	int getRule() const { return rule; }
	bool checkIsOpen() const { return isOpen; }
	bool getKeyStatus() const { return keyOK; }
	bool getSwitchStatus() const { return switchOK; }
	char getFigure() const { return figure; }

	// Update Functions
	void updateKeyOK() {           // Changes key flag to true meaning all keys for the door have been used
		keyOK = true;
	}
	void updateSwitchOK(bool ok) {      // Changes switch flag - can change back to false since you can turn a switch back off\on
		switchOK = ok;
	}
	void open() { isOpen = true; }          // Opens the door once all requirements are met
	void useKey() { neededKeys--; }           // Decreases the number of required keys by one.


};
