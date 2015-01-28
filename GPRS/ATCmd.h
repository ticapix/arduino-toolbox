#ifndef __ATCMD_H__
#define __ATCMD_H__

#include <Arduino.h>

#ifndef AT_TIMEOUT_MS
#define AT_TIMEOUT_MS 500
#endif

#define AT_IS_ERROR(v) (0 <= v && v < 10)
#define AT_IS_EVENT(v) (20 <= v)

enum at_cmd_result
	: int8_t {
	ERROR_EXEC_ALREADY_RUNNING = 0, // from here: errors
	ERROR_EXEC_INTERNAL_BUFFER_TOO_SMALL, // error while generating cmd
	ERROR_EXEC_WRITING, // serial write failed
	ERROR_EXEC_TIMEOUT, // timeout
	EXEC_PENDING = 10, // waiting for more data
	EXEC_OK,
	EXEC_ERROR,
	NO_EVENT,
	EVT_CFUN = 20,
	EVT_CPIN,
	EVT_DTMF
//	AT_CGREG,
//	AT_CLCC,
//	AT_CMGF,
//	AT_CMGL,
};

/*
 * ATCMD
 */
template<typename T, uint16_t BUFFER_SIZE>
class ATCmd {
public:
	ATCmd(T& serial) :
		_serial(serial),
		_is_executing(false),
		_exec_start (0) {
	}

	typedef StringBuffer<BUFFER_SIZE> Buffer;

	/**
	 * send a command on the serial port
	 * \param[in] format string format like printf
	 * \param[in] ... arguments if needed by format
	 * \retval EXEC_PENDING the command was successfully send and waiting for answer
	 * \retval ERROR_EXEC_ALREADY_RUNNING a command is already in execution
	 * \retval ERROR_EXEC_INTERNAL_BUFFER_TOO_SMALL the evaluation of format string is too large. Internal buffer is limited to 256
	 * \retval ERROR_EXEC_WRITING the number of byte returned by write do not match the length of its input buffer
	 */
//	enum at_cmd_result exec(const char* format, ...) {
//		if (_is_executing)
//			return ERROR_EXEC_ALREADY_RUNNING;
//		buffer.clear();
//		const size_t buff_len = 255;
//		va_list ap_check, ap;
//		va_start(ap_check, format);
//		va_copy(ap, ap_check);
//		size_t len = vsnprintf(NULL, 0, format, ap_check);
//		if (len > buff_len) {
//			va_end(ap_check);
//			va_end(ap);
//			return ERROR_EXEC_INTERNAL_BUFFER_TOO_SMALL;
//		}
//		char buff[buff_len];
//		vsnprintf(buff, buff_len, format, ap);
//		va_end(ap_check);
//		va_end(ap);
//		if (_serial.write(buff, len) == len) {
//			_exec_start = millis();
//			_is_executing = true;
//			return EXEC_PENDING;
//		}
//		return ERROR_EXEC_WRITING;
//	}
	enum at_cmd_result exec(const char* msg, size_t len) {
		if (_is_executing)
			return ERROR_EXEC_ALREADY_RUNNING;
		buffer.clear();
		if (_serial.write(msg, len) == len) {
			_exec_start = millis();
			_is_executing = true;
			return EXEC_PENDING;
		}
		return ERROR_EXEC_WRITING;
	}

	/**
	 * need to be called periodically to process command and events
	 * \note the function will return the same result if called multiple times in a row, except if exec() or parse_event() are called.
	 * \param[in] timeout default is 500 ms
	 * \retval EXEC_TIMEOUT no awnser for the sent command
	 * \retval EXEC_PENDING waiting for answer or event
	 * \retval AT_OK the command has ended successfully
	 * \retval AT_ERROR the command has ended with an error
	 * \retval EVT_* an event was triggered. If a command is in execution, it has not ended yet.
	 */
	enum at_cmd_result check_status(unsigned long timeout = AT_TIMEOUT_MS) {
		if (_is_executing && millis() - _exec_start >= timeout) {
			_is_executing = false;
			return ERROR_EXEC_TIMEOUT;
		}
		while (_serial.available()) {
			buffer.append(_serial.read());
		}
		enum at_cmd_result result = is_event_available();
		if (result != NO_EVENT)
			return result;
		result = is_execution_done();
		if (result != EXEC_PENDING)
			_is_executing = false;
		return result;
	}

	Buffer buffer;

private:
	T& _serial;
	bool _is_executing; // set to on when calling exec() && !_is_executing, set to false when timeout or 'or' or 'error'
	unsigned long _exec_start; // initialized when a new command is executed

	enum at_cmd_result is_execution_done() {
		auto pos = buffer.index_of("OK\r\n");
		if (pos != StringBuffer<BUFFER_SIZE>::END) {
			return EXEC_OK;
		}
		pos = buffer.index_of("ERROR\r\n");
		if (pos != StringBuffer<BUFFER_SIZE>::END) {
			return EXEC_ERROR;
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
		return NO_EVENT;
	}

	/*
	 * EVENTS
	 */

	struct _at_event {
		const char* token;
		enum at_cmd_result type;
	};

	static constexpr struct _at_event at_events[] = {
			{AT_CFUN::EVT, EVT_CFUN},
			{AT_CPIN::EVT, EVT_CPIN},
			{AT_DTMF::EVT, EVT_DTMF},

	//		{"+CGREG:", AT_CGREG},
	//		{"+CLCC:", AT_CLCC},
	//		{"+CMGF:", AT_CMGF},
	//		{"+CMGL:", AT_CMGL},
	//		{"+CME ERROR:", AT_DTMF},
	};
};

template<typename T, uint16_t BUFFER_SIZE>
constexpr struct ATCmd<T, BUFFER_SIZE>::_at_event ATCmd<T, BUFFER_SIZE>::at_events[];


/*
 * AT_OK
 */

namespace AT_OK {
size_t test(char* buff, size_t len) {
	return snprintf(buff, len, "\r\n\r\nAT\r\n");
}
}

/*
 * AT_ECHO
 */

namespace AT_ECHO {
enum status : int8_t {
	OFF = 0,
	ON = 1,
};

size_t write(char* buff, size_t len, enum AT_ECHO::status status) {
return snprintf(buff, len, "ATE%d\r\n", status);
}
}
#endif
