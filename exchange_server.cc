#include <iostream>
#include <sstream>
#include <unistd.h>

#include "matching_engine.hpp"


void usage()
{
    std::cout << "Exchange Server" << std::endl;
    std::cout << "Usage: ./exchange_server " << std::endl;
    std::cout << "Options: " << std::endl;
    std::cout << "  -i, input file order.csv path. If not specify, default to ../data/orders.csv" << std::endl;
    std::cout << std::endl;
}

int main( int argc, char** argv )
{
    string infile = "./tests/orders.csv";
    int opt;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch(opt) {
        case 'i':
            infile = optarg;
            break;
        default:
            usage ();
            return -1;
        }
    }

    Matching::MatchingEngine engine;
    return engine.run(infile);
}
