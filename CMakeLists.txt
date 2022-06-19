cmake_minimum_required(VERSION 3.0.0)
project(qui)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(OpenCV REQUIRED)

add_library(qoi src/qoi.c)
target_include_directories(qoi
    PUBLIC
        include)

add_executable(qoi_test src/qoi_test.cpp)
target_include_directories(qoi_test
    PRIVATE
        ${OpenCV_INCLUDE_DIRS})
target_link_libraries(qoi_test
    PRIVATE
        qoi
        ${OpenCV_LIBS})
set_target_properties(qoi_test
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)