#ifndef __GPRS_H__
#define __GPRS_H__

#include <Arduino.h>

#ifndef PRINT_BUFFER
#define PRINT_BUFFER()
#endif

#define AT_TIMEOUT_MS 500

enum at_cmd_name
	: uint8_t {
		NONE, AT, AT_CPIN, ATE, AT_CFUN, AT_CLCC, AT_DDET, AT_CGREG, ATS0
};

enum at_cmd_type
	: uint8_t {
		CHECK = 0, GET = 1, SET = 2, EVENT = 3
};

enum at_cmd_result
	: int8_t {
		EXEC_OK, // AT returned OK
	EXEC_PENDING, // waiting for more data
	EXEC_TIMEOUT, // timeout
	EXEC_ERROR, // AT returned ERROR
	ERROR_CMD_UNKNOWN, // try to exec cmd with unknown name
	ERROR_EXEC_INTERNAL_BUFFER_TOO_SMALL, // error while generating cmd
	ERROR_EXEC_WRITING, // serial write failed
	ERROR_CMD_NULL_FORMAT, // trying to exec cmd with no
	ERROR_CMD_NULL_FUNCTION,
	ERROR_EVENT_TYPE_UNKNOWN,
	ERROR_EVENT_FORMAT_UNKNOWN,
	AT_CPIN_READY,
	AT_CPIN_SIM_PIN,
	AT_CFUN_FULL
};


template<uint16_t BUFFER_SIZE>
enum at_cmd_result at_check(StringBuffer<BUFFER_SIZE>& buffer) {
	if (buffer.index_of("OK\r\n") != StringBuffer<BUFFER_SIZE>::END)
		return EXEC_OK;
	if (buffer.index_of("ERROR\r\n") != StringBuffer<BUFFER_SIZE>::END)
		return EXEC_ERROR;
	return EXEC_PENDING;
}

template<typename T, uint16_t BUFFER_SIZE>
class Gprs {
public:
	Gprs(T& serial) :
			_serial(serial) {
	}

	typedef StringBuffer<BUFFER_SIZE> Buffer;

	enum at_cmd_result exec(const char* format, ...) {
		// TODO: check for event
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
			return EXEC_PENDING;
		}
		return ERROR_EXEC_WRITING;
	}

//	enum at_cmd_result parse(enum at_cmd_name name, enum at_cmd_type type) {
//		for (uint8_t idx = 0; _at_commands[idx].name != NONE; ++idx) {
//			if (_at_commands[idx].name == name) {
//				auto fct = _at_commands[idx].fct[type];
//				if (fct == nullptr)
//					return ERROR_CMD_NULL_FUNCTION;
//				return fct(buffer);
//			}
//		}
//		return ERROR_CMD_UNKNOWN;
//	}

	enum at_cmd_result wait_completion(unsigned long timeout = AT_TIMEOUT_MS) {
		enum at_cmd_result result = EXEC_PENDING;
		while (millis() - _exec_start < timeout && result == EXEC_PENDING) {
			if (_serial.available()) {
				buffer.append(_serial.read());
			}
			// TODO: check for event
			result = at_check(buffer);
		}
		if (result != EXEC_PENDING)
			return result;
		return EXEC_TIMEOUT;
	}

	Buffer buffer;

private:
	T& _serial;
	unsigned long _exec_start; // initialized when a new command is executed
	uint8_t _at_cmd_idx; // index of the last executed command (prevent scanning the array of cmd every time)
	enum at_cmd_type _at_cmd_type; // type of the last executed command

	struct at_command {
		enum at_cmd_name name;
		const char* fmt[3];
		enum at_cmd_result (*fct[4])(StringBuffer<BUFFER_SIZE>&);
	};

	static constexpr struct at_command _at_commands[] = {
		// for sanity check
		{	AT, {nullptr, "AT\r\n", nullptr}, {nullptr, &at_check, nullptr, nullptr}},
//		// set command echo (debug only)
//		{	ATE, {nullptr, nullptr, "ATE%d\r\n"}, {nullptr, nullptr, &at_check, nullptr}},
//		// set phone functionality
//		{	AT_CFUN, {nullptr, "AT+CFUN?\r\n", "AT+CFUN=%d\r\n"}, {nullptr, &at_cfun_get, &at_check, nullptr}},
//		// enter pin
//		{	AT_CPIN, {nullptr, "AT+CPIN?\r\n", "AT+CPIN=%d\r\n"}, {nullptr, &at_cpin_get, &at_check, nullptr}},
//		// enable DTMF tone detection
//		{	AT_DDET, {nullptr, nullptr, "AT+DDET=%d\r\n"}, {nullptr, nullptr, &at_check, &at_ddet_event}},
//		// set auto answer
//		{	ATS0, {nullptr, nullptr, "ATS0=%d\r\n"}, {nullptr, nullptr, &at_check, nullptr}},
//		// check ME registration on the cell network
//		{	AT_CGREG, {nullptr, "AT+CGREG?\r\n", "AT+CGREG=%d\r\n"}, {nullptr, &at_check, &at_check, &at_cgreg_event}},
//		// enable call status notification
//		{	AT_CLCC, {nullptr, nullptr, "AT+CLCC=%d\r\n"}, {nullptr, nullptr, &at_check, &at_clcc_event}},
		// end
		{	NONE, {nullptr, nullptr, nullptr}, {nullptr, nullptr, nullptr, nullptr}},
	};
};

template<typename T, uint16_t BUFFER_SIZE>
constexpr struct Gprs<T, BUFFER_SIZE>::at_command Gprs<T, BUFFER_SIZE>::_at_commands[];

#endif
