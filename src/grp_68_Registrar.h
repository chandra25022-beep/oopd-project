// group-68
#pragma once
#include "grp_68_ProgramRules.h"
#include "grp_68_Course.h"
#include "grp_68_Student.h"
#include "grp_68_RegularCourse.h"
#include "grp_68_SuCourse.h"
#include "grp_68_ThesisCourse.h"
#include "grp_68_Exceptions.h"
#include "grp_68_Term.h"
#include "grp_68_Grade.h"
#include <map>
#include <string>
#include <memory>
#include <vector>

class Registrar {
public:
    void setProgram(const ProgramRules& rules);
    bool hasProgramBeenSet() const;

    void addCourse(const std::string& code, const std::string& type, int credits,
                   const std::string& categoryStr,
                   const std::vector<Prerequisite>& prereqs,
                   const std::vector<std::string>& antireqs);
    void addStudent(const std::string& roll, const std::string& name);

    void registerStudent(const std::string& roll, const std::string& courseCode, const Term& term);
    void grade(const std::string& roll, const std::string& courseCode, const Term& term, const std::string& symbol);
    void convert(const std::string& roll, const std::string& courseCode, const std::string& letter);

    // Bonus operations
    void save(const std::string& filename) const;
    void load(const std::string& filename);
    void changeGrade(const std::string& token, const std::string& roll,
                     const std::string& courseCode, const Term& term,
                     const std::string& newSymbol, const std::string& reason);
    const std::vector<std::string>& auditLog() const;
    void reset();

    std::string queryGpa(const std::string& roll);
    std::string queryAvg(const std::string& roll);
    std::string queryGrad(const std::string& roll);
    std::string queryEligible(const std::string& roll, const std::string& courseCode, const Term& term);
    std::string queryTranscript(const std::string& roll);

    Course* findCourse(const std::string& code);
    Student* findStudent(const std::string& roll);

private:
    ProgramRules rules_;
    bool programSet_ = false;
    std::map<std::string, std::unique_ptr<Course>> courses_;
    std::map<std::string, std::unique_ptr<Student>> students_;
    std::vector<std::string> courseOrder_;
    std::vector<std::string> auditLog_;

    struct GpaResult {
        long long numerator;
        long long denominator;
        int hundredths;
    };
    GpaResult computeGpa(const Student& student) const;
    std::string formatGpa(int hundredths) const;
    std::string gpaToAvgGrade(int hundredths) const;

    // Returns empty string if eligible, or the first failing code (E2-E7)
    std::string checkEligibilityCode(const Student& student, const Course& course, const Term& term);
    // Throws the matching exception for the first failing eligibility check
    void checkEligibilityThrow(const Student& student, const Course& course, const Term& term);

    std::vector<std::string> checkGraduation(const Student& student) const;
    int earnedCredits(const Student& student) const;
    int earnedElectiveCredits(const Student& student) const;
    Category parseCategory(const std::string& s) const;
};

