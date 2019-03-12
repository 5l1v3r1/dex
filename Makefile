CXX = c++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11
LDFLAGS = -L/usr/local/lib -ltacopie `pkg-config --libs protobuf grpc++ cpp_redis`\
           -Wl,-lgrpc++_reflection -Wl,\
           -ldl


GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

all: client server

client: order.pb.o order.grpc.pb.o client.o
	$(CXX) $^ $(LDFLAGS) -o $@

server: order.pb.o order.grpc.pb.o server.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.grpc.pb.cc: %.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	protoc --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h client server
