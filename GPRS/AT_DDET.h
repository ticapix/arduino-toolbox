#ifndef __AT_DDET_H__
#define __AT_DDET_H__

#include <Arduino.h>

namespace AT_DTMF {
enum status : int8_t {
	DISABLE = 0,
	ENABLE = 1,
};

size_t write(char* buff, size_t len, enum AT_DTMF::status status) {
return snprintf(buff, len, "AT+DDET=%d\r\n", status);
}

static constexpr char* EVT = F("+DTMF:");

template<uint16_t BUFFER_SIZE = 0>
static uint8_t parse(StringBuffer<BUFFER_SIZE>& buffer, char* tone) {
buffer.pop_until(EVT);
buffer.pop_while(' ');
*tone = buffer.pop_first();
buffer.pop_until(F("\r\n"));
return 1;
}
}

#endif
