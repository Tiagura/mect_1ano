cmake_minimum_required(VERSION 3.16)
project(opencv_example)

find_package(OpenCV REQUIRED)

SET(CMAKE_BUILD_TYPE "Release")
#SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -pthread")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

SET(BASE_DIR ${CMAKE_SOURCE_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/../bin)

#EXERCISE 1
add_executable(codec_video_tests codec_video_tests.cpp)
target_link_libraries(codec_video_tests ${OpenCV_LIBS})

