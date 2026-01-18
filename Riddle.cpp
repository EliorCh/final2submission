#include "Riddle.h"

void Riddle::setData(const std::string& q, const std::string& a)
{
    question = q;
    answer = a;
}

//function helped by GEMINI
void Riddle::printUI() const {
    const int boxWidth = (std::max)(static_cast<int>(question.length()) + 10, 44);
    int startX = Utils::getCenteredX(boxWidth);
    if (startX < 0) startX = 0;
    int startY = (SCREEN_HEIGHT - boxWidth) / 2;
    if (startY < 0) startY = 0;


    auto printBoxRow = [&](int rowOffset, const std::string& content) {
        int padding = boxWidth - 2 - static_cast<int>(content.length());
        int padLeft = padding / 2;
        int padRight = padding - padLeft;

        std::string fullRow = BOARD_RIDDLE + std::string(padLeft, ' ') + content
                             + std::string(padRight, ' ') + BOARD_RIDDLE;

        Utils::print(startX, startY + rowOffset, fullRow);
    };

    Utils::print(startX, startY, std::string(boxWidth, BOARD_RIDDLE));
    printBoxRow(1, "");
    printBoxRow(2, question);
    printBoxRow(3, "");

    std::string prompt = " Answer: ";
    printBoxRow(4, prompt);

    Utils::print(startX, startY + 5, std::string(boxWidth, BOARD_RIDDLE));
    Utils::gotoxy(startX + 1 + static_cast<int>(prompt.length()), startY + 4);
}

bool Riddle::solve() {
    if (solved)
        return true;

    Utils::clearScreen();
    Utils::restoreConsole();

    printUI();

    std::string input;
    std::cin >> input;

    std::string cleanInput = Utils::toUpperCase(input);
    std::string cleanAnswer = Utils::toUpperCase(answer);

    std::string formattedAnswer = "|" + cleanAnswer + "|";
    std::string formattedInput = "|" + cleanInput + "|";

    bool isCorrect = (formattedAnswer.find(formattedInput) != std::string::npos);
    lastInput = input;

    int feedbackRow = 8;
    int indent = 8;
    Utils::gotoxy(indent, feedbackRow);

    if (isCorrect) {
        std::cout << ">>> CORRECT! You may pass. <<<";
        solved = true;
    }
    else {
        std::cout << ">>> WRONG! You shall NOT pass. <<<";
    }

    Utils::gotoxy(indent, feedbackRow + 1);
    std::cout << "Press ENTER to continue...";

    std::cin.ignore();
    std::cin.get();

    // Restore screen after riddle interaction
    Utils::initConsole();
    Utils::clearScreen();

    return solved;
}