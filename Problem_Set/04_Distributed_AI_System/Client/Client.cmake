cmake_minimum_required (VERSION 3.8)

add_subdirectory(${CLIENT_BASE_DIR}/grpc)
add_subdirectory(${CLIENT_BASE_DIR}/Qt)

add_executable(${CMAKE_PROJECT_NAME} ${CLIENT_BASE_DIR}/clientMain.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
	Threads::Threads
	qt_wrapper
	grpc_client
	gRPC::grpc++
    protobuf::libprotobuf
	proto_library
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
	CONFIG_PATH="${CMAKE_SOURCE_DIR}/config.txt"
)