cmake_minimum_required(VERSION 3.5.0)
project(Test3 VERSION 0.1.0 LANGUAGES C CXX)

add_executable(Test3 main.cpp)

##########

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(Test3 ${OpenCV_LIBS})