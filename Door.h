#pragma once
#include "DoorElement.h"

// SwitchRule - Defines the switch condition required to open a door
enum class SwitchRule {
	ALL_ON,   // All linked switches must be ON
	ALL_OFF,  // All linked switches must be OFF
	NO_RULE   // No switch requirement
};

// Door class - represents a door that can be opened with keys and/or switches
// Inherits from DoorElement for common door-related functionality
class Door : public DoorElement {
private:
	int destRoom;          // ID of the room the door leads to
	bool isOpen;     // Indicates whether the door is open
	int neededKeys;        // Number of keys required to open the door

	SwitchRule rule;       // Type of switch interaction required (if any)
	bool keyOK;            // True if all required keys have been used
	bool switchOK;         // True if switch condition is satisfied

	void updateFigure() {
		figure = static_cast<char>(DIGIT_ZERO + destRoom);
	}

public:
	// Default constructor
	Door() : DoorElement(Point(0, 0), 0, DIGIT_ZERO),
	destRoom(0), isOpen(false), neededKeys(0),
		rule(SwitchRule::NO_RULE), keyOK(false),switchOK(false) {}

	// Full custom constructor
	Door(Point _pos, int _doorID, int _destRoom, int _neededKeys, SwitchRule _rule, bool _keyOK, bool _switchOK)
        : DoorElement(_pos, _doorID, static_cast<char>(DIGIT_ZERO + _destRoom)),
          destRoom(_destRoom), neededKeys(_neededKeys),
          rule(_rule), keyOK(_keyOK), switchOK(_switchOK) {
		isOpen = false;
	}

	// Simple constructor (from board parsing)
	Door(Point _pos, int _dest)
	: DoorElement(_pos, 0, static_cast<char>(DIGIT_ZERO + _dest)),
	  destRoom(_dest), isOpen(false), neededKeys(0),
	  rule(SwitchRule::NO_RULE), keyOK(false), switchOK(false) {}

	// Set Functions
	// Set flags
	void setFlags(bool _keyOK, bool _switchOK)
	{
		keyOK = _keyOK;           // keyOK: true if the player has used all required keys.
		switchOK = _switchOK;     // switchOK: true if the required switch is active.
	}

	// Apply rules from screen file
	void applyRules(int id, int keys, bool open, int ruleCode) {
		doorID = id;
		neededKeys = keys;
		isOpen = open;

		if (neededKeys == 0)
			keyOK = true;

		// Translate int -> enum class
		switch (ruleCode) {
			case 0:
				rule = SwitchRule::ALL_ON;
				break;
			case 1:
				rule = SwitchRule::ALL_OFF;
				break;
			default:
				rule = SwitchRule::NO_RULE;
				switchOK = true;
				break;
		}
	}

	// Get Functions
	int getDestination() const { return destRoom; }
	int getNeededKeys() const { return neededKeys; }
	SwitchRule getRule() const { return rule; }
	bool checkIsOpen() const { return isOpen; }
	bool getKeyStatus() const { return keyOK; }
	bool getSwitchStatus() const { return switchOK; }
	bool needsKey() const { return getNeededKeys() > 0; }

	// Update Functions
	void updateKeyOK() { keyOK = true; }			// all keys for the door have been used
	void updateSwitchOK(bool ok) {switchOK = ok;}   // Changes switch flag on/off when toggles
	void open() { isOpen = true; }					// Opens the door once all requirements are met
	void useKey() { neededKeys--; }					// Decreases the number of required keys by one.

};
