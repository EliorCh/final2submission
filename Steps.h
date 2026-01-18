#pragma once
#include <iosfwd>
#include <list>
#include <string>
#include <vector>

class Steps {
private:
    std::list<std::pair<size_t, char>> steps; // pair: <iteration, direction>
    unsigned int randomSeed = 0;

public:
    void addStep(size_t iteration, char step);
    static Steps* loadSteps(std::ifstream& file);
    bool saveSteps(const std::string& filename, const std::vector<std::string>& screenFiles) const;

    bool isEmpty() const { return steps.empty(); }
    bool isNextStepOnIteration(size_t iteration) const {
        return !steps.empty() && steps.front().first == iteration;
    }
    char popStep() {
        char step = steps.front().second;
        steps.pop_front();
        return step;
    }

    void setSeed(unsigned int seed) { randomSeed = seed; };
    static unsigned int readSeedFromFile(const std::string &filename);
};