// group-68
#pragma once
#include <string>
#include <stdexcept>

struct Term {
    int year = 0;
    int semester = 0;  // 1, 2, or 3

    Term() = default;
    Term(int y, int s) : year(y), semester(s) {}

    bool operator<(const Term& o) const {
        if (year != o.year) return year < o.year;
        return semester < o.semester;
    }
    bool operator==(const Term& o) const { return year == o.year && semester == o.semester; }
    bool operator!=(const Term& o) const { return !(*this == o); }
    bool operator<=(const Term& o) const { return !(o < *this); }
    bool operator>(const Term& o) const { return o < *this; }
    bool operator>=(const Term& o) const { return !(*this < o); }

    std::string toString() const {
        return std::to_string(year) + "-" + std::to_string(semester);
    }
};
