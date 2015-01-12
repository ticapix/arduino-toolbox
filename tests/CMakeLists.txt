find_package(Threads REQUIRED)
find_package(GTest)

set(CMAKE_BUILD_TYPE Debug)


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

set(GMOCK_LIBRARIES "gmock")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual") # various warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # debug, no optimisation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage

# adding source to test executable
add_executable(tests
  test.cpp                 # main
  Arduino.cpp              # define few arduino-like functions
  test-AsyncComm.cpp
  test-RingBuffer.cpp
  test-ATCmd.cpp
  test-Coroutine.cpp
  )

# enable C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../RingBuffer/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../AsyncComm/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../GPRS/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Coroutine/)

target_link_libraries(tests ${GTEST_BOTH_LIBRARIES})
target_link_libraries(tests ${GMOCK_LIBRARIES})
target_link_libraries(tests ${CMAKE_THREAD_LIBS_INIT})

add_test(AllTests tests)
