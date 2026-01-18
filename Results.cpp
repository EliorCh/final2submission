#include "Results.h"
#include <fstream>
#include <sstream>
#include <iomanip>


bool Results::ResultEntry::operator==(const ResultEntry& other) const
{
	if (type != other.type)
		return false;

	switch (type)
	{
	case ResultType::ScreenChange:
		return screenId == other.screenId;

	case ResultType::LostLife:
		return true;

	case ResultType::Riddle:
		return riddle == other.riddle &&
			answer == other.answer &&
			correct == other.correct;

	case ResultType::GameEnd:
		return score == other.score;
	}

	return false;
}

Results* Results::loadResults(std::ifstream& file) {
	Results* results = new Results();
	std::string line;

	// Read result entries line by line
	while (std::getline(file, line)) {
		if (line.empty())
			continue;  

		// Parse single result entry
		if (!results->parseResultLine(line)) {
			delete results;
			return nullptr;
		}
	}
	return results;
}

bool Results::parseResultLine(const std::string& line){

	std::istringstream iss(line);
	size_t iteration;
	std::string type;

	// Parse common prefix: iteration + result type
	if (!(iss >> iteration >> type)){
		return false;
	}

	if (type == "ScreenChange") {  // Screen changeP: <iteration> ScreenChange <screenId>
		int screenId;
		if (!(iss >> screenId))
			return false;
		addScreenChange(iteration, screenId);
	}
	else if (type == "LostLife") { // Player lost life: <iteration> LostLife
		addLostLife(iteration);
	}
	else if (type == "GameEnd") {  // Game ended: <iteration> GameEnd <finalScore>
		int finalScore;
		if (!(iss >> finalScore))
			return false;
		addGameEnd(iteration, finalScore);
	}
	else if (type == "Riddle") {         // Riddle interaction:
		std::string riddle, answer;      // <iteration> Riddle "<riddle>" "<answer>" <correct>
		bool correct;

		if (!(iss >> std::quoted(riddle) >> std::quoted(answer) >> correct))
			return false;

		addRiddleRes(iteration, riddle, answer, correct);
	}
	else {   // Unknown result type
		return false;
	}

	return true;
}

bool Results::getRiddleAtIteration(size_t iter, std::string& a) const
{
	for (const auto& [it, e] : results) {
		if (it == iter && e.type == ResultType::Riddle) {
			a = e.answer;   // answer from file
			return true;
		}
	}
	return false;
}

bool Results::saveResults(const std::string& filename,
	const std::vector<std::string>& screenFiles) const 
{
	std::ofstream file(filename);    // Saves results to file (overwrites if exists).
	if (!file)
		return false;

	file << "# screens\n";
	// Write screen files header to ensure replay consistency
	for (const auto& name : screenFiles) {   
		file << name << '\n';
	}

	// results header
	file << "# results\n";
	// Each entry: <iteration> <TYPE> <data...>
	for (const auto& entry : results) {
		size_t iteration = entry.first;
		const ResultEntry& res = entry.second;

		file << iteration << ' ';

		switch (res.type) {
		case ResultType::ScreenChange:
			file << "ScreenChange" << ' ' 
				<< res.screenId;
			break;

		case ResultType::LostLife:
			file << "LostLife";
			break;

		case ResultType::Riddle:
			file << "Riddle" << ' ' 
				<< quoted(res.riddle) << ' ' << quoted(res.answer) << ' ' << res.correct;
			break;

		case ResultType::GameEnd:
			file << "GameEnd" << ' ' 
				<< res.score;
			break;
		}
		file << '\n';
	}
	return true;
}
