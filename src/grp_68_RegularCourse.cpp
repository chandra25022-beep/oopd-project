// group-68
#include "grp_68_RegularCourse.h"
#include "grp_68_Exceptions.h"

void RegularCourse::validateGrade(const Grade& g) const {
    // Regular courses accept: A, A-, B, B-, C, C-, D, E, F, I
    if (!g.isLetter() && g.symbol() != "I") {
        throw GradeTypeMismatchException(
            "Grade " + g.symbol() + " not accepted for regular course " + code());
    }
}

bool RegularCourse::countsInGpa() const { return true; }

bool RegularCourse::earnsCredits(const Grade& g) const {
    // Earns credits when final grade is D or better (pass)
    return g.isLetter() && g.isPass();
}

std::string RegularCourse::typeKeyword() const { return "REGULAR"; }

