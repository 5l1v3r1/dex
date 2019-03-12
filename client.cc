#include <string>

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

    int sendRequest(int quantity, double price, bool type, std::string inst) {
        OrderRequest request;

        request.set_quantity(quantity);
        request.set_price(price);
        request.set_type(type);
        request.set_inst(inst);

        OrderReply reply;

        ClientContext context;

        Status status = stub_->sendRequest(&context, request, &reply);

        if(status.ok()){
            std::string ret = reply.result();
            return 0;
        } else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
        }
    }

    private:
        std::unique_ptr<OrderReceive::Stub> stub_;
};

void Run() {
    std::string address("0.0.0.0:5000");
    OrderClient client(
        grpc::CreateChannel(
            address, 
            grpc::InsecureChannelCredentials()
        )
    );

    int response;

    int quantity = 5;
    double price = 10.0;
    bool type = 1; // sell order
    std::string inst ("BTC_DER");

    response = client.sendRequest(quantity, price, type, inst);
    std::cout << "Answer received: " << response << std::endl;
}

int main(int argc, char* argv[]){
    Run();

    return 0;
}
