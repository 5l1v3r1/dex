#include <memory>
#include <iostream>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "order.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

using order::OrderReceive;
using order::OrderRequest;
using order::OrderReply;


class ServerImpl final {
 public:
  ~ServerImpl() {
    server_->Shutdown();
    // Always shutdown the completion queue after the server.
    cq_->Shutdown();
  }

  // There is no shutdown handling in this code.
  void Run() {
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Proceed to the server's main loop.
    HandleRpcs();
  }

 private:
  // Class encompasing the state and logic needed to serve a request.
  class CallData {
   public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(Greeter::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      // Invoke the serving logic right away.
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;
        client.connect("127.0.0.1", 6379, [](const std::string& host, std::size_t port, cpp_redis::connect_state status) {
            if (status == cpp_redis::connect_state::dropped) {
                std::cout << "client disconnected from " << host << ":" << port << std::endl;
                }
        });

        const char * zmq_protocol = "tcp://*:5555";
        zmq_publisher.bind(zmq_protocol);

        // As part of the initial CREATE state, we *request* that the system
        // start processing SendOrder requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSendOrder(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(service_, cq_);

        // The actual processing.
        auto now = std::chrono::system_clock::now();
        
        auto itt = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream ss;
        
        ss << std::put_time(gmtime(&itt), "%FT%TZ");
        
        // requesting value of quantity
        int quantity = request_.quantity();
        // requesting value of price
        double price = request_.price();
        // requesting type
        bool type = request_.type();
        // requesting instrument
        std::string inst = request_.inst();
        // requesting user
        std::string user = request_.user();
        // requesting pass
        std::string pass = request_.pass();
        
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

            // And we are done! Let the gRPC runtime know we've finished, using the
            // memory address of this instance as the uniquely identifying tag for
            // the event.
            status_ = FINISH;
            responder_.Finish(reply_, Status::OK, this);
        } else {
            // And we are done! Let the gRPC runtime know we've finished, using the
            // memory address of this instance as the uniquely identifying tag for
            // the event.
            status_ = FINISH;
            responder_.Finish(reply_, Status::OK, this);
        
        }
      } else {
        GPR_ASSERT(status_ == FINISH);
        // Once in the FINISH state, deallocate ourselves (CallData).
        delete this;
      }
    }

   private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Greeter::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    ServerContext ctx_;

    // What we get from the client.
    OrderRequest request_;
    // What we send back to the client.
    OrderReply reply_;

    //! Enable logging
    cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);
    // CPP redis database
    cpp_redis::client client;
    // Zeromq sockets
    zmq::context_t zmq_context(1);
    zmq::socket_t zmq_publisher (zmq_context, ZMQ_PUB);
    
    // The means to get back to the client.
    ServerAsyncResponseWriter<OrderReply> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
  };

  // This can be run in multiple threads if needed.
  void HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get());
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
      // Block waiting to read the next event from the completion queue. The
      // event is uniquely identified by its tag, which in this case is the
      // memory address of a CallData instance.
      // The return value of Next should always be checked. This return value
      // tells us whether there is any kind of event or cq_ is shutting down.
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::unique_ptr<ServerCompletionQueue> cq_;
  Greeter::AsyncService service_;
  std::unique_ptr<Server> server_;
};

int main(int argc, char** argv) {
  ServerImpl server;
  server.Run();

  return 0;
}
