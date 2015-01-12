#ifndef __ARDUINO_HH__
#define __ARDUINO_HH__

#include <stdint.h>
#include <stdarg.h>

// define some type to the closest one available on arduino
typedef uint8_t byte;

unsigned long millis();

#define delay(n) usleep(n * 1000);

#endif
