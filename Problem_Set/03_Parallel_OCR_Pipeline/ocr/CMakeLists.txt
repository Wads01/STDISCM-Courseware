cmake_minimum_required (VERSION 3.8)

set(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/Producer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Consumer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/OCRPipeline.cpp
)

add_library(ocrLib ${SOURCES})

target_include_directories(ocrLib PUBLIC
	${OpenCV_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ocrLib PUBLIC
	${OpenCV_LIBS}
	Tesseract::libtesseract
	Threads::Threads
)

target_compile_definitions(ocrLib PUBLIC 
	TESSDATA_DIRECTORY=\"${CMAKE_SOURCE_DIR}/tessdata\"
	CLEANED_DIRECTORY=\"${CMAKE_SOURCE_DIR}/cleaned\"
)