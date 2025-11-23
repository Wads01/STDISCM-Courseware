cmake_minimum_required (VERSION 3.8)

add_library(ocrLib STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/OCRPipeline.cpp
)

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
	TESSDATA_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/tessdata"
)