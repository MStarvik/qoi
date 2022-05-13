cmake_minimum_required(VERSION 3.0.0)
project(qui)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(OpenCV REQUIRED)

add_library(qoi src/qoi.cpp)
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
set_property(TARGET qoi_test PROPERTY CMAKE_CXX_STANDARD 17)
set_property(TARGET qoi_test PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(qoi_fast src/qoi_fast.c)
target_include_directories(qoi_fast
    PUBLIC
        include)
        
add_executable(qoi_fast_test src/qoi_fast_test.cpp)
target_include_directories(qoi_fast_test
    PRIVATE
        ${OpenCV_INCLUDE_DIRS})
target_link_libraries(qoi_fast_test
    PRIVATE
        qoi_fast
        ${OpenCV_LIBS})
set_property(TARGET qoi_fast_test PROPERTY CMAKE_CXX_STANDARD 17)
set_property(TARGET qoi_fast_test PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)