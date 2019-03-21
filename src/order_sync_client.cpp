#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include <picosha2.h>

#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using order::OrderReceive;
using order::OrderRequest;
using order::OrderReply;

class OrderClient {
    public:
        OrderClient(std::shared_ptr<Channel> channel) : stub_(OrderReceive::NewStub(channel)) {}

    int SendOrder(int quantity, double price, bool type, std::string inst,
                    std::string user, std::string pass) {
        OrderRequest request;
        
        std::string hash_hex_pass;
        picosha2::hash256_hex_string(pass, hash_hex_pass);
        //std::cout << hash_hex_pass << std::endl;
        request.set_quantity(quantity);
        request.set_price(price);
        request.set_type(type);
        request.set_inst(inst);
        request.set_user(user);
        request.set_pass(hash_hex_pass);

        OrderReply reply;

        ClientContext context;

        Status status = stub_->SendOrder(&context, request, &reply);

        if(status.ok()){
            return 0;
        } else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
        }
    }

    private:
        std::unique_ptr<OrderReceive::Stub> stub_;
};

void Run(std::string user, std::string pass,
         std::string inst, bool type, int quantity, double price) {
    std::string address("0.0.0.0:5000");
    OrderClient client(
        grpc::CreateChannel(
            address, 
            grpc::InsecureChannelCredentials()
        )
    );

    int response;

    //int quantity = 5;
    //double price = 10.0;
    //bool type = 1; // sell order
    //std::string inst ("BTC_DER");
    //std::string user ("dendisuhubdy");
    //std::string pass ("password");

    response = client.SendOrder(quantity, price, type, inst, user, pass);
    std::cout << "Answer received: " << response << std::endl;
}

int main(int argc, char* argv[]){
    if (argc <= 6) {
        std::cout << "Usage: " << argv[0]
                  << " <username> "
                  << " <password> "
                  << " <inst> "
                  << " <type> "
                  << " <quantity> "
                  << " <price>"
                  << std::endl;
        return -1;
    }
    else {
        std::string user (argv[1]);
        std::string pass (argv[2]);
        std::string inst (argv[3]);
        std::string arg_type (argv[4]);
        bool type;
        std::istringstream(arg_type) >> std::boolalpha >> type;
        int quantity = std::atoi(argv[5]);
        double price = std::atof(argv[6]);
        Run(user, pass, inst, type, quantity, price);
    }
    return 0;
}
