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

set(CLIENT_BASE_DIR "${CMAKE_SOURCE_DIR}/Client" CACHE PATH "Client Base Directory")
set(SERVER_BASE_DIR "${CMAKE_SOURCE_DIR}/Server" CACHE PATH "Server Base Directory")

if(${PROJECT_BUILD} STREQUAL "Client")
	include(${CLIENT_BASE_DIR}/Client.cmake)
elseif(${PROJECT_BUILD} STREQUAL "Server")
	include(${SERVER_BASE_DIR}/Server.cmake)
else()
	message(FATAL_ERROR "Invalid Build Target: ${PROJECT_BUILD}")
endif()