cmake_minimum_required (VERSION 3.8)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)

add_library(qt_wrapper STATIC 
	${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/MainWindow.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/ImageResultWidget.hpp
)

target_include_directories(qt_wrapper PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(qt_wrapper PUBLIC 
	Qt6::Widgets
	Qt6::Core
	Qt6::Gui
)