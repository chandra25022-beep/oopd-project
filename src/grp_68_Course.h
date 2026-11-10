// group-68
#pragma once
#include "grp_68_Grade.h"
#include "grp_68_CourseMaterial.h"
#include <string>
#include <vector>
#include <memory>

enum class Category { CORE, ELECTIVE, MANDATORY_SU, OPEN };

struct Prerequisite {
    std::string courseCode;
    std::string minGrade;  // default "D"
    Prerequisite(const std::string& code, const std::string& min = "D")
        : courseCode(code), minGrade(min) {}
};

class Course {
public:
    Course(const std::string& code, int credits, Category category);
    virtual ~Course() = default;

    const std::string& code() const;
    int credits() const;
    Category category() const;

    void addPrerequisite(const Prerequisite& pre);
    void addAntiRequisite(const std::string& code);

    const std::vector<Prerequisite>& prerequisites() const;
    const std::vector<std::string>& antiRequisites() const;

    void addMaterial(std::unique_ptr<CourseMaterial> material);
    std::vector<std::unique_ptr<CourseMaterial>> cloneMaterials() const;

    // Pure virtuals
    virtual void validateGrade(const Grade& g) const = 0;
    virtual bool countsInGpa() const = 0;
    virtual bool earnsCredits(const Grade& g) const = 0;
    virtual std::string typeKeyword() const = 0;

private:
    std::string code_;
    int credits_;
    Category category_;
    std::vector<Prerequisite> prerequisites_;
    std::vector<std::string> antiRequisites_;
    std::vector<std::unique_ptr<CourseMaterial>> materials_;
};

