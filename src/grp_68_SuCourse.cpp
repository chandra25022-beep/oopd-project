// group-68
#include "grp_68_SuCourse.h"
#include "grp_68_Exceptions.h"

void SuCourse::validateGrade(const Grade& g) const {
    // SU courses accept: S, X, I only
    const std::string& sym = g.symbol();
    if (sym != "S" && sym != "X" && sym != "I") {
        throw GradeTypeMismatchException(
            "Grade " + sym + " not accepted for SU course " + code());
    }
}

bool SuCourse::countsInGpa() const { return false; }

bool SuCourse::earnsCredits(const Grade& g) const {
    return g.symbol() == "S";
}

std::string SuCourse::typeKeyword() const { return "SU"; }

