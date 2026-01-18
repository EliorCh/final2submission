#include "Steps.h"
#include <fstream>
#include <sstream>

void Steps::addStep(size_t iteration, char step) {
    if (!steps.empty() && steps.back().second == step)   // Ignore repeated direction
        return;
    steps.emplace_back(iteration, step); // Adds a step with its iteration
}

Steps* Steps::loadSteps(std::ifstream& file) {
    std::string line;

    if (!std::getline(file, line)
        || line != "# steps" ) return nullptr;

    auto* steps = new Steps();

    while (std::getline(file, line)) { // Read steps: "<iteration> <key>"
        if (line.empty()) continue; // ignore blank lines inside the section

        if (line[0] == '#') {
            file.seekg(-static_cast<std::streamoff>(line.length() + 1), std::ios::cur);
            break;
        }

        std::istringstream iss(line);
        size_t iteration;
        char key;

        // Parse a single step line
        if (!(iss >> iteration >> key)) {
            delete steps;
            return nullptr;
        }

        steps->addStep(iteration, key);
    }
    return steps;
}

bool Steps::saveSteps(const std::string& filename,
    const std::vector<std::string>& screenFiles) const {
    // Open steps file (overwrites existing file)
    std::ofstream file(filename);
    if (!file) return false;

    file << randomSeed << '\n'; // seed for the riddle's shuffle
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

unsigned int Steps::readSeedFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return 0;

    unsigned int seedFromFile;
    file >> seedFromFile;
    return seedFromFile;
}