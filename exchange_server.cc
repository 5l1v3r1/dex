#include <iostream>
#include <zmq.hpp>
#include <sstream>

#include "zhelpers.hpp"

int main () {
    //  Prepare our context and subscriber
    zmq::context_t context(1);
    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5555");
    std::string inst ("BTC_DER");
    subscriber.setsockopt( ZMQ_SUBSCRIBE, inst);

    while (true) {
        std::string fix_message;
        zmq::message_t message;
        subscriber.recv(&message);
        fix_message = std::string(static_cast<char*>(message.data()), message.size());
        std::cout << fix_message << std::endl;
    }
    return 0;
}

//#include <unistd.h>

//#include "matching_engine.hpp"


//void usage()
//{
    //std::cout << "Exchange Server" << std::endl;
    //std::cout << "Usage: ./exchange_server " << std::endl;
    //std::cout << "Options: " << std::endl;
    //std::cout << "  -i, input file order.csv path. If not specify, default to ../data/orders.csv" << std::endl;
    //std::cout << std::endl;
//}

//int main( int argc, char** argv )
//{
    //string infile = "./tests/orders.csv";
    //int opt;
    //while ((opt = getopt(argc, argv, "i:")) != -1) {
        //switch(opt) {
        //case 'i':
            //infile = optarg;
            //break;
        //default:
            //usage ();
            //return -1;
        //}
    //}

    //Matching::MatchingEngine engine;
    //return engine.run(infile);
//}

