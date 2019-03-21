#include <string>
#include <iomanip>
#include <ostream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <cpp_redis/cpp_redis>
#include <zmq.hpp>

#include "zhelpers.hpp"

#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using order::OrderReceive;
using order::OrderRequest;
using order::OrderReply;


class OrderServiceImplementation final: public OrderReceive::Service { 
    Status SendOrder(
        ServerContext* context, 
        const OrderRequest* request, 
        OrderReply* reply
    ) override {
        //! Enable logging
        cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);
        // CPP redis database
        cpp_redis::client client;
        // Zeromq sockets
        //zmq::context_t zmq_context(1);
        //zmq::socket_t zmq_publisher (zmq_context, ZMQ_PUB);

        client.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::connect_state status) {
            if (status == cpp_redis::connect_state::dropped) {
                std::cout << "client disconnected from " << host << ":" << port << std::endl;
                }
        });
        //const char * protocol = "tcp://*:5555";
        //zmq_publisher.bind(protocol);

        // Example computation on server
        // timestamp received order
        auto now = std::chrono::system_clock::now();
        
        auto itt = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream ss;
        
        ss << std::put_time(gmtime(&itt), "%FT%TZ");
        
        // requesting value of quantity
        int quantity = request->quantity();
        // requesting value of price
        double price = request->price();
        // requesting type
        bool type = request->type();
        // requesting instrument
        std::string inst = request->inst();
        // requesting user
        std::string user = request->user();
        // requesting pass
        std::string pass = request->pass();
        
        auto get = client.get(user); 
        client.sync_commit();

        std::string pass_hash = get.get().as_string();
        
        if (pass_hash.compare(pass) == 0) {
            // user authenticated
            // user can send order to exchange
            // Check user and password for authentication
            // database 0 in Redis is used for authentication
            std::string fix_message = std::to_string(quantity)
                                      + " "
                                      + std::to_string(price)
                                      + " "
                                      + std::to_string(type)
                                      + " "
                                      + inst
                                      + " "
                                      + user
                                      + " "
                                      + pass;                          
            // std::cout << fix_message << std::endl;
            
            // database 1 is used for orders JSON dumps
            
            // Insert orders into Zeromq queue
            //zmq::message_t zmq_request (sizeof(fix_message));
            //std::memcpy(zmq_request.data (), fix_message.c_str(), sizeof(fix_message));
            //zmq_publisher.send(zmq_request);            
            //s_sendmore (zmq_publisher, inst);
            //s_send (zmq_publisher, fix_message);       
            // Dump values to Redis on database 1
            // We could dumpt the orders into FIX messages strings
            // client.set(ss.str(), fix_message, [](cpp_redis::reply& reply) {});
            //client.sync_commit();

            // Show orders
            std::cout << ss.str()
                      << " timed. Order consists of "
                      << "quantity " << quantity
                      << " price " << price
                      << " type " << type
                      << " instrument " << inst
                      << " username " << user
                      << " password " << pass
                      << std::endl; 

            std::string message{"0"};
            
            // send result back to client
            reply->set_message(message);
            // return an OK status
            return Status::OK;
        }
        else {
            std::string message{"-1"};
            // send message back to client
            reply->set_message(message);
            return Status::OK;
        }

    } 
};

void Run() {
    std::string address("0.0.0.0:5000");
    OrderServiceImplementation service;

    ServerBuilder builder;

    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    Run();

    return 0;
}
