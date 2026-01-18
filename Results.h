#pragma once
#include <list>
#include <string>
#include <utility>
#include <vector>
#include <fstream>

class Results {
private:
    enum class ResultType { ScreenChange, LostLife, Riddle, GameEnd };

    struct ResultEntry {
        ResultType type;

        int screenId = -1;        // ScreenChange
        std::string riddle;       // Riddle
        std::string answer;       // Riddle
        bool correct = false;     // Riddle
        int score = 0;            // GameEnd

        explicit ResultEntry(ResultType t)  // LostLife
            : type(t) {
        }
        ResultEntry(ResultType t, int screen) //ScreenChange
            : type(t), screenId(screen) {
        }
        ResultEntry(ResultType t, std::string  r, std::string  a, bool c)  // Riddle
            : type(t), riddle(std::move(r)), answer(std::move(a)), correct(c) {
        }
        ResultEntry(ResultType t, int finalScore, bool)  // GameEnd
            : type(t), score(finalScore) {
        }

        bool operator==(const ResultEntry& other) const;
    };
    std::list<std::pair<size_t, ResultEntry>> results;    // pair: <iteration, result entry>

    bool parseResultLine(const std::string& line);

public:
    const std::list<std::pair<size_t, Results::ResultEntry>>&
        getResults() const { return results; }

    bool getRiddleAtIteration(size_t iter,std::string& q, std::string& a, bool& ok) const;

    void addResult(size_t iteration, const ResultEntry& entry) {
        results.emplace_back( iteration, entry );
    }

    void addScreenChange(size_t iteration, int screenId){
        addResult(iteration, ResultEntry(ResultType::ScreenChange, screenId));
    }
    void addLostLife(size_t iteration){
        addResult(iteration, ResultEntry(ResultType::LostLife));
    }
    void addGameEnd(size_t iteration, int finalScore){
        addResult(iteration, ResultEntry(ResultType::GameEnd, finalScore, true));
    }
    void addRiddleRes(size_t iteration, const std::string& riddle, 
        const std::string& answer, bool correct) {
        addResult(iteration, ResultEntry(ResultType::Riddle, riddle, answer, correct));
    }

    static Results* loadResults(std::ifstream& file);
    bool saveResults(const std::string& filename,
        const std::vector<std::string>& screenFiles) const;

};
