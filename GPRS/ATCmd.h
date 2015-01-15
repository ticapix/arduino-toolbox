#ifndef __ATCMD_H__
#define __ATCMD_H__

#include <Arduino.h>

#ifndef PRINT_BUFFER
#define PRINT_BUFFER()
#endif

#define AT_TIMEOUT_MS 500

#define AT_IS_ERROR(v) (0 <= v && v < 10)
#define AT_IS_EVENT(v) (20 <= v)

enum at_cmd_result
	: int8_t {
	ERROR_EXEC_ALREADY_RUNNING = 0, // from here: errors
	ERROR_EXEC_INTERNAL_BUFFER_TOO_SMALL, // error while generating cmd
	ERROR_EXEC_WRITING, // serial write failed
	ERROR_EVENT_TYPE_UNKNOWN,
	ERROR_EVENT_FORMAT_UNKNOWN,
	EXEC_PENDING = 10, // waiting for more data
	EXEC_TIMEOUT, // timeout
	AT_OK,
	AT_ERROR,
	AT_CFUN = 20, // from here: events
	AT_CFUN_FULL,
	AT_CGREG,
	AT_CLCC,
	AT_CMGF,
	AT_CMGL,
	AT_CPIN,
	AT_CPIN_SIM_PIN,
	AT_CPIN_READY,
	AT_DDET,
	AT_DTMF,
	AT_NO_RETURN
};

/*
 * EVENTS
 */
template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_cfun(StringBuffer<BUFFER_SIZE>& buffer, va_list&) {
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	if (buffer.starts_with("1"))
		result = AT_CFUN_FULL;
	buffer.pop_until("\r\n");
	return result;
}


template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_cpin(StringBuffer<BUFFER_SIZE>& buffer, va_list& ) {
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	if (buffer.starts_with("SIM PIN"))
		result = AT_CPIN_SIM_PIN;
	if (buffer.starts_with("READY"))
		result = AT_CPIN_READY;
	buffer.pop_until("\r\n");
	return result;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_dtmf(StringBuffer<BUFFER_SIZE>& buffer, va_list& args) {
	char* tone = va_arg(args, char*);
	*tone = buffer.pop_first();
	buffer.pop_until("\r\n");
	return AT_NO_RETURN;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_cgreg(StringBuffer<BUFFER_SIZE>& , va_list& ) {
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	return result;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_clcc(StringBuffer<BUFFER_SIZE>& , va_list& ) {
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	return result;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_cmgf(StringBuffer<BUFFER_SIZE>& , va_list& ) {
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	return result;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_cmgl(StringBuffer<BUFFER_SIZE>& , va_list& ) {
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	return result;
}

template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_ddet(StringBuffer<BUFFER_SIZE>& , va_list& ) {
	auto result = ERROR_EVENT_FORMAT_UNKNOWN;
	return result;
}

/*
 * ATCMD
 */
template<typename T, uint16_t BUFFER_SIZE>
class ATCmd {
public:
	ATCmd(T& serial) :
		_serial(serial),
		_is_executing(false) {
	}

	typedef StringBuffer<BUFFER_SIZE> Buffer;

	enum at_cmd_result exec(const char* format, ...) {
		enum at_cmd_result result = is_event_available();
		if (result != EXEC_PENDING)
			return result;
		if (_is_executing)
			return ERROR_EXEC_ALREADY_RUNNING;
		buffer.clear();
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
			_is_executing = true;
			return EXEC_PENDING;
		}
		return ERROR_EXEC_WRITING;
	}

	/*
	 * this is not eating data from buffer
	 */
	enum at_cmd_result check_status(unsigned long timeout = AT_TIMEOUT_MS) {
		if (_is_executing && millis() - _exec_start >= timeout) {
			_is_executing = false;
			return EXEC_TIMEOUT;
		}
		while (_serial.available()) {
			buffer.append(_serial.read());
		}
		enum at_cmd_result result = is_event_available();
		if (result != EXEC_PENDING)
			return result;
		result = is_execution_done();
		if (result != EXEC_PENDING)
			_is_executing = false;
		return result;
	}

	enum at_cmd_result parse_event(enum at_cmd_result type, ...) {
		for (auto event: at_events) {
			if (event.type != type || event.fct == nullptr)
				continue;
			va_list ap;
			va_start(ap, type);
			buffer.pop_until(event.token);
			while (buffer[0] == ' ')  // left trim space
				buffer.pop_first();
			auto result = event.fct(buffer, ap);
			va_end(ap);
			return result;
		}
		return ERROR_EVENT_TYPE_UNKNOWN;
	}

	Buffer buffer;

private:
	T& _serial;
	unsigned long _exec_start; // initialized when a new command is executed
	bool _is_executing; // set to on when calling exec() && !_is_executing, set to false when timeout or 'or' or 'error'

	enum at_cmd_result is_execution_done() {
		auto pos = buffer.index_of("OK\r\n");
		if (pos != StringBuffer<BUFFER_SIZE>::END) {
//			buffer.pop_firsts(pos);
//			buffer.pop_until("\r\n");
			return AT_OK;
		}
		pos = buffer.index_of("ERROR\r\n");
		if (pos != StringBuffer<BUFFER_SIZE>::END) {
//			buffer.pop_firsts(pos);
//			buffer.pop_until("\r\n");
			return AT_ERROR;
		}
		return EXEC_PENDING;
	}

	enum at_cmd_result is_event_available() {
		for (auto event: at_events) {
			uint16_t tok = buffer.index_of(event.token);
			if (tok == StringBuffer<BUFFER_SIZE>::END) {
				continue;
			}
			uint16_t newline = buffer.index_of("\r\n", tok); // find new line
			if (newline == StringBuffer<BUFFER_SIZE>::END) {
				return EXEC_PENDING;;
			}
			return event.type;
		}
		return EXEC_PENDING;
	}

	struct at_event {
		const char* token;
		enum at_cmd_result type;
		enum at_cmd_result (*fct)(StringBuffer<BUFFER_SIZE>&, va_list&);
	};

	static constexpr struct at_event at_events[] = {
			{ "+CFUN:", AT_CFUN, &at_cfun},
			{"+CGREG:", AT_CGREG, &at_cgreg},
			{"+CLCC:", AT_CLCC, &at_clcc},
			{"+CMGF:", AT_CMGF, &at_cmgf},
			{"+CMGL:", AT_CMGL, &at_cmgl},
			{"+CPIN:", AT_CPIN, &at_cpin},
			{"+DDET:", AT_DDET, &at_ddet},
			{"+DTMF:", AT_DTMF, &at_dtmf},
	};

};

template<typename T, uint16_t BUFFER_SIZE>
constexpr struct ATCmd<T, BUFFER_SIZE>::at_event ATCmd<T, BUFFER_SIZE>::at_events[];

#endif
