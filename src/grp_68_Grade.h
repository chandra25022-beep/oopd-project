// group-68
#pragma once
#include <string>

class Grade {
public:
    // Factory: throws InvalidGradeSymbolException if symbol is not in the valid set
    // Valid symbols: A, A-, B, B-, C, C-, D, E, F, I, S, X
    static Grade fromSymbol(const std::string& symbol);

    const std::string& symbol() const;
    int points() const;       // A=10,A-=9,B=8,B-=7,C=6,C-=5,D=4,E=2,F=0; -1 for I,S,X
    bool isPass() const;      // A-D=true, E,F=false, I=false, S=true, X=false
    bool isFinal() const;     // everything except I
    bool isLetter() const;    // A through F (has numeric points, used in GPA)

    bool operator>=(const Grade& o) const; // compare by points (only meaningful for letters)
    bool operator==(const Grade& o) const;

private:
    Grade(std::string sym, int pts, bool pass, bool final_flag, bool letter);
    std::string symbol_;
    int points_;
    bool pass_;
    bool final_;
    bool letter_;
};
