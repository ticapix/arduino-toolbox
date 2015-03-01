#ifndef __AT_CPIN_H__
#define __AT_CPIN_H__

#include <Arduino.h>

namespace AT_CPIN {

static constexpr char* EVT = F("+CPIN:");

enum status : int8_t {
	READY,
	SIM_PIN,
	SIM_PUK
};

#include <ATCmd.h>

template<typename T, uint16_t BUFFER_SIZE>
COROUTINE(bool, AtCpinTest,
	CORO_ARG(AtCpinTest, ATCmd<T, BUFFER_SIZE>*, serial)
)


//COROUTINE(int, CountNumber,
//	CORO_ARG(CountNumber, int, limit)
//	CORO_VAR(int, count)
//)
//CORO_START(CountNumber);
//{
//	count = 0;
//	while (count < limit) {
//		++count;
//		YIELD();
//	}
//}
//CORO_RETURN(count);
//CORO_END();


//COROUTINE(int, CountChar,
//	CORO_ARG(CountChar, char, from)
//	CORO_ARG(CountChar, char, to)
//)
//CORO_START(CountChar);
//{
//	AWAIT(new Delay(50 * (to - from)));
//	ASSERT_TRUE(HAS_TIMEOUT());
//	AWAIT((new CountNumber())->set_limit(to - from));
//	ASSERT_EQ(to - from, RESULT(CountNumber));
//	AWAIT(new Delay(0)); // just to check that it gets properly deleted
//}
//CORO_RETURN(0);
//CORO_END();

size_t read(char* buff, size_t len) {
	return snprintf(buff, len, "AT+CPIN?\r\n");
}

size_t write(char* buff, size_t len, const char* pin) {
	return snprintf(buff, len, "AT+CPIN=%s\r\n", pin);
}

template<uint16_t BUFFER_SIZE = 0>
uint8_t parse(StringBuffer<BUFFER_SIZE>& buffer, enum AT_CPIN::status* status) {
	buffer.pop_until(EVT);
	buffer.pop_while(' ');
	uint8_t count = 0;

	if (buffer.starts_with(F("READY"))) {
		*status = READY;
		++count;
	} else if (buffer.starts_with(F("SIM PIN"))) {
		*status = SIM_PIN;
		++count;
	} else if (buffer.starts_with(F("SIM PUK"))) {
		*status = SIM_PUK;
		++count;
	}
	buffer.pop_until(F("\r\n"));
	return count;
}
}

#endif
