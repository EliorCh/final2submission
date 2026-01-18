#include "Steps.h"
#include <fstream>
#include <sstream>


void Steps::addStep(size_t iteration, char step) {   

    if (!steps.empty() && steps.back().second == step)   // Ignore repeated direction
        return;

    steps.emplace_back(iteration, step); // Adds a step with its iteration
}
Steps* Steps::loadSteps(std::ifstream& file) {
    Steps* steps = new Steps();
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        size_t iteration;
        char key;

        if (!(iss >> iteration >> key)) {
            delete steps;
            return nullptr;
        }

        steps->addStep(iteration, key);
    }
    return steps;
}

bool Steps::saveSteps(const std::string& filename,
    const std::vector<std::string>& screenFiles) const
{
    // Open steps file (overwrites existing file)
    std::ofstream file(filename);
    if (!file)
        return false;

    file << "# screens\n";
    // Write screen files header to ensure replay consistency
    for (const auto& name : screenFiles) {
        file << name << '\n';
    }

    // steps header
    file << "# steps\n";

    // Write recorded steps: iteration and direction
    for (const auto& step : steps) {
        file << step.first << ' ' << step.second << '\n';
    }

    return true;
}