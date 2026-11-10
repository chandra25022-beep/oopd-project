// group-68
#pragma once
#include <stdexcept>
#include <string>

class ErpException : public std::runtime_error {
public:
    ErpException(const std::string& code, const std::string& message)
        : std::runtime_error(message), code_(code) {}

    const std::string& code() const { return code_; }

private:
    std::string code_;
};

// ── Input exceptions ──────────────────────────────────────────────────────────

class InputException : public ErpException {
public:
    InputException(const std::string& code, const std::string& message)
        : ErpException(code, message) {}
};

class ParseException : public InputException {
public:
    explicit ParseException(const std::string& message)
        : InputException("E_PARSE", message) {}
};

// ── Entity exceptions ─────────────────────────────────────────────────────────

class EntityException : public ErpException {
public:
    EntityException(const std::string& code, const std::string& message)
        : ErpException(code, message) {}
};

class DuplicateEntityException : public EntityException {
public:
    explicit DuplicateEntityException(const std::string& message)
        : EntityException("E_DUPLICATE", message) {}
};

class UnknownStudentException : public EntityException {
public:
    explicit UnknownStudentException(const std::string& message)
        : EntityException("E_UNKNOWN_STUDENT", message) {}
};

class UnknownCourseException : public EntityException {
public:
    explicit UnknownCourseException(const std::string& message)
        : EntityException("E_UNKNOWN_COURSE", message) {}
};

class InvalidCourseException : public EntityException {
public:
    explicit InvalidCourseException(const std::string& message)
        : EntityException("E_COURSE_DEF", message) {}
};

// ── Grade exceptions ──────────────────────────────────────────────────────────

class GradeException : public ErpException {
public:
    GradeException(const std::string& code, const std::string& message)
        : ErpException(code, message) {}
};

class InvalidGradeSymbolException : public GradeException {
public:
    explicit InvalidGradeSymbolException(const std::string& message)
        : GradeException("E_GRADE_SYMBOL", message) {}
};

class GradeTypeMismatchException : public GradeException {
public:
    explicit GradeTypeMismatchException(const std::string& message)
        : GradeException("E_GRADE_TYPE", message) {}
};

class NotRegisteredException : public GradeException {
public:
    explicit NotRegisteredException(const std::string& message)
        : GradeException("E_NOT_REGISTERED", message) {}
};

class GradeAlreadyFinalException : public GradeException {
public:
    explicit GradeAlreadyFinalException(const std::string& message)
        : GradeException("E_GRADE_FINAL", message) {}
};

class InvalidThesisTransitionException : public GradeException {
public:
    explicit InvalidThesisTransitionException(const std::string& message)
        : GradeException("E_THESIS_TRANSITION", message) {}
};

class ThesisTerminatedException : public GradeException {
public:
    explicit ThesisTerminatedException(const std::string& message)
        : GradeException("E_THESIS_TERMINATED", message) {}
};

// ── Registration exceptions ───────────────────────────────────────────────────

class RegistrationException : public ErpException {
public:
    RegistrationException(const std::string& code, const std::string& message)
        : ErpException(code, message) {}
};

class AlreadyPassedException : public RegistrationException {
public:
    explicit AlreadyPassedException(const std::string& message)
        : RegistrationException("E_ALREADY_PASSED", message) {}
};

class DuplicateRegistrationException : public RegistrationException {
public:
    explicit DuplicateRegistrationException(const std::string& message)
        : RegistrationException("E_DUP_REG", message) {}
};

class PrerequisiteNotMetException : public RegistrationException {
public:
    explicit PrerequisiteNotMetException(const std::string& message)
        : RegistrationException("E_PREREQ", message) {}
};

class AntiRequisiteException : public RegistrationException {
public:
    explicit AntiRequisiteException(const std::string& message)
        : RegistrationException("E_ANTIREQ", message) {}
};

class CreditLimitExceededException : public RegistrationException {
public:
    explicit CreditLimitExceededException(const std::string& message)
        : RegistrationException("E_CREDIT_LIMIT", message) {}
};

class ThesisNotAllowedException : public RegistrationException {
public:
    explicit ThesisNotAllowedException(const std::string& message)
        : RegistrationException("E_THESIS_NOT_ALLOWED", message) {}
};

// ── Computation exceptions ────────────────────────────────────────────────────

class ComputationException : public ErpException {
public:
    ComputationException(const std::string& code, const std::string& message)
        : ErpException(code, message) {}
};

class NoGpaDataException : public ComputationException {
public:
    explicit NoGpaDataException(const std::string& message)
        : ComputationException("E_NO_GPA", message) {}
};
