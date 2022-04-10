#include <iostream>
#include "OrderedMap.h"
#include <string>
#include "InputParser.h"
#include "Database.h"

using std::cout;
using std::endl;

int main(int argc, char* argv[]) {
    InputParser input(argc, argv);
    //if (input.cmdOptionExists("-h")) {
    //}
    /*
    const std::string& filename = input.getCmdOption("-f");

    
    if (filename.empty()) {
        std::cout << "No database file specified. Exiting... " << std::endl;
        return 1;
    }
    */

    auto db = Database();
    db.addCollection("test");

    Collection& collection = db.getCollection("test");

    collection.set("test1", "value1");
    cout << collection.get("test1") << endl;

    return 0;
}