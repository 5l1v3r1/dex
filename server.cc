#include <string>

#include <grpcpp/grpcpp.h>
#include "order.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using order::OrderReceive;
using order::OrderRequest;
using order::OrderReply;


class OrderServiceImplementation final : public OrderReceive::Service {
    Status sendRequest(
        ServerContext* context, 
        const OrderRequest* request, 
        OrderReply* reply
    ) override {
        // Example computation on server
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
        // Show orders
        std::cout << "Order consists of "
                  << "quantity " << quantity
                  << " price " << price
                  << " type " << type
                  << " instrument " << inst
                  << " username " << user
                  << " password " << pass
                  << std::endl;

        int result = 0;
        
        // send result back to client
        reply->set_result(result);
        // return an OK status
        return Status::OK;
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
