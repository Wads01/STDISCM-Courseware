cmake_minimum_required (VERSION 3.8)

project ("04_Distributed_AI_System")

set(OpenCV_DIR "D:/Misc/opencv/build/x64/vc16/lib" CACHE PATH "OpenCV Lib Directory")
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/msvc2022_64/lib/cmake/Qt6" CACHE PATH "Qt CMake Directory")
set(Tesseract_DIR "D:/Misc/vcpkg/installed/x64-windows/share/tesseract" CACHE PATH "Tesseract CMake Directory")

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

set(CMAKE_DISABLE_IN_SOURCE_BUILDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE_INIT "Debug")

add_subdirectory(Qt)

# Set to either "Client" or "Server"
set(PROJECT_BUILD "Client" CACHE STRING "Build target: Client or Server")
set_property(CACHE PROJECT_BUILD PROPERTY STRINGS "Client" "Server")

if(PROJECT_BUILD STREQUAL "Client")
	include(${CMAKE_SOURCE_DIR}/Client/Client.cmake)
elseif(PROJECT_BUILD STREQUAL "Server")
	include(${CMAKE_SOURCE_DIR}/Server/Server.cmake)
else()
	message(FATAL_ERROR "Invalid Build Target: ${PROJECT_BUILD}")
endif()

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE qt_wrapper)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
	CONFIG_PATH="${CMAKE_SOURCE_DIR}/config.txt"
	DATASET_DIRECTORY="${CMAKE_SOURCE_DIR}/dataset" 
)