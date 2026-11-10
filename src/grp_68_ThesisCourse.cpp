// group-68
#include "grp_68_ThesisCourse.h"
#include "grp_68_Exceptions.h"

void ThesisCourse::validateGrade(const Grade& g) const {
    // Thesis courses accept ONLY S, X (progress grades via GRADE command)
    const std::string& sym = g.symbol();
    if (sym != "S" && sym != "X") {
        throw GradeTypeMismatchException(
            "Grade " + sym + " not accepted for thesis course " + code());
    }
}

bool ThesisCourse::countsInGpa() const { return true; }

bool ThesisCourse::earnsCredits(const Grade& g) const {
    // Earns credits when converted letter is D or better
    return g.isLetter() && g.isPass();
}

std::string ThesisCourse::typeKeyword() const { return "THESIS"; }

