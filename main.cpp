
#include "logparser.h"
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Starting tests..." << std::endl;
    {
        std::cout << "Test 1: Parsing example.log" << std::endl;
        LogParser parser(std::string(LOGS_DIR) + "/example.log");
        parser.printLogMessages(); 
    }
    {
        std::cout << "Test 2: Parsing actual_log1.log" << std::endl;
        LogParser parser(std::string(LOGS_DIR) + "/actual_log.log");
        parser.printLogMessages(); 
    }
    return 0;
}