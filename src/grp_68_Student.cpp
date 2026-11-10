// group-68
#include "grp_68_Student.h"
#include "grp_68_Course.h"

Student::Student(const std::string& roll, const std::string& name)
    : roll_(roll), name_(name) {}

const std::string& Student::roll() const { return roll_; }
const std::string& Student::name() const { return name_; }

void Student::addAttempt(std::unique_ptr<Attempt> attempt) {
    attempts_.push_back(std::move(attempt));
}

Attempt* Student::findAttempt(const std::string& courseCode, const Term& term) {
    for (auto& a : attempts_) {
        if (a->course()->code() == courseCode && a->term() == term) {
            return a.get();
        }
    }
    return nullptr;
}

Attempt* Student::findActiveThesisAttempt(const std::string& courseCode) {
    // Search in reverse to find latest active thesis
    for (auto it = attempts_.rbegin(); it != attempts_.rend(); ++it) {
        auto& a = *it;
        if (a->course()->code() == courseCode && a->isThesis() && a->isOpen()) {
            return a.get();
        }
    }
    return nullptr;
}

Attempt* Student::findLatestThesisAttempt(const std::string& courseCode) {
    for (auto it = attempts_.rbegin(); it != attempts_.rend(); ++it) {
        auto& a = *it;
        if (a->course()->code() == courseCode && a->isThesis()) {
            return a.get();
        }
    }
    return nullptr;
}

const Attempt* Student::getCountingAttempt(const std::string& courseCode) const {
    // Latest attempt with a counting grade (final for regular/SU, converted for thesis)
    for (auto it = attempts_.rbegin(); it != attempts_.rend(); ++it) {
        const auto& a = *it;
        if (a->course()->code() == courseCode && a->hasCountingGrade()) {
            return a.get();
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<Attempt>>& Student::attempts() const {
    return attempts_;
}

bool Student::hasPassedCourse(const std::string& courseCode) const {
    const Attempt* ca = getCountingAttempt(courseCode);
    if (!ca) return false;
    return ca->countingGrade().isPass();
}

bool Student::hasOpenRegistration(const std::string& courseCode) const {
    for (const auto& a : attempts_) {
        if (a->course()->code() == courseCode && a->isOpen()) {
            return true;
        }
    }
    return false;
}

std::vector<const Attempt*> Student::getAttemptsForCourse(const std::string& courseCode) const {
    std::vector<const Attempt*> result;
    for (const auto& a : attempts_) {
        if (a->course()->code() == courseCode) {
            result.push_back(a.get());
        }
    }
    return result;
}

int Student::creditsInTerm(const Term& term) const {
    int total = 0;
    for (const auto& a : attempts_) {
        // Thesis counts only in its start term
        if (a->term() == term) {
            total += a->course()->credits();
        }
    }
    return total;
}

