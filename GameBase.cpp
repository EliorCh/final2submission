#include "GameBase.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>

// Game Functions
GameBase::GameBase():
    steps(nullptr),
    results(nullptr),
    isRunning(false),
    gameOver(false),
    gameCycles(0),
    gameSeed(static_cast<unsigned int>(std::time(nullptr))),
    currRoomID(0)
{
    players.resize(static_cast<size_t>(PlayerID::PlayersNum));
    constexpr char keys1[] = { 'D','X','A','W','S','E' };
    constexpr char keys2[] = { 'L','M','J','I','K','O' };
    getPlayer(PlayerID::Player1).setControls('$', keys1);
    getPlayer(PlayerID::Player2).setControls('&', keys2);
}

GameBase::~GameBase()
{
    delete results;
    delete steps;
}

std::vector<std::string> GameBase::getScreenSourceFiles() const {
    // Used for saving metadata to allow consistent replay.
    std::vector<std::string> files;
    files.reserve(screens.size());
    for (const auto& screen : screens) {
        files.push_back(screen.getSourceFile());
    }
    return files;    // Returns the source file names of all loaded screens.
}

void GameBase::setGame() {
    gameOver = isRunning = false;
    for (auto& player : players) {
        player.initForNewGame();
    }
}

// Main game loop - manages input, updates game logic, and renders frames.
void GameBase::run() {
    isRunning = true;      // setting flags
    gameOver = false;

    render();              // draw the initial room before any movement

    while (isRunning) {
        gameCycles++;
        handleInput();     // handle user's input

        if (!isRunning) break;   // input may request to leave run()
        std::cout << "run loop: gameOver=" << gameOver << std::endl;

        if (!gameOver) update(); // update world state only in active gameplay

        if (!isRunning) break;

        render();                // redraw everything after update
        Utils::delay(getDelay());
    }
    if (gameOver)
    {
        onGameEnd();
    }
}
// Draws current room and both players.
void GameBase::render()
{
    screens[currRoomID].drawScreen();

    if (isFinalRoom(currRoomID))
        displayFinalScoreboard();
    else
        displayLegend(screens[currRoomID]);

    drawPlayers();
    std::cout << std::flush;
};

// Updates game state for all players
void GameBase::update() {
    Screen& room = screens[currRoomID];
    room.clearIllumination();         

    for (auto& player : players) {
        player.setPrevPos(player.getPos());
    }

    for (auto& player : players) {
        if (player.isFinished() || player.getRoomID() != currRoomID)   // Player not in this room or already finished
            continue;

        // Respawn logic
        if (player.getDead()) {
            player.respawn();
            continue;
        }

        handleTorch(player);

        player.tickAcceleration(); // ticking if needed
        int turns = player.getSpeed();

        // Inner loop handles cell-by-cell movement if speed > 1.
        for (int s = 0; s < turns; s++) {
            // accelerated movement
            if (player.isAccelerating()) {
                if (handleAcceleratedMovement(player)) break;
                continue; // skip normal movement block
            }

            Point nextPos = player.getNextPos();

            if (room.isLegendCell(nextPos)) break;
            if (handleSprings(player)) break;             // may override direction/force
            if (handleTeleports(player)) break;
            if (handleObstacles(player, nextPos)) break;  // may override direction/force
            if (handleRiddles(player)) break;

            if (!room.isCellFree(nextPos)) {
                // If player's in acceleration, wall stops it
                if (player.isAccelerating())
                    player.stopAcceleration();
                break;
            }

            if (playersCollide(player, nextPos)) break;  // other player collision

            player.move();   // commit movement
            // Interactions
            handleDoor(player);
            handleSwitch(player);
            handleCollectibles(player);

            // After actions player may have moved rooms or died
            if (player.getRoomID() != currRoomID || player.getDead())
                break;
        }
    }

    handleBombs();        // ticking bombs only once per frame

    bool allFinished = true;
    for (const auto& player : players) {
        if (!player.isFinished()) {
            allFinished = false;
            break;
        }
    }

    if (allFinished) {
        gameOver = true;
        onGameEnd();
    }
}

// Init Functions
void GameBase::initGame() {
    screens.clear();
    // --- Set global game state ---
    gameOver = false;
    currRoomID = ROOM1_SCREEN;

    // Set player progress
    int startY=9;
    for (auto& player : players) {
        player.initForNewGame();
        player.setRoomID(currRoomID);
        player.setStartPos(Point(3, startY));
        startY+=2;
    }
}

bool GameBase::loadGameFiles() {  // *Developed with AI assistance*

    std::vector<std::string> foundFiles;
    char filenameBuffer[50];

    // --- Find all level files ---
    for (int i = 1; i <= MAX_ROOMS_CAPACITY; ++i)
    {
        snprintf(filenameBuffer, sizeof(filenameBuffer), "adv-world_%02d.screen", i);
        std::ifstream fileCheck(filenameBuffer);
        if (fileCheck.good())
            foundFiles.emplace_back(filenameBuffer);
    }
    // No screen files found
    if (foundFiles.empty())
    {
        showError("No game files found.");
        return false;
    }
    // Not enough rooms to start a valid game
    if (foundFiles.size() < MIN_REQUIRED_ROOMS) {
        showError(
            std::string("Found only ")
            + std::to_string(foundFiles.size())
            + " rooms. Need at least "
            + std::to_string(MIN_REQUIRED_ROOMS) + "." );
        return false;
    }

    // Ensure rooms are loaded in lexicographical order
     std::sort(foundFiles.begin(), foundFiles.end());

     // Initialize screens from the discovered files
    if (!initGameFiles(foundFiles)) return false;

    return true;
}

bool GameBase::initGameFiles(const std::vector<std::string>& foundFiles)
{
    screens.clear();
    screens.resize(foundFiles.size() + 2);      // +2: index 0 unused, last index reserved for final room
    // Load each screen file
    for (size_t i = 0; i < foundFiles.size(); ++i)
    {
        std::string errorMsg, warningMsg;

        bool success = screens[i + 1].loadScreenFromFile( foundFiles[i], errorMsg, warningMsg );

        if (!success) {
            showError(errorMsg);
            return false;
        }
        // Non-fatal warnings are shown immediately
        if (!warningMsg.empty())
            showMessage(warningMsg);
    }

    if (!loadRiddles()) return false;

    int numRooms = static_cast<int>(foundFiles.size());  // number of real rooms (excluding dummy)

    for (size_t i = 1; i < screens.size(); ++i) {
        std::string errorMsg;

        // Validate doors
        if (!screens[i].validateDoors(numRooms, errorMsg)) {
            showError(errorMsg);
            return false;
        }

        // Validate legend placement
        if (!screens[i].validateLegendPlacement(errorMsg)) {
            showError(errorMsg);
            return false;
        }

        // Remove legend area from the playable board
        screens[i].clearLegendAreaFromBoard();
    }

    // Create and append the final screen
    int finalIndex = static_cast<int>(foundFiles.size()) + 1;
    screens[finalIndex].setMap(FINAL_MAP);

    // Set initial room
    currRoomID = ROOM1_SCREEN;     // Set initial room
    return true;
}

// used gemini's help for the shuffle of the riddles
bool GameBase::loadRiddles(int loadRoomID) { // if loadRoomID wasn't sent, loading all riddles
    std::ifstream file("riddles.txt");
    if (!file.is_open()) {
        showError("Cannot load riddles file");
        return false;
    }

    // temp structs for all the info before shuffling
    struct Coord { int roomID; int x; int y; }; // points
    struct Content { std::string question; std::string answer; }; // q&a

    std::vector<Coord> coords;
    std::vector<Content> contents;

    int roomID, x, y;
    std::string question, answer, dummy; //dummy for spaces

    while (file >> roomID >> x >> y) {
        std::getline(file, dummy);
        std::getline(file, question);
        std::getline(file, answer);

        if (roomID == -1) continue; // if its restart and not initial new game

        coords.push_back({ roomID, x, y });
        contents.push_back({ question, answer });
    }
    file.close();

    if (coords.empty()) return true;

    // shuffle content
    std::mt19937 g(gameSeed);
    std::shuffle(contents.begin(), contents.end(), g);

    for (size_t i = 0; i < coords.size(); ++i) {
        int room = coords[i].roomID;

        if (loadRoomID != -1 && room!=loadRoomID)
            continue;

        Point p(coords[i].x, coords[i].y);
        const auto& content = contents[i % contents.size()];

        Riddle* riddle = screens[room].getRiddleAt(p);
        if (riddle) {
            riddle->setData(content.question, content.answer);
            }
        else {
            if (loadRoomID != -1) {
                Riddle newRiddle(p);
                newRiddle.setData(content.question, content.answer);
                screens[room].addRiddle(newRiddle);
            }
            else {
                showError("Rule refers to non-existing riddle in room " + std::to_string(roomID) +
                    " at (" + std::to_string(x) + "," + std::to_string(y) + ")");
                return false;
            }
        }
    }
    return true;
}

// Restart Functions
bool GameBase::restartCurrentRoom() {
    // Final room does not support restart
    if (isFinalRoom(currRoomID)) return true;

    // Clear current room state
    screens[currRoomID].clearRoom();
    // Reload room from original file
    if (!reloadRoom(currRoomID)) return false;

    // Reset players that are currently in this room
    for (auto& player : players) {
        if (player.getRoomID() == currRoomID) {
            player.resetState();
        }
    }
    return true;
}

// Reloads a specific room from its original source file.
bool GameBase::reloadRoom(int roomID) {
    if (roomID < 0 || roomID >= screens.size())
        return false;

    std::string error, warning;
    const std::string& filename = screens[roomID].getSourceFile();
    // Room has no associated file (e.g., final room)
    if (filename.empty()) {
        showError("Restarting room failed");
        return false;
    }
    // Reload screen from file
    if (!screens[roomID].loadScreenFromFile(filename, error, warning)) {
        showError(error);
        return false;
    }

    if (!loadRiddles(roomID))
        return false;

    if (!screens[roomID].validateLegendPlacement(error)) {
        showError(error);
        return false;
    }

    screens[roomID].clearLegendAreaFromBoard();
    return true;
}

// made by chatGPT to simplify the code - for the use of showError and showMessage
void GameBase::displayPopup(const std::string& msg, const std::string& title) {
    Utils::clearScreen();

    std::string line1, line2;
    bool foundNewline = false;
    for (char c : msg) {
        if (c == '\n')
            {foundNewline = true;
            continue;}
        if (!foundNewline) line1 += c;
        else line2 += c;
    }

    if (!title.empty()) { // if there's a title like "error"
        Utils::printCentered(title, 10);
    }

    Utils::printCentered(line1, 12);

    if (!line2.empty()) {
        Utils::printCentered(line2, 13);
    }

    Utils::printCentered("Press any key to continue ", 17);

    std::cout << std::flush;
    Utils::getChar();
}


void GameBase::showError(const std::string& msg){
    displayPopup(msg, "ERROR:");
}

void GameBase::showMessage(const std::string& msg) {
    displayPopup(msg, "");
}

// Helper Functions

void GameBase::moveRoom(Player& player, int dest) {
    player.setRoomID(dest);
    player.setStartPos(getStartPoint(player));
    player.resetState();
    int idx = (&player == &players[0]) ? 0 : 1;      // determine which player is moving
    int other = 1 - idx;                             // index of the other player

    // --- Final Room Logic ---
    if (isFinalRoom(dest)) {
        Point startP = getStartPoint(player);   // starting position inside the final screen

        player.setRoomID(dest);             // mark this player in the final screen
        player.setFinished(true);;     // player reached the final room
        player.setStartPos(startP);
        player.resetState();

        // According to the rules: if only one player is finished,
        // we return to the second player
        if (!players[other].isFinished()) {
            currRoomID = players[other].getRoomID();;
            player.addScore(ScoreUtils::getValue(ScoreEvent::FinishGameFirst));
        }
        else {
            currRoomID = dest;         // both players are in the final room
            player.addScore(ScoreUtils::getValue(ScoreEvent::FinishGameSecond));
            gameOver = true;
        }

        if (results) { results->addGameEnd(gameCycles, player.getScore()); }

        return;    // finish handling final-screen transition
    }

    // --- Regular Room Transition ---
    player.incrementRoomsDone();                   // player completed one more room
    player.clearInventory();

    Point startP = getStartPoint(player);  // new position in the next room
    player.setRoomID(dest);                        // update player's destination room
    player.setStartPos(startP);
    player.resetState();

    player.addScore(ScoreUtils::getValue(ScoreEvent::OpenDoor));
    if (results) { results->addScreenChange(gameCycles, dest); }   

    // decide which room should currently be displayed:
    if (players[0].getRoomsDone() < players[1].getRoomsDone())
        currRoomID = players[0].getRoomID();       // player 1 is behind

    else if (players[1].getRoomsDone() < players[0].getRoomsDone())
        currRoomID = players[1].getRoomID();       // player 2 is behind

    else
        currRoomID = player.getRoomID();     // same progress - follow the moving player
}

// Calculates the player's starting position in the next room based on the door's position
Point GameBase::getStartPoint(Player& player) const {
    auto& room = screens[currRoomID];

    Point posOnDoor = player.getPos();                       // player is standing on the door
    int startY = posOnDoor.getY();                           // keep same row as the door

    int startX = (&player == &players[0] ? 1 : 2);      // player 0 and 1 don't get the same point

    const Point startPos(startX, startY);

   if (!room.isLegendCell(startPos) && room.charAt(startPos) == ' ')   // if cell is clear
       return startPos;

   for (int y = 1; y < SCREEN_HEIGHT; y++) {
       for (int x = 1; x < SCREEN_WIDTH; x++) {
           Point p(x, y);
           if (!room.isLegendCell(p) && room.isCellFree(p))
               return p;
       }
   }
   return startPos;
}

// Checks if the current player and the other player are about to step into the same cell
bool GameBase::playersCollide(Player& current, const Point& nextPos) {
    for (auto& other : players) {
        if (&other == &current) continue;
        if (other.getRoomID() != current.getRoomID()) return false;   // Only relevant if both players are in the same room

        Point otherPos = other.getPos();
        if (nextPos != otherPos) return false;

        Direction myDir = current.getDir();
        Direction otherDir = other.getDir();
        Point otherNext = other.getNextPos();

        Screen& room = screens[currRoomID];
        //bool otherBlocked = !room.isCellFree(otherNext);

        if (otherDir == Direction::STAY                  // Case 1: other player in stay mode = collision
            || (Point::areOpposite(myDir, otherDir))) {  // Case 2: head-on collision (opposite directions){
            current.bumpedInto(other);
            return true;
            }

        // Case 3: other player is blocked by an object
        if (!room.isCellFree(otherNext)) {
            current.bumpedInto(other);

            bool blockedByObstacle = room.isObstacle(otherNext);
            if (blockedByObstacle) {
                if (chainPushSuccess(current, myDir, otherNext)) {
                    current.setPushing(true);
                    return false;
                }
                return true; // collision
            }
            current.bumpedInto(other);
            return true; // collision
        }
    }
    // Otherwise: same direction (chasing) = no collision
    return false;
}

// Handle Functions

void GameBase::handleDoor(Player& player) {
    Screen& room = screens[currRoomID];
    Point p = player.getPos();

    if (!room.isDoor(p)) return;   // no door at this cell

    Door* d = room.getDoorAt(p);        // get door object
    int dest = d->getDestination();

    if (d->checkIsOpen()) {    // if door is already open move room immediately
        moveRoom(player, dest);
        return;
    }
    if (d->needsKey() && isMatchingKey(player, d)) // if player has a key & key fits the door
    {
        player.clearInventory(); // take the key from player
        d->useKey();             // one less key needed
        player.addScore(ScoreUtils::getValue(ScoreEvent::UseKey));  // adding score
    }

    if (d->getNeededKeys() == 0) {   // check if no more keys are needed
        d->updateKeyOK();         // change key flag
    }

    if (d->getKeyStatus() && d->getSwitchStatus()) {
        d->open();
        moveRoom(player, dest);
    }
    else {
        showDoorStatus(*d);
    }
}

void GameBase::handleSwitch(Player& player) {
    Screen& room = screens[currRoomID];
    Point p = player.getPos();

    if (!room.isSwitch(p)) return;  // no switch at this cell

    Switch* sw = room.getSwitchAt(p);
    sw->toggle();

    // Update switch character on screen
    char c = room.charAt(p);
    room.erasePoint(p);
    char fig = (c == '/' ? 'o' : '/');
    room.drawChar(p, fig);

    int id = sw->getDoorID();
    updateDoorBySwitches(id);
}

// *Logic reviewed with ChatGPT assistance*
bool GameBase::handleSprings(Player& player) {
    Screen& room = screens[currRoomID];
    Point next = player.getNextPos();

    if (!Point::checkLimits(next)) return false;

    if (player.isAccelerating()) {
        Spring* spAtNext = room.getSpringAt(next);
        if (spAtNext && player.getForcedDir() == spAtNext->getDir()) {
            return false;
        }
    }

    Spring* sp = room.getSpringAt(next);
    if (!sp) { // no spring at next pos - leaving spring / launch
        if (player.getCompression() > 0) {
            Spring* adj = findAdjacentSpring(player.getPos());
            if (adj && player.getPos() == adj->getBasePos() ) {
                launchPlayer(player, *adj);
                return true;
            }
        }
        return false;
    }

    if (player.isAccelerating() && player.getForcedDir() == sp->getDir()) {
        return false; // can leave the spring
    }

    SpringAction action = sp->interact(player);
    switch (action) {
        case SpringAction::Launch:
            launchPlayer(player, *sp);
            return true;

        case SpringAction::Compressed:
            compressSpring(player, *sp);
            player.move();
            return true;

        case SpringAction::Blocked:
            return true;

        default:
            return false;
    }
}

bool GameBase::compressSpring(Player& player, Spring& sp) {
    Screen& room = screens[currRoomID];
    Point oldTip = sp.getLinkPos(sp.getCurrSize());
    room.erasePoint(oldTip);
    player.addCompression();

    return (sp.getCurrSize() > 0);
}

void GameBase::handleBombs() {
    //Updates bomb timers in the current room and triggers explosions
    Screen& room = screens[currRoomID];
    std::vector<Bomb>& bombs = room.getBombs();
    std::vector<Point> dangerZone;

    Bomb::manageBombs(bombs, room, dangerZone);

    if (dangerZone.empty()) return;

    for (auto& player : players) {
        for (const auto& hazardPoint : dangerZone) {
            if (player.getPos() == hazardPoint) {
                applyLifeLoss(player);
                break;
            }
        }
    }
}

bool GameBase::handleRiddles(Player& player) {
    Screen& room = screens[currRoomID];
    const Point& nextPos = player.getNextPos();

    if (!Point::checkLimits(nextPos)) {
        player.setDirection(Direction::STAY);
        return true;
    }

    Riddle* r = room.getRiddleAt(nextPos);
    if (r == nullptr) return false;

    bool solved = false;

    // Virtual call - subclasses can override
    if (!getRiddleAnswer(r, solved)) {
        player.setDirection(Direction::STAY);
        return true;
    }

    if (solved) {
        room.erasePoint(nextPos);
        player.addScore(ScoreUtils::getValue(ScoreEvent::SolveRiddle));
        room.removeRiddleAt(nextPos);
    } else {
        player.setDirection(Direction::STAY);
    }
    return true;
}

void GameBase::handleTorch(Player& player) {
    Screen& room = screens[currRoomID];

    if (player.checkItem() == ItemType::TORCH)
        room.illuminateMap(player.getPos());
}

bool GameBase::handleObstacles(Player& player, const Point& nextPos) {
    Screen& room = screens[currRoomID];
    Point p = nextPos;

    // no obstacle at this cell - player can continue moving
    if (!room.isObstacle(p)) return false;

    Obstacle* ob = room.getObstacleAt(p);
    if (ob == nullptr) return false;

    Direction dir = player.getDir();
    int force = calcForce(player, ob, dir);

    if (!ob->canBePushed(force)) return true; // too weak - stop

    auto nextBody = ob->getNextBody(dir);
    if (!canMoveObstacle(nextBody, ob)) return true; // obstacle blocking the way

    room.pushObstacle(*ob, dir);
    player.setPos(p);
    return true;  // player can continue moving
}

void GameBase::handleCollectibles(Player& player) {
    // Player already holds an item cannot pick up another / just threw one
    if (!player.inventoryEmpty() || player.getDisposeFlag()) return;

    Screen& room = screens[currRoomID];
    Point p = player.getPos();
    room.collectItemAt(player, p);
}

bool GameBase::handleTeleports(Player& player) {
    Screen& room = screens[currRoomID];
    Point currentPos = player.getPos();
    if (currentPos == player.getTeleportPos()) {
        player.setTeleportPos({ -1, -1 });
        return false;
    }

    Point dest = room.getTeleportDest(currentPos);
    if (dest != currentPos) {
        player.setTeleportPos(dest);
        player.setPos(dest);
        return true;
    }
    return false;
}

bool GameBase::handleDispose(Player& player) {
    if (player.inventoryEmpty()) return false;

    Screen& room = screens[currRoomID];
    Point pos = player.getPos();
    if (room.charAt(pos) != EMPTY_CELL) return false;

    Collectible* item = room.getStoredItem(player.getInventory());

    if (item) {
        item->dispose(pos);
        player.clearInventory();
        player.setDisposeFlag(true);
        return true;
    }
    return false;
}

bool GameBase::handleAcceleratedMovement(Player& player)
{
    Screen& room = screens[player.getRoomID()];

    // build up to MAX_SUB_STEPS intermediate positions
    Point sub[MAX_SUB_STEPS];
    int count = player.getAccelerationSubSteps(sub);

    for (int k = 0; k < count; k++) {
        player.setPushing(false);

        Point nextPos = sub[k];

        if (room.isLegendCell(nextPos)) return true;

        if (!room.isCellFree(nextPos)) {
            // If player's in acceleration, wall stops it
            if (player.isAccelerating())
                player.stopAcceleration();
            break;
        }

        // spring interaction may override direction or launch
        if (handleSprings(player)) return true;

        if (handleTeleports(player)) return true;

        if (handleObstacles(player, nextPos)) return true;

        if (!handleRiddles(player)) {
            player.setDirection(Direction::STAY);
            return true;
        }

        // wall collision stops acceleration
        if (!room.isCellFree(nextPos)){
            player.stopAcceleration();
            return true;
        }

        // player collision
        if (playersCollide(player, nextPos)) return true;

        if (player.isPushing()){
            player.setPos(nextPos);
            break;
        }
        player.setPos(nextPos);          // commit movement
        handleCollectibles(player);   // collect items mid-flight
        handleSwitch(player);         // toggle switch mid-flight
        handleDoor(player);

        // if moved rooms or died stop movement now
        if (player.getRoomID() != currRoomID || player.getDead())
            return true;
    }
    return false;   // completed accelerated steps normally
}

// Helper to Handle Functions
bool GameBase::isMatchingKey(const Player& player, const Door* door) {
    Item currentItem = player.getInventory();                // get access to key object
    if (currentItem.type != ItemType::KEY) return false;     // player doesn't hold a key
    return currentItem.Index == door->getDoorID();
}

void GameBase::updateDoorBySwitches(int id)
{
    Screen& room = screens[currRoomID];
    Door& d = room.getDoorById(id);

    const auto& switches = room.getSwitches();

    int total = 0;     // number of switches linked to this door
    int countOn = 0;   // number of linked switches that are ON

    for (const auto& sw : switches) {
        if (sw.getDoorID() == id) {   // if switch [i] is linked to the door
            total++;                         // number of switched linked goes up
            countOn += sw.getState();  // if the switch is on number of countOn goes up
        }
    }

    // Update the switchOK flag by the door's rule
    if (d.getRule() == SwitchRule::ALL_ON)
        d.updateSwitchOK(total == countOn);

    else if (d.getRule() == SwitchRule::ALL_OFF)
        d.updateSwitchOK(countOn == 0);
}


Spring* GameBase::findAdjacentSpring(const Point& pos)
{
    // Finds a spring adjacent to the given position.
    Screen& room = screens[currRoomID];

    // First check if player is standing ON a spring
    Spring* sp = room.getSpringAt(pos);
    if (sp) return sp;

    static constexpr Direction ALL_DIRECTIONS[] = { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT };
    for (Direction dir : ALL_DIRECTIONS) {
        sp = room.getSpringAt(pos.next(dir));
        if (sp) return sp;
    }
    return nullptr;
}

void GameBase::launchPlayer(Player& player, Spring& sp) {
    Screen& room = screens[currRoomID];

    // Total compression force accumulated by the player
    int force = sp.springRelease();

    // Apply acceleration if any compression was done
    if (force > 0) {
        Direction dir=sp.getDir();
        Point tip = sp.getTipPos();
        Point launchPoint = tip.next(dir);

        player.accel(force, dir);   // launch in spring release direction
        player.setPos(launchPoint);
    }

    for (int i = 0; i < sp.getFullSize(); ++i) {
        room.drawChar(sp.getLinkPos(i), sp.getFigure());
    }
    player.resetCompression();  // clear stored compression count
}

int GameBase::calcForce(const Player& pusher, const Obstacle* ob, Direction dir) const
{
    int force = pusher.getSpeed();

    for (const auto& other : players) {
        if (&other == &pusher) continue;
        if (other.getRoomID() != currRoomID        // must be in same room
            || other.getDead()
            || other.getDir() != dir) continue;             // must move in same direction

        Point pusherStart = pusher.getPrevPos();
        Point otherStart = other.getPrevPos();

        // CASE 1: other player directly pushes the same obstacle
        // CASE 2: chain push (other pushes pusher, pusher pushes obstacle)
        if (ob->isObBody(otherStart.next(dir))
            || otherStart == pusherStart.next(Point::opposite(dir)))
            force += other.getSpeed();
    }
    return force;
}

bool GameBase::canMoveObstacle(const std::vector<Point>& nextBody, const Obstacle* currOb)
{
    Screen& room = screens[currRoomID];

    for (const Point& p : nextBody) {    // Check all body cells of the obstacle
        if (!Point::checkLimits(p)) return false;

        for (auto& player : players) {
            if (player.getRoomID() != currRoomID)
                continue;

            if (player.getPos() == p)
                return false;
        }

        if (room.charAt(p) == ' ') continue;

        if (room.isObstacle(p) && currOb->isObBody(p)) continue;

        return false;
    }
    return true;
}

bool GameBase::chainPushSuccess(const Player& pusher, Direction dir, const Point& obstaclePos) {
    Screen& room = screens[currRoomID];
    Obstacle* ob = room.getObstacleAt(obstaclePos);
    if (!ob) return false;

    // check combined force (existing logic)
    int force = calcForce(pusher, ob, dir);
    if (force < ob->getSize()) return false;

    // check obstacle can actually move
    auto nextBody = ob->getNextBody(dir);
    if (!canMoveObstacle(nextBody, ob)) return false;

    // push is real and will happen
    return true;
}

bool GameBase::processKey(char choice) {
    for (auto& player : players) {
        if (player.isDisposeKey(choice)) { // Dispose keys (collectibles)
            handleDispose(player);
            return true;
        }

        // Movement keys
        if (player.isMoveKey(choice)) {
            player.setDir(choice);
            return true;
        }
    }
    return false;
}

void GameBase::applyLifeLoss(Player& player)
{
    // Decrease player's life and check if still alive
    bool stillAlive = player.lowerLife();
    if (results) { results->addLostLife(gameCycles); }

    // Player has no lives left -> end game
    if (!stillAlive)
    {
        if (results) { results->addGameEnd(gameCycles, player.getScore()); }
        onPlayerDeath();
    }
}

// UI Display Functions
void GameBase::displayLegend(const Screen& room) const {
    room.getLegend().drawLegend(players);
}

void GameBase::displayFinalScoreboard() const {
    int totalScore = 0;
    int currentY = FINAL_SCOREBOARD_START_Y;

    // Center the scoreboard horizontally
    Utils::printCentered("====================", currentY++);
    Utils::printCentered("   FINAL SCORES", currentY++);
    Utils::printCentered("--------------------", currentY++);

    for (int i = 0; i < static_cast<int>(PlayerID::PlayersNum); ++i) {
        std::string playerLine = "Player " + std::to_string(i + 1) + " : " + std::to_string(players[i].getScore());
        Utils::printCentered(playerLine, currentY++);
        totalScore += players[i].getScore();
    }

    Utils::printCentered("--------------------", currentY++);
    std::string teamLine = "TEAM SCORE : " + std::to_string(totalScore);
    Utils::printCentered(teamLine, currentY++);
    Utils::printCentered("====================", currentY);
}

void GameBase::drawPlayers() const {
    for (const auto& player : players) {
        // if player isn't in current room (moved on to the next one) - no need to draw them
        if (player.getRoomID() != currRoomID || player.getDead()) {
            continue;
        }

        // draw players present in the current room
        player.draw();
    }
}

// shows how many more keys are needed / switches are ok
// function by GEMINI
void GameBase::showDoorStatus(const Door &d) {
    Utils::print(0, SCREEN_HEIGHT - 1, std::string(SCREEN_WIDTH, ' '));

    int keysLeft = d.getNeededKeys();
    bool switchOk = d.getSwitchStatus();

    std::string msg = "Door Locked: " + std::to_string(keysLeft) + " keys left | ";
    msg += (switchOk ? "Switch: OK" : "Switch: REQUIRED");

    int startX = Utils::getCenteredX(static_cast<int>(msg.length()) + 4);
    int startY = SCREEN_HEIGHT - 2;

    Utils::print(startX, startY, ">> " + msg + " <<");
    std::cout << std::flush;
}
