cmake_minimum_required (VERSION 2.6)
enable_testing()
project(test-arduino-libs C CXX)

find_package(GTest)
if(NOT GTEST_FOUND)
  message(FATAL_ERROR "GTest was not found. Maybe you have installed the header package and need to compile gtest libs:
 $ cd /usr/src/gtest
 $ sudo cmake -E make_directory build
 $ sudo cmake -E chdir build cmake ..
 $ sudo cmake --build build
 $ ls build/libgtest*
build/libgtest.a  build/libgtest_main.a
 $ sudo cp build/libgtest* /usr/local/lib/
 $ sudo rm -rf build
or look at https://github.com/snikulov/google-test-examples"
    )
endif()
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Threads REQUIRED)

add_executable(tests test.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -ansi -Wno-deprecated -std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(tests ${GTEST_BOTH_LIBRARIES})
target_link_libraries(tests ${CMAKE_THREAD_LIBS_INIT})
add_test(AllTests tests)
