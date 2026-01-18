#include "Game.h"
#include <iostream>
#include <fstream>
#include <cstdio>

// Game Functions

Game::Game()
{
    Utils::initConsole();

    gameOver = false;           
    isRunning = false;

    setGame();
    showMenu();      // going into the main menu
}

void Game::setGame()
{
    gameOver = false;
    isRunning = false;

    for (int i = 0; i < NUM_PLAYERS; ++i)
    {
        playerFinished[i] = false;
        roomsDone[i] = 0;
        playerRoom[i] = -1;  
    }

    fixedScreens[MENU_SCREEN].setMap(MENU_MAP);
    fixedScreens[INSTRUCTIONS_SCREEN].setMap(INSTRUCTIONS_MAP);
}

Game::~Game()
{
    Utils::restoreConsole();
}

// Main game loop – manages input, updates game logic, and renders frames.
void Game::run() {
    isRunning = true;      // setting flags 
    gameOver = false;

    render();              // draw the initial room before any movement
  
    while (isRunning)         
    {
        handleInput();  // handle user's input

        if (!isRunning)    // input may request to leave run()    
            break;

        if (!gameOver)    
        {
            update();    // update world state only in active gameplay     
        }

        if (!isRunning)
            break;

        render();       // redraw everything after update
        Utils::delay(150);
    }
}

void Game::handleInput()
{
    if (!Utils::hasInput())   // no key pressed this frame
        return;

    char ch = Utils::getChar();
    char c = std::toupper(ch);

    // If the game is already over (final room):
    // only 'H' should work and return to the main menu
    if (gameOver)
    {
        if (c == HOME)
        {
            isRunning = false;      // leave run() and go back to menu
        }
        return;                     // ignore all other keys in final room
    }
    // Pause the game
    if (c == ESC)
    {
        pauseGame();
        return;
    }

    // Player wants to restart room
    if (c == RESTART)
    {
        if (!restartCurrentRoom()){

            isRunning = false;
            return;
        }
        return;
    }
    // Dispose keys (bomb/key usage)
    if (players[PLAYER_1].isDisposeKey(c)) {
        
        handleDispose(players[PLAYER_1]);
        return;
    }

    if (players[PLAYER_2].isDisposeKey(c)) {
      
        handleDispose(players[PLAYER_2]);
        return;
    }
    // Movement keys
    if (players[PLAYER_1].isMoveKey(c)){
       
        players[PLAYER_1].setDir(c);
    }
    if (players[PLAYER_2].isMoveKey(c)){
        
        players[PLAYER_2].setDir(c);
    }
}

// Updates game state for all players
void Game::update()
{
    Screen& room = screens[currRoomID];
    room.clearIllumination();           // עבר לכאן כדי שלא יתבצע פעמיים עבור כל שחקן

    for (int i = 0; i < NUM_PLAYERS; i++)
        prevPos[i] = players[i].getPos();

    for (int i = 0; i < NUM_PLAYERS; i++)
    {
        Player& player = players[i];
         
        if (playerFinished[i] || playerRoom[i] != currRoomID)   // Player not in this room or already finished
            continue;

        // Respawn logic
        if (player.getDead()) 
        {
            player.respawn();     
            continue;
        }

        handleTorch(players[i]);

        Screen& room = screens[playerRoom[i]];
        int steps = player.getSpeed();

        if (player.isAccelerating())
            player.tickAcceleration();

        // Inner loop – handles cell-by-cell movement if speed > 1.
        for (int s = 0; s < steps; s++)
        {
            Point stepStartPos = player.getPos();

            // accelerated movement
            if (player.isAccelerating())
            {
                bool stop = handleAcceleratedMovement(player, i);
                if (stop) break;

                continue; // skip normal movement block
            }

            Point nextPos = player.getNextPos();

            if (room.isLegendCell(nextPos))
            {
                break;
            }

            if (handleSprings(player))   // may override direction/force
                break;

            if (handleTeleports(player))
                break;

            if (handleObstacles(player, nextPos))   // may override direction/force
                break;

            if (!handleRiddles(player)) {
                player.setDirection(STAY);
                break;
            }


            if (!room.isCellFree(nextPos))
            {
                // If player's in acceleration, wall stops it
                if (player.isAccelerating())
                {
                    player.stopAcceleration();
                }
                break;
            }

            if (playersCollide(i, nextPos))   // other player collision
                break;

            player.move();   // commit movement

            // Interactions
            handleDoor(player);
            handleSwitch(player);
            handleCollectibles(player);

            // After actions player may have moved rooms or died
            if (playerRoom[i] != currRoomID || player.getDead())
                break;
        }
    }       

    handleBombs();        // ticking bombs only once per frame

    if (playerFinished[PLAYER_1] && playerFinished[PLAYER_2])
    {
        gameOver = true;
    }
}

// Draws current room and both players.
void Game::render()
{
    Screen& room = screens[currRoomID];

    room.drawScreen();
    drawPlayers();

    if (isFinalRoom(currRoomID))
        displayFinalScoreboard();
    else
        displayLegend(room);

    std::cout << std::flush;
}

void Game::pauseGame()
{   
    Utils::clearScreen();
    Utils::gotoxy(5, 10);
    std::cout << "Game paused, press ESC again to continue or H to go back to the main menu";
    std::cout << std::flush;

    while (true)
    {
        char ch = Utils::getChar(); // Waiting for user's response
        char c = std::toupper(ch);

        // If ESC is pressed again - we return to the game
        if (c == ESC)
        {
            // clearing the pause message
            Utils::gotoxy(5, 10);
            std::cout << "                                                               ";
            return;
        }

        // H/h - stop the game
        if (c == 'h' || c == 'H')
        {
            isRunning = false;   // breaking the loop in run and returning to menu
            return;
        }
    }
}
    
void Game::showMenu()
{
    char choice = '\0'; 

    while (true)
    {
        fixedScreens[MENU_SCREEN].drawBase();
        std::cout << std::flush;

        char ch = Utils::getChar();
        choice = std::toupper(ch);

        switch (choice)
        {
        case START: // Start new game
            initGame();          // prepares the game - map, objects, players        

            if (!loadGameFiles())  // file-related error: return to main menu
                break;  

            run();    // start game
            break;

        case INSTRUCTIONS:    // Show instructions
            showInstructions();
            break;

        case EXIT:    // Exit game
            isRunning = false;
            return;

        default:       
            Utils::gotoxy(33, 14);
            std::cout << "Invalid choice.";
            Utils::delay(800);
            break;
        }
    }
}

void Game::showInstructions()
{
    Utils::clearScreen();
    fixedScreens[INSTRUCTIONS_SCREEN].drawBase();   // Shows the instructions screen

    // Title
    Utils::gotoxy(30, 2);
    std::cout << "=== INSTRUCTIONS ===";

    // Goal & Basics
    Utils::gotoxy(2, 3);
    std::cout << "GOAL: Reach Final Room together! Move through rooms and earn points.";
    Utils::gotoxy(2, 4);
    std::cout << "RESTART ROOM: 'R' || GAME OVER: If any player has 0 Lives.";
    Utils::gotoxy(2, 5);
    std::cout << "POINTS: Key(10) Door(20) Riddle(10) Win(1st:100/2nd:50).";

    // Controls
    Utils::gotoxy(4, 7);
    std::cout << "CONTROLS:         PLAYER 1      PLAYER 2";
    Utils::gotoxy(4, 8);
    std::cout << "Move (U/L/D/R):   W/A/X/D       I/J/M/L";
    Utils::gotoxy(4, 9);
    std::cout << "Stay / Dispose:   S  /  E       K  /  O";

    Utils::gotoxy(4, 11);
    std::cout << "ITEMS (Walk over an item to pick it up, max 1 item per player):";
    Utils::gotoxy(6, 12);
    std::cout << "+ Key (" << BOARD_KEY << "): Collect to open matching doors.";
    Utils::gotoxy(6, 13);
    std::cout << "+ Bomb (" << BOARD_BOMB << "): Explodes in 5 turns. Destroys players & walls (" << WALL_VERT << ' ' << WALL_HORIZ << ").";
    Utils::gotoxy(6, 14);
    std::cout << "+ Torch (" << BOARD_TORCH << "): Reveals invisible DARK AREAS (" << DARK_CHAR << ").";
    Utils::gotoxy(6, 15);
    std::cout << "Doors: Open only when the required keys and switches are set,";
    Utils::gotoxy(6, 16);
    std::cout << "Riddle (" << BOARD_RIDDLE << "): Blocks path! Answer correctly to remove.";
    Utils::gotoxy(6, 17);
    std::cout << "Switch (" << BOARD_SWITCH_ON << ' ' << BOARD_SWITCH_OFF << "): Stepping on it toggles Doors.";
    Utils::gotoxy(6, 18);
    std::cout << "Spring (" << BOARD_SPRING << "): Launches player (High Speed!).";  
    Utils::gotoxy(6, 19);
    std::cout << "Obstacle (" << BOARD_OBSTACLE << "): Heavy! Move by High Speed or Teamwork.";
    Utils::gotoxy(6, 20);
    std::cout << "Teleport (" << BOARD_TELEPORT << "): Move through portals in the room.";

    // Return
    Utils::gotoxy(2, 23);
    std::cout << "Press any key to return.";

    std::cout << std::flush;
    char c = Utils::getChar();   // Wait for user input
}

void Game::showError(const std::string& msg)
{
    Utils::clearScreen();

    std::string line1, line2;
    bool foundNewline = false;

    for (size_t i = 0; i < msg.length(); i++)
    {
        if (msg[i] == '\n')
        {
            foundNewline = true;
            continue;
        }

        if (!foundNewline)
            line1 += msg[i];
        else
            line2 += msg[i];
    }

    std::string errorTitle = "ERROR:";
    int xError = (SCREEN_WIDTH - errorTitle.length()) / 2;
    Utils::gotoxy(xError, 10);
    std::cout << errorTitle;

    int xLine1 = (SCREEN_WIDTH - line1.length()) / 2;
    Utils::gotoxy(xLine1, 12);
    std::cout << line1;

    if (!line2.empty())
    {
        int xLine2 = (SCREEN_WIDTH - line2.length()) / 2;
        Utils::gotoxy(xLine2, 13);
        std::cout << line2;
    }

    std::string pressKey = "Press any key to continue ";
    int xPress = (SCREEN_WIDTH - pressKey.length()) / 2;
    Utils::gotoxy(xPress, 17);
    std::cout << pressKey;

    std::cout << std::flush;
    Utils::getChar();
}

void Game::showMessage(const std::string& msg)
{
    Utils::clearScreen();
    std::string line1, line2;
    bool foundNewline = false;

    for (size_t i = 0; i < msg.length(); i++)
    {
        if (msg[i] == '\n')
        {
            foundNewline = true;
            continue;
        }

        if (!foundNewline)
            line1 += msg[i];
        else
            line2 += msg[i];
    }
    int xLine1 = (SCREEN_WIDTH - line1.length()) / 2;
    Utils::gotoxy(xLine1, 12);
    std::cout << line1;
    if (!line2.empty())
    {
        int xLine2 = (SCREEN_WIDTH - line2.length()) / 2;
        Utils::gotoxy(xLine2, 13);
        std::cout << line2;
    }
    std::string pressKey = "Press any key to continue ";
    int xPress = (SCREEN_WIDTH - pressKey.length()) / 2;
    Utils::gotoxy(xPress, 17);
    std::cout << pressKey;

    std::cout << std::flush;
    Utils::getChar();
}

// Init Functions
void Game::initGame()
{
    screens.clear();
    // --- Set global game state ---
    gameOver = false;
    currRoomID = ROOM1_SCREEN;

    // Set player progress
    roomsDone[PLAYER_1] = roomsDone[PLAYER_2] = 0;
    playerRoom[PLAYER_1] = playerRoom[PLAYER_2] = ROOM1_SCREEN;
    playerFinished[PLAYER_1] = playerFinished[PLAYER_2] = false;

    // --- Build Screens ---
    fixedScreens[MENU_SCREEN].setMap(MENU_MAP);
    fixedScreens[INSTRUCTIONS_SCREEN].setMap(INSTRUCTIONS_MAP);

    // --- Reset players ---
    constexpr char keys1[] = { 'D','X','A','W','S','E' };
    constexpr char keys2[] = { 'L','M','J','I','K','O' };

    players[PLAYER_1].setPlayer(Point(3, 9), '$', keys1);
    players[PLAYER_2].setPlayer(Point(3, 11), '&', keys2);
}

bool Game::loadGameFiles() {  // *Developed with AI assistance*
   
    std::vector<std::string> foundFiles;
    char filenameBuffer[50];

    // --- Find all level files ---
    for (int i = 1; i <= MAX_ROOMS_CAPACITY; ++i)
    {
        snprintf(filenameBuffer, sizeof(filenameBuffer), "adv-world_%02d.screen", i);
        std::ifstream fileCheck(filenameBuffer);
        if (fileCheck.good())
        {
            foundFiles.push_back(filenameBuffer); 
        }
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
    if (!initGameFiles(foundFiles))
    {
        return false;
    }
    return true;
}

bool Game::initGameFiles(std::vector<std::string>& foundFiles)
{
    screens.clear();
    screens.resize(foundFiles.size() + 2);      // +2: index 0 unused, last index reserved for final room
    // Load each screen file
    for (size_t i = 0; i < foundFiles.size(); ++i)
    {
        std::string errorMsg, warningMsg;

        bool success = screens[i + 1].loadScreenFromFile( foundFiles[i], errorMsg, warningMsg );

        if (!success)
        {
            showError(errorMsg);
            return false;
        }
        // Non-fatal warnings are shown immediately
        if (!warningMsg.empty())
        {
            showMessage(warningMsg);
        }
    }
    if (!loadRiddles())
    {
        return false;
    }

    int numRooms = foundFiles.size();  // number of real rooms (excluding dummy)

    for (size_t i = 1; i < screens.size(); ++i)
    {
        std::string errorMsg;

        // Validate doors
        if (!screens[i].validateDoors(numRooms, errorMsg))
        {
            showError(errorMsg);
            return false;
        }

        // Validate legend placement
        if (!screens[i].validateLegendPlacement(errorMsg))
        {
            showError(errorMsg);
            return false;
        }

        // Remove legend area from the playable board
        screens[i].clearLegendAreaFromBoard();
    }
    // Create and append the final screen
    int finalIndex = foundFiles.size() + 1;
    screens[finalIndex].setMap(FINAL_MAP);

    // Set initial room
    currRoomID = ROOM1_SCREEN;     // Set initial room
    return true;
}

bool Game::loadRiddles() {
    std::ifstream file("riddles.txt");
    if (!file.is_open())
    {
        showError("Cannot load riddles file");
        return false;
    }

    int roomID, x, y;
    std::string question, answer, dummy; //dummy for spaces

    while (file >> roomID >> x >> y) {

        std::getline(file, dummy);
        std::getline(file, question);
        std::getline(file, answer);
        Riddle* r = screens[roomID].getRiddleAt(Point(x, y));

        if (!r) {
            showError("Rule refers to non-existing riddle in room " + std::to_string(roomID) + 
                " at (" + std::to_string(x) + "," + std::to_string(y) + ")");
            return false;
        }

        r->setData(question, answer);

    }
    file.close();

    return true;
}

bool Game::loadRiddles(int loadRoomID) {
    std::ifstream file("riddles.txt");
    if (!file.is_open())
    {
        showError("Cannot load riddles file");
        return false;
    }

    int roomID, x, y;
    std::string question, answer, dummy; //dummy for spaces

    while (file >> roomID >> x >> y) {
        std::getline(file, dummy);
        std::getline(file, question);
        std::getline(file, answer);
        if (roomID != loadRoomID) continue;
        Riddle* r = screens[roomID].getRiddleAt(Point(x, y));

        if (!r) {
            Riddle newRiddle(Point(x, y));
            newRiddle.setData(question, answer);
            screens[roomID].addRiddle(newRiddle);
        }
        r->setData(question, answer);
    }
    file.close();
    return true;
}

// UI Display Functions
void Game::displayLegend(const Screen& room)
{
    const LegendArea& legend = room.getLegend();
    if (!legend.exists)
        return;

    int x0 = legend.topLeft.getX();
    int y0 = legend.topLeft.getY();

    // 1. Clear entire legend area
    for (int y = 0; y < LEGEND_HEIGHT; ++y)
    {
        Utils::gotoxy(x0, y0 + y);
        std::cout << std::string(LEGEND_WIDTH, ' ');
    }

    // 2. Draw frame
    // Top border
    Utils::gotoxy(x0, y0);
    std::cout << LEGEND_CORNER
        << std::string(LEGEND_WIDTH - 2, LEGEND_H_BORDER)
        << LEGEND_CORNER;

    // Side borders
    for (int y = 1; y < LEGEND_HEIGHT - 1; ++y)
    {
        Utils::gotoxy(x0, y0 + y);
        std::cout << LEGEND_V_BORDER
            << std::string(LEGEND_WIDTH - 2, ' ')
            << LEGEND_V_BORDER;
    }

    // Bottom border
    Utils::gotoxy(x0, y0 + LEGEND_HEIGHT - 1);
    std::cout << LEGEND_CORNER
        << std::string(LEGEND_WIDTH - 2, LEGEND_H_BORDER)
        << LEGEND_CORNER;

    // 3. Draw content (inside frame)

    int cx = x0 + 1; // content start X
    int cy = y0 + 1; // content start Y

    int colScore = cx;
    int colLives = cx + 8;
    int colInv = cx + 18;

    // --- Header ---
    Utils::gotoxy(cx + 3, cy);

    std::cout << "SCORE  LIVES  INV";

    // Line 2
     // --- Player 1 ---
    Utils::gotoxy(colScore, cy + 1);
    std::cout << "P1: " << players[PLAYER_1].getScore();

    Utils::gotoxy(colLives, cy + 1);
    for (int i = 0; i < players[PLAYER_1].getLife(); ++i)
        std::cout << "<3 ";

    Utils::gotoxy(colInv, cy + 1);
    std::cout << players[PLAYER_1].getInventoryChar();

    // --- Player 2 ---
    Utils::gotoxy(colScore, cy + 2);
    std::cout << "P2: " << players[PLAYER_2].getScore();

    Utils::gotoxy(colLives, cy + 2);
    for (int i = 0; i < players[PLAYER_2].getLife(); ++i)
        std::cout << "<3 ";

    Utils::gotoxy(colInv, cy + 2);
    std::cout << players[PLAYER_2].getInventoryChar();
}

void Game::displayFinalScoreboard() const
{
    int score1 = players[PLAYER_1].getScore();
    int score2 = players[PLAYER_2].getScore();
    int totalScore = score1 + score2;

    // Center the scoreboard horizontally
    const int boxWidth = FINAL_SCOREBOARD_WIDTH;
    const int startX = (SCREEN_WIDTH - boxWidth) / 2;
    const int startY = FINAL_SCOREBOARD_START_Y;

    Utils::gotoxy(startX, startY);
    std::cout << "====================";

    Utils::gotoxy(startX, startY + 1);
    std::cout << "   FINAL SCORES";

    Utils::gotoxy(startX, startY + 2);
    std::cout << "--------------------";

    Utils::gotoxy(startX, startY + 3);
    std::cout << "Player 1 : " << score1;

    Utils::gotoxy(startX, startY + 4);
    std::cout << "Player 2 : " << score2;

    Utils::gotoxy(startX, startY + 5);
    std::cout << "--------------------";

    Utils::gotoxy(startX, startY + 6);
    std::cout << "TEAM SCORE : " << totalScore;

    Utils::gotoxy(startX, startY + 7);
    std::cout << "====================";
}

void Game::drawPlayers()
{
    for (int i = 0; i < NUM_PLAYERS; ++i)
    {
        // if player isn't in current room (moved on to the next one) - no need to draw them
        if (playerRoom[i] != currRoomID || players[i].getDead())  
            continue;

        // draw players present in the current room
        players[i].draw();

    }
}

// Restart Functions

bool Game::restartCurrentRoom()
{
    // Final room does not support restart
    if (isFinalRoom(currRoomID))
    {
        return true;
    }
    // Clear current room state
    screens[currRoomID].clearRoom();
    // Reload room from original file
    if (!reloadRoom(currRoomID))
    {
        return false;
    }
    // Reset players that are currently in this room
    for (int i = 0; i < NUM_PLAYERS; ++i)
    {
        if (playerRoom[i] == currRoomID)
            players[i].resetForRoom();
    }
    return true;
}

bool Game::reloadRoom(int roomID)
{
    // Reloads a specific room from its original source file.

    if (roomID < 0 || roomID >= screens.size())
        return false;

    std::string error, warning;
    const std::string& filename = screens[roomID].getSourceFile();
    // Room has no associated file (e.g., final room)
    if (filename.empty())
    {
        showError("Restarting room failed");
        return false;
    }   
    // Reload screen from file
    if (!screens[roomID].loadScreenFromFile(filename, error, warning)) {
        showError(error);
        return false;
    }
    if (!loadRiddles(roomID))
    {
        return false;
    }
    if (!screens[roomID].validateLegendPlacement(error))
    {
        showError(error);
        return false;
    }
    screens[roomID].clearLegendAreaFromBoard();

    return true;
}

// Helper Functions

void Game::moveRoom(Player& player, int dest)
{
    int idx = (&player == &players[PLAYER_1]) ? PLAYER_1 : PLAYER_2;      // determine which player is moving
    int other = 1 - idx;                             // index of the other player

    // --- Final Room Logic ---
    if (isFinalRoom(dest))
    {
        Point startP = getStartPoint(player, idx);   // starting position inside the final screen

        playerRoom[idx] = dest;      // mark this player in the final screen
        playerFinished[idx] = true;          // player reached the final room

        player.setStartPos(startP);

        // According to the rules: if only one player is finished,
        // we return to the second player
        if (!playerFinished[other])
        {
            currRoomID = playerRoom[other];   
            player.addScore(scoreValue(ScoreEvent::FinishGameFirst));
        }
        else
        {
            currRoomID = dest;         // both players are in the final room
            player.addScore(scoreValue(ScoreEvent::FinishGameSecond));
            gameOver = true;
        }

        return;    // finish handling final-screen transition
    }

    // --- Regular Room Transition ---
    roomsDone[idx]++;                          // player completed one more room
    player.clearInventory();

    Point startP = getStartPoint(player, idx);           // new position in the next room

    playerRoom[idx] = dest;                    // update player's destination room

    player.setStartPos(startP);

    player.addScore(scoreValue(ScoreEvent::OpenDoor));  

    // decide which room should currently be displayed:
    if (roomsDone[PLAYER_1] < roomsDone[PLAYER_2])
        currRoomID = playerRoom[PLAYER_1];       // player 1 is behind
    else if (roomsDone[PLAYER_2] < roomsDone[PLAYER_1])
        currRoomID = playerRoom[PLAYER_2];       // player 2 is behind
    else
        currRoomID = playerRoom[idx];     // same progress - follow the moving player
}

// Calculates the player's starting position in the next room based on the door's position 
Point Game::getStartPoint(Player& player, int idx) const {
   auto& room = screens[currRoomID];

   Point posOnDoor = player.getPos();      // player is standing on the door

   int startY = posOnDoor.getY();          // keep same row as the door
   int startX = (idx == 0?1:2);            // player 0 and 1 don't get the same point

   Point startPos(startX, startY);

   if (!room.isLegendCell(startPos) && room.charAt(startPos) == ' ') {   // if cell is clear  
       return startPos;
   }

   for (int y = 1; y < SCREEN_HEIGHT; y++) {
       for (int x = 1; x < SCREEN_WIDTH; x++) {
           Point p(x, y);
           if (!room.isLegendCell(p) && room.isCellFree(p)) {
               return p;
           }
       }
   }
   return startPos;
}

// Checks if the current player and the other player are about to step into the same cell
bool Game::playersCollide(int idx, const Point& nextPos)
{
    int other = 1 - idx;

    // Only relevant if both players are in the same room 
    if (playerRoom[other] != playerRoom[idx])
        return false;

    Point otherPos = players[other].getPos();
    Point otherNext = players[other].getNextPos();

    Screen& room = screens[currRoomID];

    // No physical contact = no collision
    if (nextPos != otherPos)
        return false;

    Direction myDir = players[idx].getDir();
    Direction otherDir = players[other].getDir();
    bool otherBlocked = (otherNext != otherPos);    // true if other is blocked by an object 

    // Case 1: other player in stay mode = collision
    if (otherDir == STAY)
    {
        players[idx].bumpedInto(players[other]);

        return true;
    }
    
    // Case 2: head-on collision (opposite directions)
    if (Point::areOpposite(myDir, otherDir))
    {
        players[idx].bumpedInto(players[other]);

        return true;
    }

    // Case 3: other player is blocked
    if (otherBlocked)
    {
        players[idx].bumpedInto(players[other]);

        bool blockedByObstacle = room.isObstacle(otherNext);

        if (blockedByObstacle)
        {
            if (chainPushSuccess(idx, myDir, otherNext))
            {
                players[idx].setPushing(true);
                return false;
            }
            return true;
        }
        else
        {
            players[idx].bumpedInto(players[other]);
            return true;    // collision 
        }
    }
    // Otherwise: same direction (chasing) = no collision
    return false;
}

void Game::applyLifeLoss(Player& player)
{
    // Decrease player's life and check if still alive
    bool stillAlive = player.lowerLife();

    // Player has no lives left -> end game
    if (!stillAlive)
    {
        showMessage("Player is dead. Better luck next time... -.-");
        gameOver = true;
        isRunning = false;   
    }
}

// Handle Functions

void Game::handleDoor(Player& player)
{
    Screen& room = screens[currRoomID];
    Point p = player.getPos();

    if (!room.isDoor(p))    // no door at this cell
    {
        return;
    }

    Door* d = room.getDoorAt(p);        // get door object  
    int dest = d->getDestination();

    if (d->checkIsOpen())            // if door is already open move room immidiately   
    {
        moveRoom(player, dest);

    }
    else
    {
        if (!(d->getKeyStatus()))
        {
            if (isMatchingKey(player, room, d)) // if player has a key & key fits the door
            {
                player.clearInventory(); // take the key from player
                d->useKey();             // one less key needed
                player.addScore(scoreValue(ScoreEvent::UseKey));  // adding score

                if (d->getNeededKeys() == 0)   // check if more keys are needed
                {
                    d->updateKeyOK();         // change key flag  
                }
            }
        }
    }

    if (d->getKeyStatus() && d->getSwitchStatus())
    {
        d->open();
        moveRoom(player, dest);
    }
}

void Game::handleSwitch(Player& player)
{
    Screen& room = screens[currRoomID];  
    Point p = player.getPos();

    if (!room.isSwitch(p))   // no switch at this cell
    {
        return;
    }

    Switch* sw = room.getSwitchAt(p);

    sw->toggle();

    // Update switch character on screen
    char c = room.charAt(p);
    room.erase(p);
    char fig = (c == '/' ? 'o' : '/');
    room.drawChar(p, fig);

    int id = sw->getDoorID();

    updateDoorBySwitches(id);

}

bool Game::handleSprings(Player& player)
{                                          // *Logic reviewed with ChatGPT assistance*
    Screen& room = screens[currRoomID];
    Point next = player.getPos().next(player.getDir());  

    // Check spring on target cell
    Spring* sp = room.getSpringAt(next);

    if (!sp)   // If no spring was found in nextPos
    {
        // Player not stepping on a spring link.
        // If no compression has started, nothing to do.
        if (player.getCompression() == 0)
        {
            return false;

        }
        // Player has started compression but changed their direction -> find the spring they were compressing
        Spring* adj = findAdjacentSpring(player.getPos());

        if (!adj)
        {
            // No spring found nearby – fail safely
            player.resetCompression();
            return false;
        }
        // We found the spring -> launch player
        launchPlayer(player, *adj);
        return false;
    }
    else if (sp)  
    {
        Point tip = sp->getTipPos();
        if (next != tip)    
        {
            // Player is colliding into a spring from the side (blocked from moving)
            return true;
        }

        Direction spDir = sp->getDir();    
        Direction playerDir = player.getDir();
        bool compressDir = Point::areOpposite(playerDir, spDir);  

        if (next == tip && !compressDir)  
        {
            // Player is colliding into the tip from the side (blocked also)
            return true;
        }

        if (compressDir && next == tip)  
        {
            // Player is moving into the spring
           
            if (player.isAccelerating())
            {
                // If player is accelerating from another spring (reset accel)
                player.stopAcceleration();
            }
            if (compressSpring(player, *sp)) 
            {
                // curr size > 0 still
                player.move();           // Successfully collapsed a link - move forward

                return true;            // no movement needed in Update
            }
            else
            {
                // Player reached the wall
                launchPlayer(player, *sp);    // No link left to collapse - must launch
                return false;
            }
        }
       
    }

    return false; // no special spring handling

}

void Game::handleBombs() {
    //Updates bomb timers in the current room and triggers explosions
    Screen& room = screens[currRoomID];
    std::vector<Bomb>& bombs = room.getBombs();
    std::vector<Point> toExplode;

    // Check which bombs are ready to explode
    for (auto& bomb : bombs) {
        if (bomb.tick()) {
            toExplode.push_back(bomb.getPos());
        }
    }

    // Explode bombs after iteration
    for (const auto& p : toExplode) {
        explodeBomb(p);
    }
}

bool Game::handleRiddles(Player& player) {
    // Handles interaction between a player and a riddle.

    Screen& room = screens[currRoomID];
    const Point& nextPos = player.getNextPos();

    Riddle* r = room.getRiddleAt(nextPos);           // Check if there is a riddle at the next position
    if (r == nullptr) return true;

    Utils::clearScreen();      // Clear screen before presenting the riddle
    Utils::restoreConsole();
    bool solved = r->solve();
    Utils::initConsole();

    if (solved) {
        // Remove riddle from board and award score
        room.removeRiddleAt(nextPos);
        room.erase(nextPos);
        player.addScore(scoreValue(ScoreEvent::SolveRiddle));
    }
    // Restore game screen after riddle interaction
    Utils::clearScreen();
    return solved;
}

void Game::handleTorch(Player& player)
{
    Screen& room = screens[currRoomID];

    if (player.checkItem() == TORCH)
    {
        room.illuminateMap(player.getPos());
    }

    return;
}

bool Game::handleObstacles(Player& player, const Point& nextPos)
{
    Screen& room = screens[currRoomID];
    Point p = nextPos;

    if (!room.isObstacle(p))   // no obstacle at this cell
    {
        return false;    // player can continue moving 
    }

    Obstacle* ob = room.getObstacleAt(p);

    if (ob == nullptr)
    {
        return false;
    }

    Direction dir = player.getDir();
    int force = calcForce(player, ob, dir);

    if (ob->canBePushed(force))
    {
        auto nextBody = ob->getNextBody(dir);

        if (canMoveObstacle(nextBody, ob))
        {
            room.pushObstacle(*ob, dir);
            player.setPos(p);
            return true;     // player can continue movin
        }
        else
        {
            return true;     // moving not approved
        }
    }
    else
    {
        return true;     // player needs to stop
    }
}

void Game::handleCollectibles(Player& player)
{
    Screen& room = screens[currRoomID];  
    Point p = player.getPos();

    // Player already holds an item – cannot pick up another
    if (!player.inventoryEmpty())
    {
        return;
    }
    else if (!room.isItem(p))   // No collectible item at this cell
    {
        return;
    }
    else if (player.getDisposeFlag())
    {
        return;
    }

    else    // Decide which item is here based on its character
    {
        char c = room.charAt(p);

        if (c == BOARD_KEY)  room.collectKey(player, p);

        if (c == BOARD_BOMB) room.collectBomb(player, p);

        if (c == BOARD_TORCH) room.collectTorch(player, p);
    }
}

bool Game::handleTeleports(Player& player) {
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


bool Game::handleDispose(Player& player)
{                                       
    Screen& room = screens[currRoomID];  

    if (player.inventoryEmpty())
    {
        return false;
    }
    else
    {
        Point p = player.getPos();

        if (room.charAt(p) != ' ')   // Cannot place an item on an occupied cell
        {
            return false;
        }

        const ItemType& type = player.checkItem();
        int index = player.getIndex();

        switch (type)
        {
        case KEY:
        {
            // Take key from storage, place it on the board, and activate it
            Key& k = room.getStoredKey(index);
            k.setPos(p);
            player.clearInventory();
            player.setDisposeFlag(true);
            k.activate();

            break;
        }
        case BOMB:
        {
            // Take bomb from storage and arm it at the player's position
            Bomb& b = room.getStoredBomb(index);
            player.clearInventory();
            player.setDisposeFlag(true);
            b.arm(p);

            break;
        }
        case TORCH:
        {
            Torch& t = room.getStoredTorch(index);
            t.setPos(p);
            player.clearInventory();
            player.setDisposeFlag(true);
            t.activate();
            break;
        }

        default: break;

        }
    }
    return true;
}

bool Game::handleAcceleratedMovement(Player& player, int index)
{
    Screen& room = screens[playerRoom[index]];

    // build up to MAX_SUB_STEPS intermediate positions
    Point sub[Player::MAX_SUB_STEPS];
    int count = player.getAccelerationSubSteps(sub);

    for (int k = 0; k < count; k++)
    {
        player.setPushing(false);

        Point nextPos = sub[k];

        if (room.isLegendCell(nextPos))
        {
            return true;
        }

        if (!room.isCellFree(nextPos))
        {
            // If player's in acceleration, wall stops it
            if (player.isAccelerating())
            {
                player.stopAcceleration();
            }
            break;
        }

        // spring interaction may override direction or launch
        if (handleSprings(player))
            return true;    // stop movement for this loop

        if (handleTeleports(player))
            return true;

        if (handleObstacles(player, nextPos)){  // stop movement for this loop
        
            return true;
        }

        if (!handleRiddles(player)) {
            player.setDirection(STAY);  // stop movement for this loop
            return true;
        }

        // wall collision stops acceleration
        if (!room.isCellFree(nextPos)){
            player.stopAcceleration();
            return true;
        }

        // player collision 
        if (playersCollide(index, nextPos)){
            return true;
        }
        else if (player.isPushing()){
            player.setPos(nextPos);
            break;
        }
        // commit movement
        player.setPos(nextPos);

        // collect items mid-flight
        handleCollectibles(player);

        // toggle switch mid-flight
        handleSwitch(player);

        handleDoor(player);

        // if moved rooms or died — stop movement now
        if (playerRoom[index] != currRoomID || player.getDead())
            return true;
    }
    return false;   // completed accelerated steps normally
}

// Helper to Handle Functions

bool Game::isMatchingKey(Player& player, Screen& room, Door* door) const
{
    if (player.checkItem() != KEY)      // False if player doesn't hold a key
        return false;

    int keyIndex = player.getIndex();      // get access to key object

    const Key& k = room.getStoredKey(keyIndex);

    // compare key's doorID to the door's doorID (not destination room)
    return k.getDoorID() == door->getDoorID();
}

void Game::updateDoorBySwitches(int id)
{                                        
    Screen& room = screens[currRoomID];   
    Door& d = room.getDoorById(id);

    const auto& switches = room.getSwitches();

    int total = 0;     // number of switches linked to this door
    int countOn = 0;   // number of linked switches that are ON

    for (const auto& sw : switches) {
        if (sw.getDoorID() == id)    // if switch [i] is linked to the door
        {
            total++;                         // number of switched linked goes up
            countOn += sw.getState();  // if the switch is on number of countOn goes up
        }
    }

    // Update the switchOK flag by the door's rule
    if (d.getRule() == ALL_ON)      
    {
        d.updateSwitchOK(total == countOn);
    }
    else if (d.getRule() == ALL_OFF)
    {
        d.updateSwitchOK(countOn == 0);
    }
}

void Game::explodeBomb(Point center) {
    Screen& room = screens[currRoomID];    // *Developed with ChatGPT assistance*
    room.removeBombAt(center);

    // getting all points that are in the bomb explosion area
    // the points are in order: from the center - out
    auto blastPattern = Bomb::getBlastPattern(center, BOMB_BLAST_RADIUS);

    for (const auto& ray : blastPattern)
    {
        for (size_t i = 0; i < ray.size(); i++)
        {
            Point p = ray[i];

            if (!Point::checkLimits(p)) break; // the point and those after it in this ray are out of limits
            char c = room.charAt(p);

            if (c == 'W' || c == '=' || c == '|') {
                if (i == 0 && (c == '=' || c == '|')) {       // some barriers ('=' or '|') can be destroyed is those are adjacent to bomb
                    room.erase(p);
                }
                break;
            }
            if (room.removeBombAt(p)) explodeBomb(p);
            room.removeObjectsAt(p); 

            for (auto& player : players) {
                if (player.getPos() == p) {
                    applyLifeLoss(player);
                }
            }
            room.erase(p);
        }
    }
}

Spring* Game::findAdjacentSpring(const Point& pos)
{         
    // Finds a spring adjacent to the given position (checks UP/DOWN/LEFT/RIGHT).
    Screen& room = screens[currRoomID];

    Point up = pos.next(UP);
    Point down = pos.next(DOWN);
    Point left = pos.next(LEFT);
    Point right = pos.next(RIGHT);

    Spring* sp;

    sp = room.getSpringAt(up);
    
    if (sp)
        return sp;
    
    sp = room.getSpringAt(down);
    if (sp)
        return sp;

    sp = room.getSpringAt(left);
    if (sp)
        return sp;

    sp = room.getSpringAt(right);
    if (sp)
        return sp;

    return nullptr;
}

bool Game::compressSpring(Player& player, Spring& sp)
{
    // Compresses the spring by removing its tip and updating player / spring state.
    Screen& room = screens[currRoomID];

    Point tip = sp.getLinkPos(sp.getCurrSize() - 1);

    room.erase(tip);

    sp.decreaseSize();

    player.addCompression();

    return (sp.getCurrSize() > 0 );    // return true if spring still compressible

}

void Game::launchPlayer(Player& player, Spring& sp)
{
    // Total compression force accumulated by the player
    int force = sp.springRelease();

    // Apply acceleration if any compression was done
    if (force > 0)
    {
        player.accel(force, sp.getDir());   // launch in spring release direction
    }

    player.resetCompression();  // clear stored compression count

}

int Game::calcForce(const Player& pusher, const Obstacle* ob, Direction dir) const
{
    int force = pusher.getSpeed();

    int idx = (&pusher == &players[PLAYER_1]) ? PLAYER_1 : PLAYER_2;
    int otherIdx = 1 - idx;
    const Player& other = players[otherIdx];

    // must be in same room
    if (playerRoom[otherIdx] != currRoomID || players[otherIdx].getDead())
        return force;

    // must move in same direction
    if (other.getDir() != dir)
        return force;

    // use snapshot from start of step ONLY
    Point pusherStart = prevPos[idx];
    Point otherStart = prevPos[otherIdx];

    // CASE 1: other player directly pushes the same obstacle
    if (ob->isObBody(otherStart.next(dir)))
    {
        force += other.getSpeed();
        return force;
    }
    // CASE 2: chain push (other pushes pusher, pusher pushes obstacle)
    if (otherStart == pusherStart.next(Point::opposite(dir)))
    {
        force += other.getSpeed();
        return force;
    }
    return force;
}

bool Game::canMoveObstacle(const std::vector<Point>& nextBody, const Obstacle* currOb)
{
    Screen& room = screens[currRoomID];

    for (const Point& p : nextBody)    // Check all body cells of the obstacle
    {
        if (!Point::checkLimits(p))
            return false;

        for (int i = 0; i < NUM_PLAYERS; ++i)
        {
            if (playerRoom[i] != currRoomID)
                continue;

            if (players[i].getPos() == p)
                return false;
        }

        if (room.charAt(p) == ' ')
            continue;

        if (room.isObstacle(p) && currOb->isObBody(p))
            continue;

        return false;

    }

    return true;
}

bool Game::chainPushSuccess(int idx, Direction dir, const Point& obstaclePos)
{
    Screen& room = screens[currRoomID];
    Obstacle* ob = room.getObstacleAt(obstaclePos);
    if (!ob)
        return false;

    // check combined force (existing logic)
    int force = calcForce(players[idx], ob, dir);
    if (force < ob->getSize())
        return false;

    // check obstacle can actually move
    auto nextBody = ob->getNextBody(dir);
    if (!canMoveObstacle(nextBody, ob))
        return false;

    // push is real and will happen
    return true;
}