cmake_minimum_required(VERSION 3.25)

project(FileOperations)

# compile the lib
add_library(${PROJECT_NAME} STATIC 
	"src/FileOperations.h"
	"src/FileOperations.cpp"
)
target_include_directories(${PROJECT_NAME} PUBLIC "src")

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

add_subdirectory("misc/test/")