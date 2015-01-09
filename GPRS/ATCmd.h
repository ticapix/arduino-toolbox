#ifndef __ATCMD_H__
#define __ATCMD_H__

#include <Arduino.h>

#ifndef PRINT_BUFFER
#define PRINT_BUFFER()
#endif

#define AT_TIMEOUT_MS 500

enum at_cmd_result
	: int8_t {
	EXEC_PENDING = 0, // waiting for more data
	EXEC_TIMEOUT, // timeout
	ERROR_EXEC_INTERNAL_BUFFER_TOO_SMALL, // error while generating cmd
	ERROR_EXEC_WRITING, // serial write failed
	ERROR_EVENT_TYPE_UNKNOWN,
	ERROR_EVENT_FORMAT_UNKNOWN,
	AT_OK,
	AT_ERROR,
	AT_CPIN_READY,
	AT_CPIN_SIM_PIN,
	AT_CFUN_FULL
};

template<uint16_t BUFFER_SIZE>
bool find_string_start_end(StringBuffer<BUFFER_SIZE>& buffer, const char* tok_start, const char* tok_end, uint16_t& off_start, uint16_t& off_end) {
	off_start = buffer.index_of(tok_start);
	if (off_start == StringBuffer<BUFFER_SIZE>::END)
		return false;
	off_end = buffer.index_of(tok_end, off_start);
	if (off_end == StringBuffer<BUFFER_SIZE>::END)
		return false;
	return true;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_check(StringBuffer<BUFFER_SIZE>& buffer) {
	auto pos = buffer.index_of("OK\r\n");
	if (pos != StringBuffer<BUFFER_SIZE>::END) {
		buffer.pop_firsts(pos);
		buffer.pop_until("\r\n");
		return AT_OK;
	}
	pos = buffer.index_of("ERROR\r\n");
	if (pos != StringBuffer<BUFFER_SIZE>::END) {
		buffer.pop_firsts(pos);
		buffer.pop_until("\r\n");
		return AT_ERROR;
	}
	return EXEC_PENDING;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_cfun(StringBuffer<BUFFER_SIZE>& buffer) {
	uint16_t start, end;
	if (!find_string_start_end(buffer, "+CFUN:", "\r\n", start, end))
		return EXEC_PENDING;
	buffer.pop_firsts(start);
	buffer.pop_until(" ");
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	if (buffer.starts_with("1"))
		result = AT_CFUN_FULL;
	buffer.pop_until("\r\n");
	return result;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_cpin(StringBuffer<BUFFER_SIZE>& buffer) {
	uint16_t start, end;
	if (!find_string_start_end(buffer, "+CPIN:", "\r\n", start, end))
		return EXEC_PENDING;
	buffer.pop_firsts(start);
	buffer.pop_until(" ");
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	if (buffer.starts_with("SIM PIN"))
		result = AT_CPIN_SIM_PIN;
	if (buffer.starts_with("READY"))
		result = AT_CPIN_READY;
	buffer.pop_until("\r\n");
	return result;
}

template<typename T, uint16_t BUFFER_SIZE>
class ATCmd {
public:
	ATCmd(T& serial) :
			_serial(serial) {
	}

	typedef StringBuffer<BUFFER_SIZE> Buffer;

	enum at_cmd_result exec(const char* format, ...) {
		enum at_cmd_result result = check_events();
		if (result != EXEC_PENDING)
			return result;
		const uint16_t buff_len = 255;
		char buff[buff_len];
		va_list ap;
		va_start(ap, format);
		size_t len = vsnprintf(buff, 32, format, ap);
		va_end(ap);
		if (len > buff_len)
			return ERROR_EXEC_INTERNAL_BUFFER_TOO_SMALL;
		if (_serial.write(buff, len) == len) {
			_exec_start = millis();
			return EXEC_PENDING;
		}
		return ERROR_EXEC_WRITING;
	}

	enum at_cmd_result check_events() {
		for (uint8_t count = 0; at_events[count] != nullptr; ++count) {
			auto result = at_events[count](buffer);
			if (result != EXEC_PENDING)
				return result;
		}
		return EXEC_PENDING;
	}

	enum at_cmd_result check_status(unsigned long timeout = AT_TIMEOUT_MS) {
		if (millis() - _exec_start >= timeout)
			return EXEC_TIMEOUT;
		while (_serial.available()) {
			buffer.append(_serial.read());
		}
		enum at_cmd_result result = check_events();
		if (result != EXEC_PENDING)
			return result;
		return at_check(buffer);
	}

	Buffer buffer;

private:
	T& _serial;
	unsigned long _exec_start; // initialized when a new command is executed

	static constexpr enum at_cmd_result (*at_events[])(StringBuffer<BUFFER_SIZE>&) = {
			&at_cfun,
			&at_cpin,
			nullptr // KEEP IT HERE
	};

};


template<typename T, uint16_t BUFFER_SIZE>
constexpr enum at_cmd_result (*ATCmd<T, BUFFER_SIZE>::at_events[])(StringBuffer<BUFFER_SIZE>&);

#endif
