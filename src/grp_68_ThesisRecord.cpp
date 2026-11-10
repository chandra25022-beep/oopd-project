// group-68
#include "grp_68_ThesisRecord.h"

ThesisRecord::ThesisRecord(const Term& startTerm)
    : state_(ThesisState::REGISTERED), startTerm_(startTerm), lastWasX_(false) {}

ThesisState ThesisRecord::state() const { return state_; }
const Term& ThesisRecord::startTerm() const { return startTerm_; }

void ThesisRecord::submitProgress(const Grade& g, const Term& term) {
    // Check terminal states
    if (state_ == ThesisState::TERMINATED) {
        throw ThesisTerminatedException("Thesis is terminated");
    }
    if (state_ == ThesisState::FINAL) {
        throw GradeAlreadyFinalException("Thesis already has a final grade");
    }

    // Check term ordering
    if (progressGrades_.empty()) {
        // First progress grade: term must be >= start term
        if (term < startTerm_) {
            throw InvalidThesisTransitionException(
                "Progress term " + term.toString() + " before start term " + startTerm_.toString());
        }
    } else {
        // Subsequent: term must be strictly after last progress term
        const Term& lastTerm = progressGrades_.back().first;
        if (!(term > lastTerm)) {
            throw InvalidThesisTransitionException(
                "Progress term " + term.toString() + " not after previous " + lastTerm.toString());
        }
    }

    // Apply state transition
    if (g.symbol() == "S") {
        state_ = ThesisState::IN_PROGRESS;
        lastWasX_ = false;
    } else {  // X
        if (lastWasX_) {
            state_ = ThesisState::TERMINATED;
        } else {
            state_ = ThesisState::AT_RISK;
        }
        lastWasX_ = true;
    }

    progressGrades_.emplace_back(term, g);
}

void ThesisRecord::convert(const Grade& letter) {
    if (state_ == ThesisState::FINAL) {
        throw GradeAlreadyFinalException("Thesis already converted");
    }
    if (state_ == ThesisState::TERMINATED) {
        throw ThesisTerminatedException("Thesis is terminated");
    }
    if (state_ != ThesisState::IN_PROGRESS) {
        throw InvalidThesisTransitionException(
            "Cannot convert thesis in state other than IN_PROGRESS");
    }
    convertedGrade_ = letter;
    state_ = ThesisState::FINAL;
}

void ThesisRecord::overrideConvertedGrade(const Grade& letter) {
    convertedGrade_ = letter;
    state_ = ThesisState::FINAL;
}

const Grade& ThesisRecord::convertedGrade() const {
    return *convertedGrade_;
}

bool ThesisRecord::hasConvertedGrade() const {
    return convertedGrade_.has_value();
}

const std::vector<std::pair<Term, Grade>>& ThesisRecord::progressGrades() const {
    return progressGrades_;
}

