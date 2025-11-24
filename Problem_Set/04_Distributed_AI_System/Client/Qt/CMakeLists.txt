cmake_minimum_required (VERSION 3.8)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

add_library(qt_wrapper STATIC 
	${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/MainWindow.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/ImageResultWidget.hpp
)

target_include_directories(qt_wrapper PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/Client/grpc/include
)

target_link_libraries(qt_wrapper PUBLIC 
	Qt6::Widgets
	Qt6::Core
	Qt6::Gui
	grpc_client
)

set_target_properties(qt_wrapper PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	AUTOMOC ON
)