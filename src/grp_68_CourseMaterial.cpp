// group-68
#include "grp_68_CourseMaterial.h"

// ── Syllabus ──────────────────────────────────────────────────────────────────

Syllabus::Syllabus(const std::string& content) : content_(content) {}

std::unique_ptr<CourseMaterial> Syllabus::clone() const {
    return std::make_unique<Syllabus>(*this);
}

std::string Syllabus::getType() const { return "Syllabus"; }
const std::string& Syllabus::content() const { return content_; }
void Syllabus::setContent(const std::string& content) { content_ = content; }

// ── LectureNotes ──────────────────────────────────────────────────────────────

LectureNotes::LectureNotes(const std::vector<std::string>& notes) : notes_(notes) {}

std::unique_ptr<CourseMaterial> LectureNotes::clone() const {
    return std::make_unique<LectureNotes>(*this);
}

std::string LectureNotes::getType() const { return "LectureNotes"; }
void LectureNotes::addNote(const std::string& note) { notes_.push_back(note); }
const std::vector<std::string>& LectureNotes::notes() const { return notes_; }

// ── Assignment ────────────────────────────────────────────────────────────────

Assignment::Assignment(const std::string& title, int maxMarks)
    : title_(title), maxMarks_(maxMarks) {}

std::unique_ptr<CourseMaterial> Assignment::clone() const {
    return std::make_unique<Assignment>(*this);
}

std::string Assignment::getType() const { return "Assignment"; }
const std::string& Assignment::title() const { return title_; }
int Assignment::maxMarks() const { return maxMarks_; }

