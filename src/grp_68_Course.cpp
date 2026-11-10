// group-68
#include "grp_68_Course.h"

Course::Course(const std::string& code, int credits, Category category)
    : code_(code), credits_(credits), category_(category) {}

const std::string& Course::code() const { return code_; }
int Course::credits() const { return credits_; }
Category Course::category() const { return category_; }

void Course::addPrerequisite(const Prerequisite& pre) {
    prerequisites_.push_back(pre);
}

void Course::addAntiRequisite(const std::string& code) {
    antiRequisites_.push_back(code);
}

const std::vector<Prerequisite>& Course::prerequisites() const {
    return prerequisites_;
}

const std::vector<std::string>& Course::antiRequisites() const {
    return antiRequisites_;
}

void Course::addMaterial(std::unique_ptr<CourseMaterial> material) {
    materials_.push_back(std::move(material));
}

std::vector<std::unique_ptr<CourseMaterial>> Course::cloneMaterials() const {
    std::vector<std::unique_ptr<CourseMaterial>> result;
    result.reserve(materials_.size());
    for (const auto& m : materials_) {
        result.push_back(m->clone());
    }
    return result;
}

