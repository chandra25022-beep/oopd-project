// group-68
#pragma once
#include "grp_68_Registrar.h"
#include <iostream>
#include <string>
#include <vector>

class CommandParser {
public:
    explicit CommandParser(Registrar& registrar);
    void run(std::istream& in, std::ostream& out, std::ostream& err);

private:
    Registrar& registrar_;
    int lineNumber_;

    void processLine(const std::string& line, std::ostream& out, std::ostream& err);
    void processProgram(const std::vector<std::string>& tokens);
    void processCourse(const std::vector<std::string>& tokens);
    void processStudent(const std::vector<std::string>& tokens);
    void processRegister(const std::vector<std::string>& tokens);
    void processGrade(const std::vector<std::string>& tokens);
    void processConvert(const std::vector<std::string>& tokens);
    void processGpa(const std::vector<std::string>& tokens, std::ostream& out);
    void processAvg(const std::vector<std::string>& tokens, std::ostream& out);
    void processGrad(const std::vector<std::string>& tokens, std::ostream& out);
    void processEligible(const std::vector<std::string>& tokens, std::ostream& out);
    void processTranscript(const std::vector<std::string>& tokens, std::ostream& out);
    void processSave(const std::vector<std::string>& tokens);
    void processLoad(const std::vector<std::string>& tokens);
    void processChange(const std::vector<std::string>& tokens);

    Term parseTerm(const std::string& s);
    std::vector<std::string> tokenize(const std::string& line);
    int parseGpaHundredths(const std::string& s);
};

