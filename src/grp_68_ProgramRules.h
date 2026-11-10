// group-68
#pragma once
#include <string>

class ProgramRules {
public:
    int totalCredits = 160;
    int electiveCredits = 24;
    int mingpaHundredths = 500;      // 5.00 stored as 500
    std::string thesisMinGrade = "C";
    int thesisMinCredits = 100;
    int thesisMinGpaHundredths = 600; // 6.00 stored as 600
    int maxload = 24;
};
