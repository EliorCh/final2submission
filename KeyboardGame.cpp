#include "KeyboardGame.h"

KeyboardGame::KeyboardGame(bool _saveMode) :GameBase() {
       Utils::initConsole();
       this->saveMode = _saveMode;

       fixedScreens[MENU_SCREEN].setMap(MENU_MAP);
       fixedScreens[INSTRUCTIONS_SCREEN].setMap(INSTRUCTIONS_MAP);

       if (_saveMode) {  // recording steps & results
           setSteps(new Steps());
           setResults(new Results());   
       }

       setGame();
       showMenu();  

}

KeyboardGame::~KeyboardGame() = default;


void KeyboardGame::handleInput() {
        if (!Utils::hasInput()) return;  // no key pressed this frame

        char ch = Utils::getChar();
        char key = static_cast<char>(std::toupper(ch));

        // If the game is already over (final room):
        // only 'H' should work and return to the main menu
        if (gameOver) {
            if (key == HOME) isRunning = false;      // leave run() and go back to menu
            onGameEnd();
            return;                                // ignore all other keys in final room
        }

        // Pause the game
        if (key == ESC) return pauseGame();

        // Player wants to restart room
        if (key == RESTART) {
            if (!restartCurrentRoom()) isRunning = false;
            return;
        }
        if (processKey(key) && saveMode) {   // In save mode, record gameplay
            getSteps()->addStep(gameCycles, key);
        }
    }

bool KeyboardGame::getRiddleAnswer(Riddle* riddle, bool& outSolved) {
     
    outSolved = riddle->solve();    // Show UI and get user input

    getResults()->addRiddleRes( gameCycles,
        riddle->getQuestion(), riddle->getLastInput(),outSolved );

    return true;
}

void KeyboardGame::onGameEnd()
{
    // Save game data only in save mode
    if (!saveMode)
        return;

    // Retrieve screen source file names from GameBase
    std::vector<std::string> screenFiles = getScreenSourceFiles();

    // Save steps and results files
    bool stepsOk = getSteps()->saveSteps("adv-world.steps", screenFiles);
    bool resultsOk = getResults()->saveResults("adv-world.results", screenFiles);

    // Notify the user if saving failed
    if (!stepsOk || !resultsOk) {
        showError("Error saving game files");
    }
}
   
 void KeyboardGame::onPlayerDeath() {
        showMessage("Player is dead. Better luck next time... -_-");
        gameOver = true;
        isRunning = false;
    }

void KeyboardGame::pauseGame()
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

void KeyboardGame::showMenu() {
    char choice = '\0';
    while (true) {
        fixedScreens[MENU_SCREEN].drawBase();
        std::cout << std::flush;

        char ch = Utils::getChar();
        choice = static_cast<char>(std::toupper(ch));

        switch (choice) {
        case START:                      // Start new game
            initGame();                  // prepares the game - map, objects, players

            if (!loadGameFiles()) break; // file-related error: return to main menu

            run();                       // start game
            break;

        case INSTRUCTIONS:               // Show instructions
            showInstructions();
            break;

        case EXIT:                       // Exit game
            isRunning=false;
            return;

        default:
            Utils::gotoxy(33, 14);
            std::cout << "Invalid choice.";
            Utils::delay(800);
            break;
        }
    }
}

void KeyboardGame::showInstructions()
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
    [[maybe_unused]]char c=Utils::getChar();   // Wait for user input
}
