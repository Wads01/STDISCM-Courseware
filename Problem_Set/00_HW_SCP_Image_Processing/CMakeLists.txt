cmake_minimum_required (VERSION 3.8)

project ("SCP_Image_Processing")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(OpenCV_DIR "D:/Misc/opencv/build/x64/vc16/lib")

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_executable (${CMAKE_PROJECT_NAME} "main.cpp")

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${OpenCV_LIBS} Threads::Threads)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
	DATASET_DIRECTORY=\"${CMAKE_SOURCE_DIR}/dataset\" 
	CLEANED_DIRECTORY=\"${CMAKE_SOURCE_DIR}/cleaned\"
)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
 cmake_policy(SET CMP0141 NEW)
 set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
