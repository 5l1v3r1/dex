CXX = c++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++14
LDFLAGS = -L/usr/local/lib -ltacopie -lzmq `pkg-config --libs protobuf grpc++ cpp_redis`\
           -Wl,-lgrpc++_reflection -Wl,\
           -ldl


GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

all: order_client order_server exchange_server

exchange_server: exchange_server.o 
	$(CXX) exchange_server.cc $(CXXFLAGS) -lzmq -o exchange_server

order_client: order.pb.o order.grpc.pb.o order_client.o
	$(CXX) $^ $(LDFLAGS) -o $@

order_server: order.pb.o order.grpc.pb.o order_server.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.grpc.pb.cc: %.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	protoc --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h order_client order_server exchange_server
