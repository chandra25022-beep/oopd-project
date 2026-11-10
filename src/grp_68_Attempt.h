// group-68
#pragma once
#include "grp_68_Term.h"
#include "grp_68_Grade.h"
#include "grp_68_ThesisRecord.h"
#include "grp_68_Exceptions.h"
#include <string>
#include <memory>
#include <optional>

class Course;  // forward declaration

class Attempt {
public:
    Attempt(const std::string& roll, const Course* course, const Term& term);

    const std::string& roll() const;
    const Course* course() const;
    const Term& term() const;

    // For regular/SU courses
    void setGrade(const Grade& g);
    void overrideGrade(const Grade& g);
    bool hasGrade() const;
    bool hasFinalGrade() const;
    const Grade& currentGrade() const;

    // For thesis courses
    ThesisRecord* thesisRecord();
    const ThesisRecord* thesisRecord() const;
    bool isThesis() const;

    // Counting grade: the grade used for GPA / graduation
    bool hasCountingGrade() const;
    const Grade& countingGrade() const;

    // Is this attempt open? (pending, has I, or thesis not FINAL/TERMINATED)
    bool isOpen() const;

private:
    std::string roll_;
    const Course* course_;
    Term term_;
    std::optional<Grade> grade_;  // for regular/SU
    std::unique_ptr<ThesisRecord> thesisRecord_;  // for thesis
};

