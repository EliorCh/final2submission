#include "Riddle.h"

void Riddle::setData(const std::string& q, const std::string& a)
{
    question = q;
    answer = a;
}

void Riddle::printUI() const {     //function helped by GEMINI
    constexpr char BORDER = '?';
    const int  INDENT = 8;
    const int WIDTH = (std::max)(static_cast<int>(question.length()) + 4, 44);

    auto printRow = [&](std::string const text = "", bool isLeftAlign = false) {
        int textLen = static_cast<int>(text.length());
        int padding = WIDTH - 2 - textLen;

        int padLeft = isLeftAlign ? 1 : padding / 2;
        int padRight = padding - padLeft;

        std::cout << std::string(INDENT, ' ') << BORDER
            << std::string(padLeft, ' ') << text << std::string(padRight, ' ')
            << BORDER << "\n";
        };

    std::cout << "\n\n";
    std::cout << std::string(INDENT, ' ') << std::string(WIDTH, BORDER) << "\n";
    printRow();
    printRow(question);
    printRow();
    printRow("Answer: ", true);
    std::cout << std::string(INDENT, ' ') << std::string(WIDTH, BORDER) << "\n";

    std::string prompt = "Answer: ";
    int promptLen = static_cast<int>(prompt.length());

    Utils::gotoxy(INDENT + 1 + 1 + promptLen, 6);
}

bool Riddle::solve() {
    if (solved)
        return true;

    Utils::clearScreen();
    Utils::restoreConsole();

    printUI();

    std::string input;
    std::cin >> input;

    bool isCorrect = matchRiddleAnswer(answer, input);

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