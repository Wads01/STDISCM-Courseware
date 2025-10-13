cmake_minimum_required (VERSION 3.8)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)

add_library(qt_wrapper STATIC 
	${QT_BASE_DIR}/src/MainWindow.cpp
	${QT_BASE_DIR}/include/MainWindow.hpp
	${QT_BASE_DIR}/src/VoronoiWidget.cpp
	${QT_BASE_DIR}/include/VoronoiWidget.hpp
)

target_include_directories(qt_wrapper PUBLIC
	${QT_BASE_DIR}/include
)

target_link_libraries(qt_wrapper PUBLIC 
	Qt6::Widgets
	Qt6::Core
	Qt6::Gui
)