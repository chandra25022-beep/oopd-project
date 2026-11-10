// group-68
#include "grp_68_Grade.h"
#include "grp_68_Exceptions.h"
#include <unordered_map>

struct GradeInfo {
    int points;
    bool pass;
    bool final_flag;
    bool letter;
};

static const std::unordered_map<std::string, GradeInfo>& gradeTable() {
    static const std::unordered_map<std::string, GradeInfo> table = {
        {"A",  {10, true,  true,  true}},
        {"A-", {9,  true,  true,  true}},
        {"B",  {8,  true,  true,  true}},
        {"B-", {7,  true,  true,  true}},
        {"C",  {6,  true,  true,  true}},
        {"C-", {5,  true,  true,  true}},
        {"D",  {4,  true,  true,  true}},
        {"E",  {2,  false, true,  true}},
        {"F",  {0,  false, true,  true}},
        {"I",  {-1, false, false, false}},
        {"S",  {-1, true,  true,  false}},
        {"X",  {-1, false, true,  false}},
    };
    return table;
}

Grade::Grade(std::string sym, int pts, bool pass, bool final_flag, bool letter)
    : symbol_(std::move(sym)), points_(pts), pass_(pass), final_(final_flag), letter_(letter) {}

Grade Grade::fromSymbol(const std::string& symbol) {
    const auto& table = gradeTable();
    auto it = table.find(symbol);
    if (it == table.end()) {
        throw InvalidGradeSymbolException("Invalid grade symbol: " + symbol);
    }
    const auto& info = it->second;
    return Grade(symbol, info.points, info.pass, info.final_flag, info.letter);
}

const std::string& Grade::symbol() const { return symbol_; }
int Grade::points() const { return points_; }
bool Grade::isPass() const { return pass_; }
bool Grade::isFinal() const { return final_; }
bool Grade::isLetter() const { return letter_; }

bool Grade::operator>=(const Grade& o) const {
    return points_ >= o.points_;
}

bool Grade::operator==(const Grade& o) const {
    return symbol_ == o.symbol_;
}

