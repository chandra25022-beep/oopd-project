// group-68
#pragma once
#include "grp_68_Course.h"

class RegularCourse : public Course {
public:
    using Course::Course;
    void validateGrade(const Grade& g) const override;
    bool countsInGpa() const override;
    bool earnsCredits(const Grade& g) const override;
    std::string typeKeyword() const override;
};

