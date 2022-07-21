cmake_minimum_required(VERSION 3.23.2)

project(HTTPD VERSION 1.0.0)

add_compile_options(-Wall)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin/")
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")
add_executable(httpd ${SOURCES})

target_include_directories(httpd PUBLIC headers)
target_link_libraries(httpd)