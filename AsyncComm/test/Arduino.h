#ifndef __ARDUINO_HH__
#define __ARDUINO_HH__

#include <chrono>

// define some type to the closest one available on arduino
typedef uint8_t byte;
typedef std::string String;
using namespace std::chrono;

unsigned long millis() {
  auto time = system_clock::now(); // get the current time
  auto since_epoch = time.time_since_epoch(); // get the duration since epoch
  auto millis = duration_cast<milliseconds>(since_epoch);
  return millis.count();
}

#endif
