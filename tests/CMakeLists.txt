find_package(Threads REQUIRED)
find_package(GTest)

if(NOT GTEST_FOUND)
  message(FATAL_ERROR "GTest/GMock was not found
 $ cd /usr/src/gtest
 $ sudo cmake -E make_directory build && sudo cmake -E chdir build cmake .. && sudo cmake --build build
 $ sudo cp build/libgtest* /usr/local/lib/ && sudo rm -rf build
 $ cd /usr/src/gmock
 $ sudo cmake -E make_directory build && sudo cmake -E chdir build cmake .. && sudo cmake --build build
 $ sudo cp build/libgmock* /usr/local/lib/ && sudo rm -rf build")
endif()

include_directories(${GTEST_INCLUDE_DIRS})

set(GMOCK_LIBRARIES "-lgmock")

add_executable(tests
  test.cpp                 # main
  Arduino.cpp              # define few arduino-like functions
  test-AsyncComm.cpp
  test-RingBuffer.cpp
  )

add_test(AllTests tests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -ansi -Wno-deprecated -std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RingBuffer/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../AsyncComm/)

target_link_libraries(tests ${GTEST_BOTH_LIBRARIES})
target_link_libraries(tests ${GMOCK_LIBRARIES})
target_link_libraries(tests ${CMAKE_THREAD_LIBS_INIT})
