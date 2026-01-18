#pragma once
#include "Point.h"
#include "Utils.h"
#include <string>
#include <iostream>
#include <utility>
#include <iomanip>


class Riddle {
private:
    Point pos;
    char figure = '?';
    std::string question;
    std::string answer;
    bool solved = false;
    void printUI() const;
    std::string lastInput;

public:
    Riddle() : pos(0, 0)  {}
    explicit Riddle(Point p) : pos(p) {}
    Riddle(Point p, int id, std::string q, std::string a)
        : pos(p), question(std::move(q)), answer(std::move(a)) {}

    void setData(const std::string& q, const std::string& a);
    Point getPos() const { return pos; }
    char getFigure() const { return figure; }
    bool isSolved() const { return solved; }
    std::string getAnswer() const { return answer; }
    std::string getQuestion() const { return question; }
    std::string getLastInput() const { return lastInput; }

    bool solve();

};