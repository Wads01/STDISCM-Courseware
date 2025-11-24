cmake_minimum_required (VERSION 3.8)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(grpc_client STATIC 
	${CMAKE_CURRENT_SOURCE_DIR}/src/OCRClient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/OCRClient.hpp
)

target_include_directories(grpc_client PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_BINARY_DIR}/proto/generated
)

target_link_libraries(grpc_client PUBLIC 
	proto_library
	gRPC::grpc++
	protobuf::libprotobuf
	Threads::Threads
	Qt6::Core
)

set_target_properties(grpc_client PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	AUTOMOC ON
)