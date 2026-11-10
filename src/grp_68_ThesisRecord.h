// group-68
#pragma once
#include "grp_68_Term.h"
#include "grp_68_Grade.h"
#include "grp_68_Exceptions.h"
#include <vector>
#include <optional>

enum class ThesisState {
    REGISTERED,
    IN_PROGRESS,
    AT_RISK,
    TERMINATED,
    FINAL
};

class ThesisRecord {
public:
    explicit ThesisRecord(const Term& startTerm);

    ThesisState state() const;
    const Term& startTerm() const;

    // Submit a progress grade (S or X) for a given term
    // Throws: ThesisTerminatedException if TERMINATED
    //         GradeAlreadyFinalException if FINAL
    //         InvalidThesisTransitionException if term ordering wrong
    //         GradeAlreadyFinalException if same term already has a progress grade
    void submitProgress(const Grade& g, const Term& term);

    // Convert thesis to a letter grade (only from IN_PROGRESS)
    // Throws: InvalidThesisTransitionException if not IN_PROGRESS
    //         GradeAlreadyFinalException if already FINAL
    void convert(const Grade& letter);
    void overrideConvertedGrade(const Grade& letter);

    const Grade& convertedGrade() const;  // only valid if FINAL
    bool hasConvertedGrade() const;

    const std::vector<std::pair<Term, Grade>>& progressGrades() const;

private:
    ThesisState state_;
    Term startTerm_;
    std::vector<std::pair<Term, Grade>> progressGrades_;
    std::optional<Grade> convertedGrade_;
    bool lastWasX_;
};
