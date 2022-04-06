#include <iostream>
#include "OrderedMap.h"
#include <string>
#include "InputParser.h"

int main(int argc, char* argv[]) {
    InputParser input(argc, argv);
    if (input.cmdOptionExists("-h")) {
        // Do stuff
    }
    const std::string& filename = input.getCmdOption("-f");
    if (!filename.empty()) {
        // Do interesting things ...
    }
    return 0;

	return 0;
}