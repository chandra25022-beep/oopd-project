// group-68
#include "grp_68_Registrar.h"
#include <sstream>
#include <set>
#include <algorithm>

// ── Helpers ───────────────────────────────────────────────────────────────────

Category Registrar::parseCategory(const std::string& s) const {
    if (s == "CORE") return Category::CORE;
    if (s == "ELECTIVE") return Category::ELECTIVE;
    if (s == "MANDATORY_SU") return Category::MANDATORY_SU;
    if (s == "OPEN") return Category::OPEN;
    throw ParseException("Unknown category: " + s);
}

Course* Registrar::findCourse(const std::string& code) {
    auto it = courses_.find(code);
    return it != courses_.end() ? it->second.get() : nullptr;
}

Student* Registrar::findStudent(const std::string& roll) {
    auto it = students_.find(roll);
    return it != students_.end() ? it->second.get() : nullptr;
}

// ── Program ───────────────────────────────────────────────────────────────────

void Registrar::setProgram(const ProgramRules& rules) {
    rules_ = rules;
    programSet_ = true;
}

bool Registrar::hasProgramBeenSet() const { return programSet_; }

// ── Entity creation ───────────────────────────────────────────────────────────

void Registrar::addCourse(const std::string& code, const std::string& type, int credits,
                           const std::string& categoryStr,
                           const std::vector<Prerequisite>& prereqs,
                           const std::vector<std::string>& antireqs) {
    if (courses_.count(code)) {
        throw DuplicateEntityException("Course " + code + " already exists");
    }

    Category cat = parseCategory(categoryStr);

    // Validate: MANDATORY_SU only for SU courses
    if (cat == Category::MANDATORY_SU && type != "SU") {
        throw InvalidCourseException("MANDATORY_SU category only valid for SU courses");
    }

    // Validate credits
    if (type == "SU") {
        if (credits < 0 || credits > 24) {
            throw InvalidCourseException("Credits must be 0-24 for SU course");
        }
    } else {
        if (credits < 1 || credits > 24) {
            throw InvalidCourseException("Credits must be 1-24 for non-SU course");
        }
    }

    // Validate prerequisites exist and are not self-referencing
    for (const auto& pre : prereqs) {
        if (pre.courseCode == code) {
            throw InvalidCourseException("Course cannot be its own prerequisite");
        }
        if (!courses_.count(pre.courseCode)) {
            throw InvalidCourseException("Prerequisite " + pre.courseCode + " does not exist");
        }
    }

    // Validate anti-requisites exist
    for (const auto& anti : antireqs) {
        if (!courses_.count(anti)) {
            throw InvalidCourseException("Anti-requisite " + anti + " does not exist");
        }
    }

    // Create the course
    std::unique_ptr<Course> course;
    if (type == "REGULAR") {
        course = std::make_unique<RegularCourse>(code, credits, cat);
    } else if (type == "SU") {
        course = std::make_unique<SuCourse>(code, credits, cat);
    } else if (type == "THESIS") {
        course = std::make_unique<ThesisCourse>(code, credits, cat);
    } else {
        throw ParseException("Unknown course type: " + type);
    }

    for (const auto& pre : prereqs) {
        course->addPrerequisite(pre);
    }
    for (const auto& anti : antireqs) {
        course->addAntiRequisite(anti);
    }

    courseOrder_.push_back(code);
    courses_[code] = std::move(course);
}

void Registrar::addStudent(const std::string& roll, const std::string& name) {
    if (students_.count(roll)) {
        throw DuplicateEntityException("Student " + roll + " already exists");
    }
    students_[roll] = std::make_unique<Student>(roll, name);
}

// ── GPA computation (integer arithmetic only!) ────────────────────────────────

Registrar::GpaResult Registrar::computeGpa(const Student& student) const {
    long long numerator = 0;
    long long denominator = 0;

    // Collect unique course codes the student has attempts for
    std::set<std::string> seenCourses;
    for (const auto& a : student.attempts()) {
        seenCourses.insert(a->course()->code());
    }

    for (const auto& courseCode : seenCourses) {
        const Attempt* ca = student.getCountingAttempt(courseCode);
        if (!ca) continue;

        const Course* course = ca->course();

        // SU courses never count in GPA
        if (!course->countsInGpa()) continue;

        if (ca->isThesis()) {
            // Thesis: only count if FINAL (has converted grade)
            if (!ca->thesisRecord()->hasConvertedGrade()) continue;
            const Grade& g = ca->thesisRecord()->convertedGrade();
            numerator += static_cast<long long>(course->credits()) * g.points();
            denominator += course->credits();
        } else {
            // Regular: must have final grade
            if (!ca->hasFinalGrade()) continue;
            const Grade& g = ca->countingGrade();
            if (!g.isLetter()) continue;  // safety: skip I/S/X
            numerator += static_cast<long long>(course->credits()) * g.points();
            denominator += course->credits();
        }
    }

    if (denominator == 0) {
        throw NoGpaDataException("No GPA-eligible courses for " + student.roll());
    }

    // Exact rounding: floor((200*num + den) / (2*den))
    int hundredths = static_cast<int>((200LL * numerator + denominator) / (2LL * denominator));

    return {numerator, denominator, hundredths};
}

std::string Registrar::formatGpa(int hundredths) const {
    int intPart = hundredths / 100;
    int fracPart = hundredths % 100;
    std::string result = std::to_string(intPart) + ".";
    if (fracPart < 10) result += "0";
    result += std::to_string(fracPart);
    return result;
}

std::string Registrar::gpaToAvgGrade(int hundredths) const {
    if (hundredths >= 950) return "A";
    if (hundredths >= 850) return "A-";
    if (hundredths >= 750) return "B";
    if (hundredths >= 650) return "B-";
    if (hundredths >= 550) return "C";
    if (hundredths >= 450) return "C-";
    if (hundredths >= 350) return "D";
    if (hundredths >= 100) return "E";
    return "F";
}

// ── Eligibility checks ───────────────────────────────────────────────────────

std::string Registrar::checkEligibilityCode(const Student& student, const Course& course, const Term& term) {
    const std::string& courseCode = course.code();

    // E2: Already passed (unless improvement applies)
    const Attempt* counting = student.getCountingAttempt(courseCode);
    if (counting && counting->countingGrade().isPass()) {
        // Check if improvement is allowed
        bool improvementAllowed = false;
        if (course.typeKeyword() == "REGULAR") {
            const Grade& cg = counting->countingGrade();
            if (cg.symbol() == "C-" || cg.symbol() == "D") {
                // Check improvement not already used
                auto allAttempts = student.getAttemptsForCourse(courseCode);
                int passCount = 0;
                for (const auto* a : allAttempts) {
                    if (a->hasCountingGrade() && a->countingGrade().isPass()) {
                        passCount++;
                    }
                }
                if (passCount <= 1) {
                    improvementAllowed = true;
                }
            }
        }
        if (!improvementAllowed) {
            return "E2";
        }
    }

    // E3: Duplicate registration (open registration exists)
    if (student.hasOpenRegistration(courseCode)) {
        return "E3";
    }

    // E4: Prerequisites not met
    for (const auto& pre : course.prerequisites()) {
        const Course* preCourse = courses_.at(pre.courseCode).get();
        const Attempt* preCounting = student.getCountingAttempt(pre.courseCode);
        if (!preCounting) {
            return "E4";
        }
        const Grade& preGrade = preCounting->countingGrade();
        if (preCourse->typeKeyword() == "SU") {
            // SU prereq: must have S
            if (preGrade.symbol() != "S") {
                return "E4";
            }
        } else {
            // Regular/thesis prereq: grade must be >= minGrade
            Grade minG = Grade::fromSymbol(pre.minGrade);
            if (!(preGrade >= minG)) {
                return "E4";
            }
        }
    }

    // E5: Anti-requisite passed
    for (const auto& anti : course.antiRequisites()) {
        if (student.hasPassedCourse(anti)) {
            return "E5";
        }
    }

    // E6: Credit limit exceeded
    int currentLoad = student.creditsInTerm(term);
    if (currentLoad + course.credits() > rules_.maxload) {
        return "E6";
    }

    // E7: Thesis-specific checks
    if (course.typeKeyword() == "THESIS") {
        // Need enough earned credits
        int earned = earnedCredits(student);
        if (earned < rules_.thesisMinCredits) {
            return "E7";
        }

        // Need sufficient GPA
        try {
            GpaResult gpa = computeGpa(student);
            if (gpa.hundredths < rules_.thesisMinGpaHundredths) {
                return "E7";
            }
        } catch (const NoGpaDataException&) {
            return "E7";
        }

        // No other thesis open or passed
        for (const auto& a : student.attempts()) {
            if (!a->isThesis()) continue;
            // Skip attempts for this same course that are failed
            if (a->isOpen()) {
                return "E7";
            }
            if (a->hasCountingGrade() && a->countingGrade().isPass()) {
                return "E7";
            }
        }
    }

    return "";  // all checks passed
}

void Registrar::checkEligibilityThrow(const Student& student, const Course& course, const Term& term) {
    const std::string& courseCode = course.code();

    // E2
    const Attempt* counting = student.getCountingAttempt(courseCode);
    if (counting && counting->countingGrade().isPass()) {
        bool improvementAllowed = false;
        if (course.typeKeyword() == "REGULAR") {
            const Grade& cg = counting->countingGrade();
            if (cg.symbol() == "C-" || cg.symbol() == "D") {
                auto allAttempts = student.getAttemptsForCourse(courseCode);
                int passCount = 0;
                for (const auto* a : allAttempts) {
                    if (a->hasCountingGrade() && a->countingGrade().isPass()) {
                        passCount++;
                    }
                }
                if (passCount <= 1) {
                    improvementAllowed = true;
                }
            }
        }
        if (!improvementAllowed) {
            throw AlreadyPassedException("Course " + courseCode + " already passed");
        }
    }

    // E3
    if (student.hasOpenRegistration(courseCode)) {
        throw DuplicateRegistrationException("Open registration exists for " + courseCode);
    }

    // E4
    for (const auto& pre : course.prerequisites()) {
        const Course* preCourse = courses_.at(pre.courseCode).get();
        const Attempt* preCounting = student.getCountingAttempt(pre.courseCode);
        if (!preCounting) {
            throw PrerequisiteNotMetException("Prerequisite " + pre.courseCode + " not met");
        }
        const Grade& preGrade = preCounting->countingGrade();
        if (preCourse->typeKeyword() == "SU") {
            if (preGrade.symbol() != "S") {
                throw PrerequisiteNotMetException("Prerequisite " + pre.courseCode + " not met (need S)");
            }
        } else {
            Grade minG = Grade::fromSymbol(pre.minGrade);
            if (!(preGrade >= minG)) {
                throw PrerequisiteNotMetException("Prerequisite " + pre.courseCode + " not met");
            }
        }
    }

    // E5
    for (const auto& anti : course.antiRequisites()) {
        if (student.hasPassedCourse(anti)) {
            throw AntiRequisiteException("Anti-requisite " + anti + " is passed");
        }
    }

    // E6
    int currentLoad = student.creditsInTerm(term);
    if (currentLoad + course.credits() > rules_.maxload) {
        throw CreditLimitExceededException("Credit limit exceeded in " + term.toString());
    }

    // E7
    if (course.typeKeyword() == "THESIS") {
        int earned = earnedCredits(student);
        if (earned < rules_.thesisMinCredits) {
            throw ThesisNotAllowedException("Not enough earned credits for thesis");
        }
        try {
            GpaResult gpa = computeGpa(student);
            if (gpa.hundredths < rules_.thesisMinGpaHundredths) {
                throw ThesisNotAllowedException("GPA too low for thesis");
            }
        } catch (const NoGpaDataException&) {
            throw ThesisNotAllowedException("No GPA data for thesis eligibility");
        }
        for (const auto& a : student.attempts()) {
            if (!a->isThesis()) continue;
            if (a->isOpen()) {
                throw ThesisNotAllowedException("Another thesis is currently open");
            }
            if (a->hasCountingGrade() && a->countingGrade().isPass()) {
                throw ThesisNotAllowedException("A thesis is already passed");
            }
        }
    }
}

// ── Graduation checks ─────────────────────────────────────────────────────────

int Registrar::earnedCredits(const Student& student) const {
    int total = 0;
    std::set<std::string> seen;
    for (const auto& a : student.attempts()) {
        const std::string& cc = a->course()->code();
        if (seen.count(cc)) continue;

        const Attempt* ca = student.getCountingAttempt(cc);
        if (!ca) continue;
        seen.insert(cc);

        if (ca->isThesis()) {
            if (ca->thesisRecord()->hasConvertedGrade() &&
                ca->course()->earnsCredits(ca->thesisRecord()->convertedGrade())) {
                total += ca->course()->credits();
            }
        } else {
            if (ca->hasFinalGrade() && ca->course()->earnsCredits(ca->countingGrade())) {
                total += ca->course()->credits();
            }
        }
    }
    return total;
}

int Registrar::earnedElectiveCredits(const Student& student) const {
    int total = 0;
    std::set<std::string> seen;
    for (const auto& a : student.attempts()) {
        const std::string& cc = a->course()->code();
        if (seen.count(cc)) continue;
        if (a->course()->category() != Category::ELECTIVE) continue;

        const Attempt* ca = student.getCountingAttempt(cc);
        if (!ca) continue;
        seen.insert(cc);

        if (ca->isThesis()) {
            if (ca->thesisRecord()->hasConvertedGrade() &&
                ca->course()->earnsCredits(ca->thesisRecord()->convertedGrade())) {
                total += ca->course()->credits();
            }
        } else {
            if (ca->hasFinalGrade() && ca->course()->earnsCredits(ca->countingGrade())) {
                total += ca->course()->credits();
            }
        }
    }
    return total;
}

std::vector<std::string> Registrar::checkGraduation(const Student& student) const {
    std::vector<std::string> failures;

    // R1: Earned credits >= total
    if (earnedCredits(student) < rules_.totalCredits) {
        failures.push_back("R1");
    }

    // R2: Every CORE course is passed
    for (const auto& code : courseOrder_) {
        const Course* c = courses_.at(code).get();
        if (c->category() != Category::CORE) continue;
        if (!student.hasPassedCourse(code)) {
            failures.push_back("R2");
            break;
        }
    }

    // R3: Passed elective credits >= electiveCredits
    if (earnedElectiveCredits(student) < rules_.electiveCredits) {
        failures.push_back("R3");
    }

    // R4: Every MANDATORY_SU course has counting grade S
    for (const auto& code : courseOrder_) {
        const Course* c = courses_.at(code).get();
        if (c->category() != Category::MANDATORY_SU) continue;
        const Attempt* ca = student.getCountingAttempt(code);
        if (!ca || ca->countingGrade().symbol() != "S") {
            failures.push_back("R4");
            break;
        }
    }

    // R5: A thesis is FINAL with letter >= thesis_min_grade
    {
        bool thesisPassed = false;
        Grade minGrade = Grade::fromSymbol(rules_.thesisMinGrade);
        for (const auto& a : student.attempts()) {
            if (!a->isThesis()) continue;
            if (a->thesisRecord()->state() == ThesisState::FINAL) {
                const Grade& converted = a->thesisRecord()->convertedGrade();
                if (converted >= minGrade) {
                    thesisPassed = true;
                    break;
                }
            }
        }
        if (!thesisPassed) {
            failures.push_back("R5");
        }
    }

    // R6: Rounded GPA >= mingpa
    {
        try {
            GpaResult gpa = computeGpa(student);
            if (gpa.hundredths < rules_.mingpaHundredths) {
                failures.push_back("R6");
            }
        } catch (const NoGpaDataException&) {
            failures.push_back("R6");
        }
    }

    // R7: No regular or SU registration pending or holds I
    for (const auto& a : student.attempts()) {
        if (a->isThesis()) continue;  // thesis not checked for R7
        if (a->isOpen()) {
            failures.push_back("R7");
            break;
        }
    }

    return failures;
}

// ── Commands ──────────────────────────────────────────────────────────────────

void Registrar::registerStudent(const std::string& roll, const std::string& courseCode, const Term& term) {
    // E1
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");
    Course* course = findCourse(courseCode);
    if (!course) throw UnknownCourseException("Course " + courseCode + " not found");

    // E2-E7 (throw on first failure)
    checkEligibilityThrow(*student, *course, term);

    // All checks passed — create the attempt
    auto attempt = std::make_unique<Attempt>(roll, course, term);
    student->addAttempt(std::move(attempt));
}

void Registrar::grade(const std::string& roll, const std::string& courseCode,
                       const Term& term, const std::string& symbol) {
    // E1: find student and course
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");
    Course* course = findCourse(courseCode);
    if (!course) throw UnknownCourseException("Course " + courseCode + " not found");

    // Validate symbol
    Grade g = Grade::fromSymbol(symbol);  // throws InvalidGradeSymbolException

    // Validate grade type for this course
    course->validateGrade(g);  // throws GradeTypeMismatchException

    // Find the attempt
    if (course->typeKeyword() == "THESIS") {
        // For thesis: find the active (or latest) thesis attempt
        Attempt* attempt = student->findLatestThesisAttempt(courseCode);
        if (!attempt) {
            throw NotRegisteredException("No thesis registration for " + roll + " in " + courseCode);
        }
        // The ThesisRecord handles state validation
        attempt->thesisRecord()->submitProgress(g, term);
    } else {
        // For regular/SU: find attempt for exact (course, term)
        Attempt* attempt = student->findAttempt(courseCode, term);
        if (!attempt) {
            throw NotRegisteredException(
                "No registration for " + roll + " in " + courseCode + " for " + term.toString());
        }
        attempt->setGrade(g);
    }
}

void Registrar::convert(const std::string& roll, const std::string& courseCode,
                         const std::string& letter) {
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");
    Course* course = findCourse(courseCode);
    if (!course) throw UnknownCourseException("Course " + courseCode + " not found");

    // Must be a thesis course
    if (course->typeKeyword() != "THESIS") {
        throw GradeTypeMismatchException("CONVERT only valid for thesis courses");
    }

    // Validate the letter grade
    Grade g = Grade::fromSymbol(letter);  // throws InvalidGradeSymbolException
    if (!g.isLetter()) {
        throw GradeTypeMismatchException("CONVERT requires a letter grade, got " + letter);
    }

    // Find thesis attempt
    Attempt* attempt = student->findLatestThesisAttempt(courseCode);
    if (!attempt) {
        throw NotRegisteredException("No thesis registration for " + roll + " in " + courseCode);
    }

    // ThesisRecord handles state validation
    attempt->thesisRecord()->convert(g);
}

// ── Queries ───────────────────────────────────────────────────────────────────

std::string Registrar::queryGpa(const std::string& roll) {
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");

    GpaResult gpa = computeGpa(*student);  // throws NoGpaDataException
    return "GPA " + roll + " " + formatGpa(gpa.hundredths);
}

std::string Registrar::queryAvg(const std::string& roll) {
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");

    GpaResult gpa = computeGpa(*student);
    return "AVG " + roll + " " + gpaToAvgGrade(gpa.hundredths);
}

std::string Registrar::queryGrad(const std::string& roll) {
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");

    std::vector<std::string> failures = checkGraduation(*student);

    if (failures.empty()) {
        return "GRAD " + roll + " YES";
    }

    std::string result = "GRAD " + roll + " NO ";
    for (size_t i = 0; i < failures.size(); ++i) {
        if (i > 0) result += ",";
        result += failures[i];
    }
    return result;
}

std::string Registrar::queryEligible(const std::string& roll, const std::string& courseCode, const Term& term) {
    // E1: throws for unknown student/course
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");
    Course* course = findCourse(courseCode);
    if (!course) throw UnknownCourseException("Course " + courseCode + " not found");

    // E2-E7: report code (don't throw)
    std::string failCode = checkEligibilityCode(*student, *course, term);

    if (failCode.empty()) {
        return "ELIGIBLE " + roll + " " + courseCode + " YES";
    }
    return "ELIGIBLE " + roll + " " + courseCode + " NO " + failCode;
}

std::string Registrar::queryTranscript(const std::string& roll) {
    Student* student = findStudent(roll);
    if (!student) throw UnknownStudentException("Student " + roll + " not found");

    std::ostringstream oss;
    oss << "TRANSCRIPT " << roll << " " << student->name() << "\n";

    for (const auto& a : student->attempts()) {
        oss << a->course()->code() << " " << a->term().toString() << " ";

        if (a->isThesis()) {
            const ThesisRecord* tr = a->thesisRecord();
            // Print progress grades
            for (size_t i = 0; i < tr->progressGrades().size(); ++i) {
                if (i > 0) oss << ",";
                oss << tr->progressGrades()[i].second.symbol()
                    << "(" << tr->progressGrades()[i].first.toString() << ")";
            }
            // Print state
            switch (tr->state()) {
                case ThesisState::REGISTERED:  oss << " REGISTERED"; break;
                case ThesisState::IN_PROGRESS: oss << " IN_PROGRESS"; break;
                case ThesisState::AT_RISK:     oss << " AT_RISK"; break;
                case ThesisState::TERMINATED:  oss << " TERMINATED"; break;
                case ThesisState::FINAL:
                    oss << " FINAL=" << tr->convertedGrade().symbol();
                    break;
            }
        } else {
            if (a->hasGrade()) {
                oss << a->currentGrade().symbol();
            } else {
                oss << "PENDING";
            }
        }
        oss << "\n";
    }

    oss << "END_TRANSCRIPT";
    return oss.str();
}

