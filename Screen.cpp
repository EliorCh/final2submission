#include "Screen.h"
#include "Templates.h"
#include <iostream>
#include <sstream>

// LEGEND
void LegendArea::drawLegend(const std::vector<Player>& players) const {
	if (!exists) return;

	int x0 = topLeft.getX();
	int y0 = topLeft.getY();

	auto print = [x0, y0](int relativeX, int relativeY, const std::string& text) {
		Utils::print(x0 + relativeX, y0 + relativeY, text);
	};

	// 1. Clear entire legend area
	for (int y = 0; y < LEGEND_HEIGHT; ++y) {
		print(0, y, std::string(LEGEND_WIDTH, ' '));
	}

	// 2. Draw frame
	// Top border + bottom border
	std::string horizontalEdge = std::string(1, LEGEND_CORNER) +
								 std::string(LEGEND_WIDTH - 2, LEGEND_H_BORDER) +
								 std::string(1, LEGEND_CORNER);
	print(0, 0, horizontalEdge);
	print(0, LEGEND_HEIGHT - 1, horizontalEdge);

	// Side borders
	for (int y = 1; y < LEGEND_HEIGHT - 1; ++y) {
		print(0, y, std::string(1, LEGEND_V_BORDER)); // left
		print(LEGEND_WIDTH - 1, y, std::string(1, LEGEND_V_BORDER)); // right
	}

	// 3. Draw content (inside frame)
	int contentX = 1;
	int contentY = 1;
	// --- Header ---
	print(contentX+3, contentY, "SCORE  LIVES  INV");

	// Line 2
	for (const auto& player : players) {
		int idx = static_cast<int>(&player - &players[0]);
		int rowY = contentY + 1 + idx; // different line to each player

		// player's name & score
		std::string nameScore = "P" + std::to_string(idx + 1) + ": " + std::to_string(player.getScore());
		print(contentX, rowY, nameScore);

		// life
		std::string hearts; // empty char
		for (int j = 0; j < player.getLife(); ++j) hearts += "<3 ";
		print(9, rowY, hearts);

		// inventory
		char invChar = ItemTypeUtils::toChar(player.getInventory().type);
		std::string invStr = (invChar == ' ' ? "-" : std::string(1, invChar));
		print(19, rowY, invStr);
	}
}

// SCREEN
// Init Functions
void Screen::setMap(const char* map[SCREEN_HEIGHT])
{
	// creating board for constant screens (menu, final etc..)
	for (int r = 0; r < SCREEN_HEIGHT; r++)
		for (int c = 0; c < SCREEN_WIDTH; c++)
			board[c][r] = map[r][c];
}

/*
 * Loads a full screen definition from a file
 * Handles map loading, object creation, and rule parsing
 * Returns false if a fatal error occurs
 */
bool Screen::loadScreenFromFile(const std::string& filename, std::string& errorMsg, std::string& warningMsg)
{
	std::ifstream file(filename);
	if (!file.is_open())        // File could not be opened
	{
		errorMsg = "Cannot open screen file: " + filename;
		return false;
	}
	// Load the board layout (map area)
	if (!loadMapFromFile(file, filename, errorMsg, warningMsg))
	    return false;
	// Create game objects according to board characters
	if (!buildObjectsFromBoard(errorMsg))
		return false;
	// Read additional rules and data
	if (!readDataFromFile(file, filename, errorMsg))
		return false;

	return true;
}

bool Screen::loadMapFromFile(std::ifstream& file, const std::string& filename, std::string& errorMsg ,std::string& warningMsg)
{
	// Reads the screen map from file and fills the board array.
	std::string line;
	for (int y = 0; y < SCREEN_HEIGHT; y++)
	{    // Expect exactly SCREEN_HEIGHT lines
		if (!std::getline(file, line))
		{
			errorMsg = "Map from file: " + filename + "has too few lines";
			return false;
		}
		// Each line must be wide enough
		if (static_cast<int>(line.length() < SCREEN_WIDTH))
		{
			errorMsg = "Map from file:" + filename + "\nLine " + std::to_string(y) + " is too short";
			return false;
		}

		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{			
			char c = line[x];

			if (c == LEGEND)
			{
				// Legend anchor must appear exactly once
				if (legend.doesExists())
				{
					errorMsg = "Map from file:" + filename + "\nMultiple legend anchors found";
					return false;
				}

				setLegendAnchor(x, y);   
				board[x][y] = ' ';     // legend cell is not part of the board
			}
			else if (isValidBoardChar(c))
			{
				// Valid game element
				board[x][y] = c;
			}
			else
			{   
				// Unknown character: replace with space and warn once
				board[x][y] = ' ';

				if (warningMsg.empty())
				{
					warningMsg =
						"Map from file: " + filename + "\n"
						"Unknown characters were replaced with spaces";
				}
			}
		}
	}
	// Legend must exist
	if (!legend.doesExists())
	{
		errorMsg = "Map file: " + filename + "\n"
			       "Legend is missing";	
		return false;
	}

	sourceFile = filename;
	return true;
}

bool Screen::buildObjectsFromBoard(std::string& error)
{
	// Iterates over the board and creates game objects
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			char c = board[x][y];
			handleChar(c, x, y);
		}
	}
	// Post-processing for multi-cell objects
	if (!buildSpringsFromBoard(error))
		return false;


	buildObstaclesFromBoard();
	return true;
}

void Screen::handleChar(char c, int x, int y)
{
	// Creates a specific game object based on a board character.
	// ----- Door -----
	if (c >= DOOR_MIN_CHAR && c <= DOOR_MAX_CHAR)
	{
		int dest = c - DIGIT_ZERO;
		addDoor(Door(Point(x, y), dest));
	}
	// ----- Key -----
	else if (c == BOARD_KEY)
	{
		int idx = static_cast<int>(keys.size());
		addKey(Key(Point(x, y), -1, idx));
	}
	// ----- Bomb -----
	else if (c == BOARD_BOMB)
	{
		int idx = static_cast<int>(bombs.size());
		addBomb(Bomb(Point(x, y), idx));
	}
	// ----- Torch -----
	else if (c == BOARD_TORCH)
	{
		int idx = static_cast<int>(torches.size());
		addTorch(Torch(Point(x, y), idx));
	}
	// ----- Switch -----
	else if (c == BOARD_SWITCH_OFF || c == BOARD_SWITCH_ON)
	{
		bool state = (c == BOARD_SWITCH_ON);
		addSwitch(Switch(Point(x, y), -1, state));
	}
	else if (c == BOARD_RIDDLE)
	{
		addRiddle(Riddle(Point(x, y)));
	}
	else
	{
		// Empty or non-object cell
		return;
	}
}

bool Screen::readDataFromFile(std::ifstream& file, const std::string& filename, std::string& errorMsg)
{   // Reads additional rule lines after the map section.
	std::string line;
	int lineNumber = SCREEN_HEIGHT; // Start reading from the end of the map

	while (std::getline(file, line))
	{
		++lineNumber;

		// Skip empty lines 
		const size_t firstChar = line.find_first_not_of(" \t\r\n");
		if (firstChar == std::string::npos)
			continue;


		// Parse and 
		// 
		// rule
		std::string ruleError;
		if (!parseDataLine(line, ruleError))
		{
			std::stringstream ss;
			ss << "Screen file: " << filename
			   << "\nLine " << lineNumber
			   << ": " << ruleError;

			errorMsg = ss.str();			return false;
		}
	}

	return true;
}

bool Screen::parseDataLine(const std::string& line, std::string& errorMsg)
{ // Parses a single rule line and applies it to the relevant object.
	std::stringstream ss(line);
	std::string type;

	ss >> type;
	int x, y;
	std::string tmp;    // if there's a problem check if it's ok

	if (type == "DARK")
	{
		int x1, x2, y1, y2;

		ss >> x1 >> y1 >> x2 >> y2;

		addDarkArea(Point(x1, y1), Point(x2, y2));
	}
	else if (type == "DOOR")
	{
		int doorID, isOpen, doorKeys, rule;

	    // Expected structured format
		if (!(ss >> x >> y >> tmp >> doorID >> tmp >> isOpen >> tmp >> doorKeys >> tmp >> rule))
		{
			errorMsg = "Invalid Door rule format" ;
			return false;
		}
		// Ensure the rule refers to an existing door on the board
		Door* d = getDoorAt(Point(x, y));
		if (!d)
		{
			errorMsg = "Rule refers to non-existing door at (" +
				std::to_string(x) + "," + std::to_string(y) + ")";

			return false;
		}

		d->applyRules(doorID, doorKeys, isOpen != 0, rule);
	}
	else if (type == "KEY")
	{
		int doorID;
 
		if (!(ss >> x >> y >> tmp >> doorID))  	// Expected structured format
		{
			errorMsg = "Invalid Key rule format";
			return false;
		}

		Key* k = getKeyAt(Point(x, y));
		
		if (!k)  // Ensure the rule refers to an existing key on the board
		{
			errorMsg = "Rule refers to non-existing key at (" +
				std::to_string(x) + "," + std::to_string(y) + ")";
			return false;
		}

		k->setDoorId(doorID);
	}
	else if (type == "SWITCH")
	{
		int doorID;

		if (!(ss >> x >> y >> tmp >> doorID))    	// Expected structured format
		{
			errorMsg = "Invalid Switch rule format";
			return false;
		}

		Switch* sw = getSwitchAt(Point(x, y));
		
		if (!sw)  // Ensure the rule refers to an existing switch on the board
		{
			errorMsg = "Rule refers to non-existing switch at (" +
				std::to_string(x) + "," + std::to_string(y) + ")";
			return false;
		}

		sw->setDoorID(doorID);
	}
	else if (type == "TELEPORT")
	{
		int x1, y1, x2, y2;
		if (!(ss >> x1 >> y1 >> x2 >> y2))
		{
			errorMsg = "Invalid TELEPORT format (expected: TELEPORT x1 y1 x2 y2)";
			return false;
		}
		return addTeleporterPair(Point(x1, y1), Point(x2, y2), errorMsg);
	}

	else    
	{
		errorMsg = "Unknown rule type: " + line;
		return false;
	}
	return true;
}

bool Screen::addTeleporterPair(Point p1, Point p2, std::string& errorMsg)
{
	// Validate points is within bounds
	if (p1.getX() < 0 || p1.getX() >= SCREEN_WIDTH ||
		p1.getY() < 0 || p1.getY() >= SCREEN_HEIGHT) {
		errorMsg = "Teleporter out of bounds at (" +
			std::to_string(p1.getX()) + "," + std::to_string(p1.getY()) + ")";
		return false;
	}
	if (p2.getX() < 0 || p2.getX() >= SCREEN_WIDTH ||
		p2.getY() < 0 || p2.getY() >= SCREEN_HEIGHT) {
		errorMsg = "Teleporter out of bounds at (" +
			std::to_string(p2.getX()) + "," + std::to_string(p2.getY()) + ")";
		return false;
	}
	// Validate source position has teleporter char
	if (charAt(p1) != BOARD_TELEPORT) {
		errorMsg = "No teleporter char at (" +
			std::to_string(p1.getX()) + "," + std::to_string(p1.getY()) + ")";
		return false;
	}
	if (charAt(p2) != BOARD_TELEPORT) {
		errorMsg = "No teleporter char at (" +
			std::to_string(p2.getX()) + "," + std::to_string(p2.getY()) + ")";
		return false;
	}
	// Pair got the same points
	if (p1 == p2) {
		errorMsg = "Teleporter cannot point to itself";
		return false;
	}
	// Check for duplicate teleporter at same position
	for (const auto& tp : teleporters) {
		Point p=tp.getP1();
		if (p == p1 || p == p2) {
			errorMsg = "Duplicate teleporter definition at (" +
				std::to_string(p.getX()) + "," + std::to_string(p.getY()) + ")";
			return false;
		}
	}
	// All good - add it
	teleporters.emplace_back( p1, p2 );
	teleporters.emplace_back( p2, p1 );
	return true;
}

bool Screen::buildSpringsFromBoard(std::string& error)
{
	// Save links that are already part of a spring so we won't check them twice
	// Also, if we find later links that are not in the cet means that they don't have a base
	std::set<Point> usedSpringCells;

	// Determines whether a spring cell (board[x][y] == BOARD_SPRING) is a base.
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			if (!(isSpring({ x,y })))
				continue;

			// It is a spring!
			Direction dir;
			// Only build spring if this cell is a base
			if (!isSpringBase(x, y, dir))
				continue;

			Point base(x, y);
			// Count spring length
			int size = 0;
			Point p = base;

			while (Point::checkLimits(p) &&
				charAt(p) == BOARD_SPRING)
			{
				usedSpringCells.insert(p); // cell is assigned to existing spring, don't check it again
				size++;
				p = p.next(dir);
			}
			// Spring MUST be more than one char!
			if (size < 2) {
				error = "Spring at (" + std::to_string(x) +
					"," + std::to_string(y) + ") must be at least 2 characters";
				return false;
			}

			// Create spring object
			addSpring(Spring(base, size, dir));
		}
	}
	// this part was made with Claude help
	// going through the screen and sees if there were links that weren't add to the set (because they had no base)
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			if (isSpring({ x, y }) && usedSpringCells.find({ x, y }) == usedSpringCells.end())
			{
				bool hasAdjacentWall =
					(y > 0 && isWall({ x, y - 1 })) ||
					(y < SCREEN_HEIGHT - 1 && isWall({ x, y + 1 })) ||
					(x > 0 && isWall({ x - 1, y })) ||
					(x < SCREEN_WIDTH - 1 && isWall({ x + 1, y }));

				if (hasAdjacentWall) {
					error = "Spring at (" + std::to_string(x) +
						"," + std::to_string(y) + ") must be at least 2 characters";
				}
				else {
					error = "Spring character at (" + std::to_string(x) +
						"," + std::to_string(y) + ") is not attached to a wall";
				}
				return false;
			}
		}
	}
	return true;
}

bool Screen::isSpringBase(int x, int y, Direction& dir) const
{
	// Spring MUST be more than one char!
	// Wall above -> spring goes DOWN
	if (y > 0 && isWall({ x, y - 1 }))
	{
		if (isSpring({ x,y + 1 })) {
			dir = Direction::DOWN;
			return true;
		}
	}
	// Wall below -> spring goes UP
	if (y < SCREEN_HEIGHT - 1 && isWall({ x, y + 1 })) {
		if (isSpring({ x,y - 1 })) {
			dir = Direction::UP;
			return true;
		}
	}
	// Wall to the left -> spring goes RIGHT
	if (x > 0 && isWall({ x - 1, y })) {
		if (isSpring({ x + 1,y })) {
			dir = Direction::RIGHT;
			return true;
		}
	}
	// Wall to the right -> spring goes LEFT
	if (x < SCREEN_WIDTH - 1 && isWall({ x + 1, y })) {
		if (isSpring({ x - 1,y })) {
			dir = Direction::LEFT;
			return true;
		}
	}
	return false;
}

void Screen::buildObstaclesFromBoard()  // *Developed with ChatGPT assistance*
{	
	// Scans the board and builds obstacle objects from connected obstacle cells.
	bool visited[SCREEN_WIDTH][SCREEN_HEIGHT] = { false };   // Marks board cells that were already processed

	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			// Start DFS only from unvisited obstacle cells
			if (charAt({ x, y }) == BOARD_OBSTACLE && !visited[x][y])
			{
				std::vector<Point> body;
				collectObstacleDFS(x, y, visited, body);   // Collect all connected obstacle cells
				addObstacle(Obstacle(body));     // Create a single obstacle from the collected cells
			}
		}
	}
}

void Screen::collectObstacleDFS(int x, int y, bool visited[][SCREEN_HEIGHT], std::vector<Point>& body)
{ // Recursively explores the four cardinal directions 

	// Stop if out of board bounds
	if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
		return;

	// Stop if this cell was already processed
	if (visited[x][y])
		return;

	// Stop if this cell is not an obstacle
	if (charAt({ x, y }) != BOARD_OBSTACLE)
		return;

	// Mark cell as visited and add it to the obstacle body
	visited[x][y] = true;
	body.emplace_back(x, y);

	// Explore neighboring cells
	collectObstacleDFS(x + 1, y, visited, body);
	collectObstacleDFS(x - 1, y, visited, body);
	collectObstacleDFS(x, y + 1, visited, body);
	collectObstacleDFS(x, y - 1, visited, body);
}

bool Screen::isValidBoardChar(char c)
{	// Checks whether a character is allowed in the screen map.
	if (c >= DOOR_MIN_CHAR && c <= DOOR_MAX_CHAR) 
		return true;
	switch (c)
	{
	case ' ':
	case BOARD_WALL:
	case WALL_HORIZ:
	case WALL_VERT:
	case BOARD_KEY:
	case BOARD_TORCH:
	case BOARD_BOMB:
	case BOARD_SWITCH_ON:
	case BOARD_SWITCH_OFF:
	case BOARD_RIDDLE:
	case BOARD_OBSTACLE:
	case BOARD_SPRING:
	case BOARD_TELEPORT:
	case LEGEND:
		return true;
	default: // c isn't one of the other cases
		return false;
	}
}

bool Screen::validateLegendPlacement(std::string& errorMsg) const
{   // Validates that the legend does not overlap forbidden board elements.
	if (!legend.doesExists())
		return true;

	Point topLeft = legend.getTopLeft();
	Point bottomRight = legend.getBottomRight();

	for (int y = topLeft.getY(); y <= bottomRight.getY(); ++y)
	{
		for (int x = topLeft.getX(); x <= bottomRight.getX(); ++x)
		{
			char c = board[x][y];

			// Legend may overlap only empty cells or outer walls
			if (c != ' ' && c != 'W')
			{
				errorMsg = "Invalid LEGEND placement: overlaps forbidden object '";
				errorMsg += c;
				errorMsg += "' at position (";
				errorMsg += std::to_string(x);
				errorMsg += ",";
				errorMsg += std::to_string(y);
				errorMsg += ").";

				return false;
			}
		}
	}

	return true;
}

bool Screen::validateDoors(int numRooms, std::string& errorMsg) const
{
	for (const Door& d : doors)
	{
		int dest = d.getDestination();

		if (dest < 2 || dest > numRooms + 1)
		{
			errorMsg =
				"Map from file: " + sourceFile +
				"\nDoor destination " + std::to_string(dest) +
				" out of range, allowed range: (2 - " + std::to_string(numRooms + 1) + ")";
			return false;
		}
	}
	return true;
}


void Screen::clearRoom()
{
	// clear board
	for (auto & x : board)
		for (char & y : x)
			y = ' ';

	// clear illumination
	clearIllumination();

	// clear all objects
	resetObjects();
	
}

void Screen::resetObjects()
{
	legend = LegendArea{};
	darkAreas.clear();

	doors.clear();
	keys.clear();
	bombs.clear();
	springs.clear();
	switches.clear();
	torches.clear();
	riddles.clear();
	obstacles.clear();
	teleporters.clear();
}

// Display Functions

void Screen::drawChar(const Point& p,const char c)
{
	// Draws a character on screen and updates the board buffer.
    board[p.getX()][p.getY()] = c;
	if (isVisible(p))
	{
		Utils::gotoxy(p.getX(), p.getY());
		std::cout << c;
	}
}

void Screen::erasePoint(const Point& p)
{
	if (Point::checkLimits(p)) {
		board[p.getX()][p.getY()] = ' ';
	}
}

// Draws the full screen: map base + all active items.
void Screen::drawScreen() 
{
	drawBase();
	drawItems();
}

// Prints the entire board buffer to the console.
void Screen::drawBase() const
{
	for (int y = 0; y < SCREEN_HEIGHT; ++y)
	{
		Utils::gotoxy(0, y);

		for (int x = 0; x < SCREEN_WIDTH; ++x)
		{
			Point p(x, y);

			if (isVisible(p) || board[x][y] == BOARD_WALL)
				std::cout << board[x][y];
			else
				std::cout << DARK_CHAR;   
		}
	}
}

template <typename T>
void Screen::drawCollectibleVector(const std::vector<T>& items) {
	for (const auto& item : items) {
		if (item.isActive()) {
			drawChar(item.getPos(), item.getFigure());
		}
	}
}

// Draws all visible objects onto the screen.
void Screen::drawItems()
{
	for (const auto& d : doors) {
		Point p = d.getPos();
		char fig = d.getFigure();
		drawChar(p, fig);
	}

	for (const auto& sw : switches) {
		Point p = sw.getPos();
		char fig = sw.getFigure();
		drawChar(p, fig);
	}

	drawCollectibleVector(keys);
	drawCollectibleVector(bombs);
	drawCollectibleVector(torches);

	for (const auto& r : riddles) {
		if (!r.isSolved())
		{
			Point p = r.getPos();
			char fig = r.getFigure();
			drawChar(p, fig);
		}
	}

	for (const auto& sp : springs) 
	{
		for (int k = 0; k < sp.getCurrSize(); k++)
		{
			Point p = sp.getLinkPos(k);
			char fig = sp.getFigure();
			drawChar(p, fig);
		}
	}

	for (const Obstacle& ob : obstacles)
	{
		const std::vector<Point>& body = ob.getBody();
		char fig = ob.getFigure();

		for (const Point& p : body)
		{
			drawChar(p, fig);
		}
	}
}

// Functions that check objects

bool Screen::isCellFree(const Point& pos) const
{
	int x = pos.getX();
	int y = pos.getY();

	// checking pos isn't out of screen bounds
	if (x < 0 || x >= SCREEN_WIDTH ||
		y < 0 || y >= SCREEN_HEIGHT)
	{
		return false;
	}

	// checking pos isn't a wall
	if (isWall(pos))
	{
		return false;
	}

	// cell is free:
	return true;
}


bool Screen::isWall(const Point& p) const {
	char c = charAt(p);
	return c == BOARD_WALL || c == WALL_VERT || c == WALL_HORIZ;
}

bool Screen::isItem(const Point& p) const {
	char c = charAt(p);
	return c == BOARD_KEY || c == BOARD_BOMB || c == BOARD_TORCH;
}

bool Screen::isDoor(const Point& p) const {
	return isdigit(charAt(p));
}

bool Screen::isSwitch(const Point& p) const {
	char c = charAt(p);
	return c == BOARD_SWITCH_ON || c == BOARD_SWITCH_OFF;
}

bool Screen::isObstacle(const Point& p) const {
	return charAt(p) == BOARD_OBSTACLE;
}

bool Screen::isSpring(const Point& p) const {
	return charAt(p) == BOARD_SPRING;
}

/// Get Objects Functions

Spring* Screen::getSpringAt(Point p)
{
	for (Spring& sp: springs)
	{
		if (sp.isSpringBody(p))
		{
			return &sp;
		}
	}

	return nullptr;   // no spring at this position
}

Obstacle* Screen::getObstacleAt(Point p)
{
	for (Obstacle& ob : obstacles)
	{
		if (ob.isObBody(p))
			return &ob;
	}
	return nullptr;   // no obstacle at this position
}

Door& Screen::getDoorById(int id)
{
	// Search by logical doorID
	for (auto & door : doors) {
		if (door.getDoorID() == id) {
			return door;
		}
	}
	throw std::runtime_error("Error: Door ID not found in getDoorById");   // needed otherwise get error on Mac
}

Point Screen::getTeleportDest(const Point& p) const { // returns portal dest
	for (const auto& pair : teleporters) {
		Point dest = pair.getPartner(p);
		if (dest != p) return dest;
	}
	return p;
}


// Helper Functions

// Adding a collected item to player's inventory
void Screen::collectItemAt(Player& player, const Point& p)
{
	Collectible* item = collect(p);
	if (item && item->isActive() && item->canPickUp()) {
		player.collectItem(item->getItemID());
		item->deactivate();
		erasePoint(p);
	}
}

Collectible* Screen::collect(const Point& p) {
	if (auto* k = getItemAt(keys, p))    return k;
	if (auto* b = getItemAt(bombs, p))   return b;
	if (auto* t = getItemAt(torches, p)) return t;
	return nullptr;
}

Collectible* Screen::getStoredItem(const Item &id) {
	if (id.type == ItemType::NONE || id.Index == -1) return nullptr;
	switch (id.type) {
		case ItemType::KEY:   return &keys[id.Index];
		case ItemType::BOMB:  return &bombs[id.Index];
		case ItemType::TORCH: return &torches[id.Index];
		default:              return nullptr;
	}
}

void Screen::pushObstacle(Obstacle& ob, Direction dir) {
	// Clears obstacle from board and moves it atomically
	for (const Point& cell : ob.getBody()) { // Remove all current obstacle cells from the board
		erasePoint(cell);
	}
	// Move the entire obstacle body
	ob.move(dir);
}

// Legend helpers

void Screen::setLegendAnchor(int x, int y)
{
	// Sets the legend top left anchor and calculates its bounding rectangle.
	Point topLeft(x, y);
	Point bottomRight(
		x + LEGEND_WIDTH - 1,
		y + LEGEND_HEIGHT - 1
	);

	legend = LegendArea(topLeft, bottomRight, true);
	legend.setExists(true);
}

bool Screen::isLegendCell(const Point& p) const
{   // Checks whether a given point lies inside the legend area.
	return legend.isInside(p);
}

void Screen::clearLegendAreaFromBoard()
{
	if (!legend.doesExists())
		return;

	Point topLeft=legend.getTopLeft();
	Point BottomRight=legend.getBottomRight();

	for (int y = topLeft.getY(); y <= BottomRight.getY(); ++y)
	{
		for (int x = topLeft.getX(); x <= BottomRight.getX(); ++x)
		{
			board[x][y] = ' ';
		}
	}
}

// Dark Areas & Torch helpers

bool Screen::isVisible(const Point& p) const
{
	// Determines whether the given cell should be visible on screen
	if (!isInDarkArea(p) || isIlluminated(p))
		return true; // cell isn't in a dark area or illuminated by a torch

	return false;
}

bool Screen::isInDarkArea(const Point& p) const
{
	// Checks whether the given position lies within any predefined dark area
	for (const DarkArea& area : darkAreas) {
		if (area.isInside(p)) return true;
	}
	return false;
}

bool Screen::isIlluminated(const Point& p) const  // MAYBE MOVE TO HEADER AS INLINE
{
	// Returns whether the given cell is currently marked as illuminated
	return illuminated[p.getX()][p.getY()];
}

void Screen::illuminateMap(const Point& center)
{   // Illuminates a circular area around a given center point.
	int cx = center.getX();
	int cy = center.getY();

	for (int dy = -2; dy <= 2; dy++) {
		for (int dx = -2; dx <= 2; dx++) {
			// Skip points outside the illumination radius
			if (abs(dx) + abs(dy) > 3) continue;

			int x = cx + dx;
			int y = cy + dy;

			if (Point::checkLimits(Point(x, y))) {
				illuminated[x][y] = true; 	// Always illuminate the center cell
			}
		}
	}
}

void Screen::clearIllumination()
{
	// Clears all illumination marks before recalculating lighting.
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (auto & x : illuminated) {
			x[y] = false;
		}
	}
}


// These functions remove objects from the screen's arrays
// They are mainly used during bomb explosions

bool Screen::removeObjectsAt(const Point& p)
{
	bool removed = false;

	removed |= removeItemAt(doors, p);
	removed |= removeItemAt(keys, p);
	removed |= removeItemAt(switches, p);
	removed |= removeItemAt(riddles, p);
	removed |= removeItemAt(torches, p);
	removed |= removeTeleporterAt(p);

	removeSpringAt(p);
	removeObstacleAt(p);

	return removed;
}

bool Screen::removeSpringAt(const Point& p) {
	for (auto it = springs.begin(); it != springs.end(); ++it) {
		if (it->isSpringBody(p)) { // checks if p is part of the spring
			int distX = abs(p.getX() - it->getPos().getX()); // abs because index might be negative
			int distY = abs(p.getY() - it->getPos().getY());

			int hitIndex = distX + distY; //how far is it from base/link's index

			if (hitIndex == 0) { //hit index is springs base
				for (int i = 0; i < it->getCurrSize(); i++) { //fully delete spring so we won't have "flying" links
					erasePoint(it->getLinkPos(i)); //erase from screen
				}
				springs.erase(it); //erase from vector
				return true;
			}

			else {
				int oldSize = it->getCurrSize();
				for (int i = hitIndex; i < oldSize; i++) {
					erasePoint(it->getLinkPos(i)); //erase from screen
				}

				int linksToRemove = it->getCurrSize() - hitIndex; //logic: how many links were removed
				it->decreaseSize(linksToRemove);
			}
			return true;
		}
	}
	return false;
}

// remove obstacle using iterators
void Screen::removeObstacleAt(const Point& p)
{
	for (auto obIt = obstacles.begin(); obIt != obstacles.end(); ++obIt)	{
		auto& body = obIt->getBody();

		for (auto bodyIt = body.begin(); bodyIt != body.end(); ++bodyIt) {
			if (*bodyIt == p) {
				body.erase(bodyIt);

				if (body.empty()) {
					obstacles.erase(obIt);
				}
				return;
			}
		}
	}
}

bool Screen::removeTeleporterAt(const Point& p)
{
	bool removed = false;
	auto it = teleporters.begin();

	while (it != teleporters.end()) {
		// if point is p or p dest
		Point p1=it->getP1();
		Point p2=it->getP2();
		if (p1 == p || p2 == p) {
			// screen
			drawChar(p1, ' ');
			drawChar(p2, ' ');
			// logic
			it = teleporters.erase(it);;
			removed = true;
		}
		else ++it;
	}
	return removed;
}

