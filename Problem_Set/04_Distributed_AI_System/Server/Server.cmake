cmake_minimum_required (VERSION 3.8)

add_subdirectory(${SERVER_BASE_DIR}/ocr)
add_subdirectory(${SERVER_BASE_DIR}/grpc)

add_executable(${CMAKE_PROJECT_NAME} ${SERVER_BASE_DIR}/serverMain.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	ocrLib
	grpc_server
	${OpenCV_LIBS}
	Threads::Threads
	Tesseract::libtesseract
	gRPC::grpc++
    protobuf::libprotobuf
	proto_library
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
	CONFIG_PATH="${CMAKE_SOURCE_DIR}/config.txt"
)