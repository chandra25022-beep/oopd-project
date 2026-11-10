// group-68
#include "grp_68_CommandParser.h"
#include <sstream>
#include <algorithm>

CommandParser::CommandParser(Registrar& registrar)
    : registrar_(registrar), lineNumber_(0) {}

std::vector<std::string> CommandParser::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

Term CommandParser::parseTerm(const std::string& s) {
    auto pos = s.find('-');
    if (pos == std::string::npos || pos == 0 || pos == s.size() - 1) {
        throw ParseException("Invalid term format: " + s);
    }
    int year = 0;
    int sem = 0;
    try {
        year = std::stoi(s.substr(0, pos));
        sem = std::stoi(s.substr(pos + 1));
    } catch (...) {
        throw ParseException("Non-numeric term: " + s);
    }
    if (sem < 1 || sem > 3) {
        throw ParseException("Invalid semester in term: " + s);
    }
    return Term(year, sem);
}

int CommandParser::parseGpaHundredths(const std::string& s) {
    auto dot = s.find('.');
    if (dot == std::string::npos) {
        throw ParseException("GPA must have decimal point: " + s);
    }
    std::string intPart = s.substr(0, dot);
    std::string fracPart = s.substr(dot + 1);
    if (fracPart.size() != 2) {
        throw ParseException("GPA must have exactly 2 decimal places: " + s);
    }
    int ip = 0, fp = 0;
    try {
        ip = std::stoi(intPart);
        fp = std::stoi(fracPart);
    } catch (...) {
        throw ParseException("Non-numeric GPA: " + s);
    }
    return ip * 100 + fp;
}

void CommandParser::run(std::istream& in, std::ostream& out, std::ostream& err) {
    std::string line;
    lineNumber_ = 0;

    while (std::getline(in, line)) {
        lineNumber_++;

        // Strip trailing \r (CRLF support)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Skip blank lines and comments
        std::string trimmed = line;
        // Find first non-whitespace
        auto firstNonSpace = trimmed.find_first_not_of(" \t");
        if (firstNonSpace == std::string::npos) continue;  // blank
        if (trimmed[firstNonSpace] == '#') continue;  // comment

        try {
            processLine(line, out, err);
        } catch (const ErpException& e) {
            out << "ERROR " << lineNumber_ << " " << e.code() << "\n";
            err << e.what() << "\n";
        }
    }
}

void CommandParser::processLine(const std::string& line, std::ostream& out, std::ostream& err) {
    (void)err;
    auto tokens = tokenize(line);
    if (tokens.empty()) return;

    const std::string& cmd = tokens[0];

    if (cmd == "PROGRAM")       { processProgram(tokens); }
    else if (cmd == "COURSE")   { processCourse(tokens); }
    else if (cmd == "STUDENT")  { processStudent(tokens); }
    else if (cmd == "REGISTER") { processRegister(tokens); }
    else if (cmd == "GRADE")    { processGrade(tokens); }
    else if (cmd == "CONVERT")  { processConvert(tokens); }
    else if (cmd == "GPA")      { processGpa(tokens, out); }
    else if (cmd == "AVG")      { processAvg(tokens, out); }
    else if (cmd == "GRAD")     { processGrad(tokens, out); }
    else if (cmd == "ELIGIBLE") { processEligible(tokens, out); }
    else if (cmd == "TRANSCRIPT") { processTranscript(tokens, out); }
    else {
        throw ParseException("Unknown command: " + cmd);
    }
}

void CommandParser::processProgram(const std::vector<std::string>& tokens) {
    ProgramRules rules;

    for (size_t i = 1; i < tokens.size(); ++i) {
        auto eq = tokens[i].find('=');
        if (eq == std::string::npos) {
            throw ParseException("PROGRAM: expected key=value, got " + tokens[i]);
        }
        std::string key = tokens[i].substr(0, eq);
        std::string val = tokens[i].substr(eq + 1);

        try {
            if (key == "total") {
                rules.totalCredits = std::stoi(val);
            } else if (key == "elective") {
                rules.electiveCredits = std::stoi(val);
            } else if (key == "mingpa") {
                rules.mingpaHundredths = parseGpaHundredths(val);
            } else if (key == "thesis_min_grade") {
                // Validate it's a real grade
                Grade::fromSymbol(val);
                rules.thesisMinGrade = val;
            } else if (key == "thesis_min_credits") {
                rules.thesisMinCredits = std::stoi(val);
            } else if (key == "thesis_min_gpa") {
                rules.thesisMinGpaHundredths = parseGpaHundredths(val);
            } else if (key == "maxload") {
                rules.maxload = std::stoi(val);
            } else {
                throw ParseException("Unknown PROGRAM key: " + key);
            }
        } catch (const ParseException&) {
            throw;
        } catch (const ErpException&) {
            throw;
        } catch (...) {
            throw ParseException("Invalid value for " + key + ": " + val);
        }
    }

    registrar_.setProgram(rules);
}

void CommandParser::processCourse(const std::vector<std::string>& tokens) {
    if (tokens.size() < 5) {
        throw ParseException("COURSE requires at least 4 arguments");
    }

    const std::string& code = tokens[1];
    const std::string& type = tokens[2];

    if (type != "REGULAR" && type != "SU" && type != "THESIS") {
        throw ParseException("Unknown course type: " + type);
    }

    int credits = 0;
    try {
        credits = std::stoi(tokens[3]);
    } catch (...) {
        throw ParseException("Non-numeric credits: " + tokens[3]);
    }

    const std::string& categoryStr = tokens[4];

    std::vector<Prerequisite> prereqs;
    std::vector<std::string> antireqs;

    for (size_t i = 5; i < tokens.size(); ++i) {
        if (tokens[i].substr(0, 4) == "PRE=") {
            std::string preStr = tokens[i].substr(4);
            // Parse comma-separated prerequisites
            std::istringstream pss(preStr);
            std::string entry;
            while (std::getline(pss, entry, ',')) {
                auto colon = entry.find(':');
                if (colon != std::string::npos) {
                    prereqs.emplace_back(entry.substr(0, colon), entry.substr(colon + 1));
                } else {
                    prereqs.emplace_back(entry);
                }
            }
        } else if (tokens[i].substr(0, 5) == "ANTI=") {
            std::string antiStr = tokens[i].substr(5);
            std::istringstream ass(antiStr);
            std::string entry;
            while (std::getline(ass, entry, ',')) {
                antireqs.push_back(entry);
            }
        } else {
            throw ParseException("Unexpected token in COURSE: " + tokens[i]);
        }
    }

    registrar_.addCourse(code, type, credits, categoryStr, prereqs, antireqs);
}

void CommandParser::processStudent(const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        throw ParseException("STUDENT requires roll and name");
    }
    registrar_.addStudent(tokens[1], tokens[2]);
}

void CommandParser::processRegister(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw ParseException("REGISTER requires roll, course, and term");
    }
    Term term = parseTerm(tokens[3]);
    registrar_.registerStudent(tokens[1], tokens[2], term);
}

void CommandParser::processGrade(const std::vector<std::string>& tokens) {
    if (tokens.size() < 5) {
        throw ParseException("GRADE requires roll, course, term, and symbol");
    }
    Term term = parseTerm(tokens[3]);
    registrar_.grade(tokens[1], tokens[2], term, tokens[4]);
}

void CommandParser::processConvert(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw ParseException("CONVERT requires roll, course, and letter grade");
    }
    registrar_.convert(tokens[1], tokens[2], tokens[3]);
}

void CommandParser::processGpa(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw ParseException("GPA requires roll");
    }
    out << registrar_.queryGpa(tokens[1]) << "\n";
}

void CommandParser::processAvg(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw ParseException("AVG requires roll");
    }
    out << registrar_.queryAvg(tokens[1]) << "\n";
}

void CommandParser::processGrad(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw ParseException("GRAD requires roll");
    }
    out << registrar_.queryGrad(tokens[1]) << "\n";
}

void CommandParser::processEligible(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 4) {
        throw ParseException("ELIGIBLE requires roll, course, and term");
    }
    Term term = parseTerm(tokens[3]);
    out << registrar_.queryEligible(tokens[1], tokens[2], term) << "\n";
}

void CommandParser::processTranscript(const std::vector<std::string>& tokens, std::ostream& out) {
    if (tokens.size() < 2) {
        throw ParseException("TRANSCRIPT requires roll");
    }
    out << registrar_.queryTranscript(tokens[1]) << "\n";
}

