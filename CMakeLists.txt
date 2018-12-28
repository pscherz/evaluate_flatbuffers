cmake_minimum_required(VERSION 3.0)

project(flatbuffers_test2 VERSION 0.1.0 LANGUAGES CXX)

find_package(Flatbuffers PATHS "C:/Users/patri/Documents/Projects/Libs/lib/cmake/flatbuffers")

add_custom_command(
	OUTPUT message_generated.h
	COMMAND flatbuffers::flatc ARGS --gen-object-api --cpp ${CMAKE_CURRENT_SOURCE_DIR}/message.fbs
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS message.fbs)

add_executable(${PROJECT_NAME} main.cpp message_generated.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE flatbuffers::flatbuffers)