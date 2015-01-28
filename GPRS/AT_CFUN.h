#ifndef __AT_CFUN_H__
#define __AT_CFUN_H__

#include <Arduino.h>

namespace AT_CFUN {

enum fun : int8_t {
	MINIMAL = 0,
	FULL = 1,
	DISABLE = 4 // flight mode
};

enum rst : int8_t {
	NO_RESET = 0,
	RESET = 1 // reset modem before applying new level
};

static constexpr char* EVT = F("+CFUN:");

size_t test(char* buff, size_t len) {
	return snprintf(buff, len, "AT+CFUN=?\r\n");
}

size_t read(char* buff, size_t len) {
	return snprintf(buff, len, "AT+CFUN?\r\n");
}

size_t write(char* buff, size_t len, enum AT_CFUN::fun fun, enum AT_CFUN::rst rst = NO_RESET) {
	return snprintf(buff, len, "AT+CFUN=%d,%d\r\n", fun, rst);
}

template<uint16_t BUFFER_SIZE = 0>
uint8_t parse(StringBuffer<BUFFER_SIZE>& buffer, enum AT_CFUN::fun* fun) {
	buffer.pop_until(EVT);
	buffer.pop_while(' ');
	uint8_t count = 0;

	if (buffer[0] == '0') {
		*fun = MINIMAL;
		++count;
	} else if (buffer[0] == '1') {
		*fun = FULL;
		++count;
	} else if (buffer[0] == '4') {
		*fun = DISABLE;
		++count;
	}
	buffer.pop_until(F("\r\n"));
	return count;
}
}
#endif
