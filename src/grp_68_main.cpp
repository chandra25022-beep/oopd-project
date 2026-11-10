// group-68
#include "grp_68_Registrar.h"
#include "grp_68_CommandParser.h"
#include <iostream>
#include <cstdlib>

int main() {
    try {
        Registrar registrar;
        CommandParser parser(registrar);
        parser.run(std::cin, std::cout, std::cerr);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FATAL: " << e.what() << std::endl;
        return 2;
    } catch (...) {
        std::cerr << "FATAL: unknown error" << std::endl;
        return 2;
    }
}

