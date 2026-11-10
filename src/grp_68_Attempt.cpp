// group-68
#include "grp_68_Attempt.h"
#include "grp_68_Course.h"

Attempt::Attempt(const std::string& roll, const Course* course, const Term& term)
    : roll_(roll), course_(course), term_(term) {
    if (course->typeKeyword() == "THESIS") {
        thesisRecord_ = std::make_unique<ThesisRecord>(term);
    }
}

const std::string& Attempt::roll() const { return roll_; }
const Course* Attempt::course() const { return course_; }
const Term& Attempt::term() const { return term_; }

void Attempt::setGrade(const Grade& g) {
    if (grade_.has_value()) {
        if (grade_->isFinal()) {
            throw GradeAlreadyFinalException(
                "Final grade already set for " + roll_ + " in " + course_->code());
        }
        // Current grade is I — can replace with a final grade
        if (!g.isFinal()) {
            // Trying to set I again
            throw GradeAlreadyFinalException(
                "Cannot replace I with another I for " + roll_ + " in " + course_->code());
        }
    }
    grade_ = g;
}

bool Attempt::hasGrade() const { return grade_.has_value(); }

bool Attempt::hasFinalGrade() const {
    return grade_.has_value() && grade_->isFinal();
}

const Grade& Attempt::currentGrade() const { return *grade_; }

ThesisRecord* Attempt::thesisRecord() { return thesisRecord_.get(); }
const ThesisRecord* Attempt::thesisRecord() const { return thesisRecord_.get(); }

bool Attempt::isThesis() const { return thesisRecord_ != nullptr; }

bool Attempt::hasCountingGrade() const {
    if (isThesis()) {
        return thesisRecord_->hasConvertedGrade();
    }
    return hasFinalGrade();
}

const Grade& Attempt::countingGrade() const {
    if (isThesis()) {
        return thesisRecord_->convertedGrade();
    }
    return *grade_;
}

bool Attempt::isOpen() const {
    if (isThesis()) {
        ThesisState st = thesisRecord_->state();
        return st != ThesisState::FINAL && st != ThesisState::TERMINATED;
    }
    // Regular/SU: open if no grade or grade is I
    return !hasFinalGrade();
}

