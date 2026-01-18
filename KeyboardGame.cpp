#include "KeyboardGame.h"

// Local constants for screen indices
constexpr int MENU_SCREEN_IDX = 0;
constexpr int INSTRUCTIONS_SCREEN_IDX = 1;

KeyboardGame::KeyboardGame(bool _saveMode) : GameBase() {
   Utils::initConsole();
   this->saveMode = _saveMode;

   fixedScreens[MENU_SCREEN_IDX].setMap(MENU_MAP);
   fixedScreens[INSTRUCTIONS_SCREEN_IDX].setMap(INSTRUCTIONS_MAP);

   if (_saveMode) {  // recording steps & results
       auto* steps = new Steps();
       steps->setSeed(getGameSeed());
       setSteps(steps);
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
    if (key == ESC_KEY) return pauseGame();

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
    if (!saveMode) return;

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
        char c = static_cast<char>(std::toupper(ch));

        // If ESC is pressed again - we return to the game
        if (c == ESC_KEY)
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
        fixedScreens[MENU_SCREEN_IDX].drawBase();
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

// simplified by Gemini
void KeyboardGame::showInstructions() const
{
    Utils::clearScreen();
    fixedScreens[INSTRUCTIONS_SCREEN_IDX].drawBase();   // Shows the instructions screen

    // print to x,y using lambada
    auto print = [](int x, int y, const std::string& text) {
        Utils::print(x, y, text);
    };

    // Title
    print(30, 2, "=== INSTRUCTIONS ===");

    // Goal & Basics
    print(2, 3, "GOAL: Reach Final Room together! Move through rooms and earn points.");
    print(2, 4, "RESTART ROOM: 'R' || GAME OVER: If any player has 0 Lives.");
    print(2, 5, "POINTS: Key(10) Door(20) Riddle(10) Win(1st:100/2nd:50).");

    // Controls
    print(4, 7, "CONTROLS:         PLAYER 1      PLAYER 2");
    print(4, 8, "Move (U/L/D/R):   W/A/X/D       I/J/M/L");
    print(4, 9, "Stay / Dispose:   S  /  E       K  /  O");

    print(4, 11, "ITEMS (Walk over an item to pick it up, max 1 item per player):");

    struct LegendItem { char icon; std::string name; std::string desc; };

    const LegendItem items[] = {
        { BOARD_KEY,       "Key",      "Collect to open matching doors." },
        { BOARD_BOMB,      "Bomb",     "Explodes in 5 turns. Destroys players & walls." }, // קיצרתי טיפה כדי שייכנס בטוח
        { BOARD_TORCH,     "Torch",    "Reveals invisible DARK AREAS (" + std::string(1, DARK_CHAR) + ")." },
        { ' ',             "Doors",    "Open only when the required keys and switches are set," }, // אין אייקון לדלת כללית כאן
        { BOARD_RIDDLE,    "Riddle",   "Blocks path! Answer correctly to remove." },
        { BOARD_SWITCH_ON, "Switch",   "Stepping on it toggles Doors." }, // המקורי הראה גם ON וגם OFF, כאן שמתי אחד
        { BOARD_SPRING,    "Spring",   "Launches player (High Speed!)." },
        { BOARD_OBSTACLE,  "Obstacle", "Heavy! Move by High Speed or Teamwork." },
        { BOARD_TELEPORT,  "Teleport", "Move through portals in the room." }
    };

    int currentY = 12;
    for (const auto& item : items) {
        Utils::gotoxy(6, currentY++);
        if (item.name == "Doors")
            std::cout << "+ Doors (0-9): " << item.desc;
        else
            std::cout << "+ " << item.name << " (" << item.icon << "): " << item.desc;
    }

    // Return
    print(2, 23, "Press any key to return.");
    std::cout << std::flush;
    Utils::getChar();   // Wait for user input
}
