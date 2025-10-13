cmake_minimum_required (VERSION 3.8)

find_package(Threads REQUIRED)

add_library(sim STATIC
	${SIM_BASE_DIR}/src/Worker.cpp
	${SIM_BASE_DIR}/src/Sensor.cpp
	${SIM_BASE_DIR}/include/Worker.hpp
	${SIM_BASE_DIR}/include/Sensor.hpp
)

target_include_directories(sim PUBLIC
	${SIM_BASE_DIR}/include
)

target_link_libraries(sim PUBLIC 
	Threads::Threads
)