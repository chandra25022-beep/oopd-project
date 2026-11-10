// group-68
#pragma once
#include <memory>
#include <string>
#include <vector>

class CourseMaterial {
public:
    virtual ~CourseMaterial() = default;
    virtual std::unique_ptr<CourseMaterial> clone() const = 0;
    virtual std::string getType() const = 0;
};

class Syllabus : public CourseMaterial {
    std::string content_;
public:
    explicit Syllabus(const std::string& content);
    std::unique_ptr<CourseMaterial> clone() const override;
    std::string getType() const override;
    const std::string& content() const;
    void setContent(const std::string& content);
};

class LectureNotes : public CourseMaterial {
    std::vector<std::string> notes_;
public:
    explicit LectureNotes(const std::vector<std::string>& notes = {});
    std::unique_ptr<CourseMaterial> clone() const override;
    std::string getType() const override;
    void addNote(const std::string& note);
    const std::vector<std::string>& notes() const;
};

class Assignment : public CourseMaterial {
    std::string title_;
    int maxMarks_;
public:
    Assignment(const std::string& title, int maxMarks);
    std::unique_ptr<CourseMaterial> clone() const override;
    std::string getType() const override;
    const std::string& title() const;
    int maxMarks() const;
};
