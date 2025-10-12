cmake_minimum_required (VERSION 3.8)

project ("WSN_Simulation" VERSION 1.0.0 LANGUAGES CXX)

# Handle MSVC debug information format policy
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_DISABLE_IN_SOURCE_BUILDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE_INIT "Debug")

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/msvc2022_64/lib/cmake/Qt6")
set(QT_BASE_DIR "${CMAKE_SOURCE_DIR}/Qt" CACHE PATH "Qt SRC" FORCE)

add_subdirectory(Qt)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE qt_wrapper)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE CONFIG_PATH="${CMAKE_SOURCE_DIR}/config.txt"
)