#define _WIN32_WINNT 0x0601

#include <iostream>
#include <boost/asio.hpp>
#include <string>

#include "OrderedMap.h"
#include "InputParser.h"
#include "Database.h"
#include "Server.h"

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

    try
    {
        boost::asio::io_context io_context;
        Server server(io_context);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
