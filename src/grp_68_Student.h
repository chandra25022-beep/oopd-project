// group-68
#pragma once
#include "grp_68_Attempt.h"
#include <string>
#include <vector>
#include <memory>

class Student {
public:
    Student(const std::string& roll, const std::string& name);

    const std::string& roll() const;
    const std::string& name() const;

    void addAttempt(std::unique_ptr<Attempt> attempt);

    // Find attempt for exact (course, term) — for regular/SU GRADE
    Attempt* findAttempt(const std::string& courseCode, const Term& term);

    // Find active thesis attempt for a course — for thesis GRADE/CONVERT
    Attempt* findActiveThesisAttempt(const std::string& courseCode);

    // Find latest thesis attempt (any state) for a course
    Attempt* findLatestThesisAttempt(const std::string& courseCode);

    // Get counting attempt (latest with final/counting grade)
    const Attempt* getCountingAttempt(const std::string& courseCode) const;

    const std::vector<std::unique_ptr<Attempt>>& attempts() const;

    bool hasPassedCourse(const std::string& courseCode) const;
    bool hasOpenRegistration(const std::string& courseCode) const;
    std::vector<const Attempt*> getAttemptsForCourse(const std::string& courseCode) const;
    int creditsInTerm(const Term& term) const;

private:
    std::string roll_;
    std::string name_;
    std::vector<std::unique_ptr<Attempt>> attempts_;
};

