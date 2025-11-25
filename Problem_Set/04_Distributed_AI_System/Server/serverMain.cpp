#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "grpc/include/OCRServiceImpl.hpp"

void RunServer(const std::string& server_address) {
	OCRServiceImpl service(0, 100, 500);

	grpc::ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "OCR Server listening on " << server_address << std::endl;

	server->Wait();
}

int main(int argc, char* argv[]) {
	std::string server_address = "0.0.0.0:50051";
	
	if (argc > 1) {
		server_address = argv[1];
	}
	
	std::cout << "Starting OCR gRPC Server..." << std::endl;
	std::cout << "Server address: " << server_address << std::endl;
	
	RunServer(server_address);
	
	return 0;
}
