cmake_minimum_required(VERSION 3.10)
project(Minie)

set(CMAKE_CXX_STANDARD 17)

include_directories(includes)

file(GLOB SOURCES "src/*.cpp")

add_executable(engine ${SOURCES})