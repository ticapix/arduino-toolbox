#ifndef __GPRS_H__
#define __GPRS_H__

#include <Arduino.h>

#define ASSERT(a, b) b

#define YIELD(result) return result;

template<typename T, uint16_t BUFFER_SIZE>
class GPRS {
public:

	GPRS(T& serial) :
			_atcmd(serial) {
	}

	enum at_cmd_result set_serial_ok() {
		uint8_t attempt = 3;
		auto result = AT_OK;
		while (attempt--) {
			auto result = AT_OK<T, BUFFER_SIZE>::test(_atcmd);
			if (result != EXEC_PENDING) {
				continue;
			}
			// wait for answer
			while (result == EXEC_PENDING) {
				result = _atcmd.check_status();
				if (AT_IS_EVENT(result)) {
					YIELD(result);
				}
			}
			// if got another event or error, exit
			if (result != AT_OK)
				continue;
			return AT_OK;
		}
		return result;
	}

	enum at_cmd_result set_pin_code(const char* pin) {
		// make sure modem can communicate
		auto result;
		while (true) {
			result = set_serial_ok();
			if (AT_IS_EVENT(result)) {
				YIELD(result);
			} else {
				break;
			}
		}
		if (result != AT_OK)
			return result;
		// exec at cmd
		result = AT_CPIN<T, BUFFER_SIZE>::read(_atcmd);
		// if cmd failed, exit
		if (result != EXEC_PENDING)
			return result;
		// wait for answer
		while (result == EXEC_PENDING)
			result = _atcmd.check_status();
		// if got another event or error, exit
		if (result != EVT_CPIN)
			return result;
		// parse CPIN message
		enum AT_CPIN<T, BUFFER_SIZE>::status status;
		count = AT_CPIN<T, BUFFER_SIZE>::parse(_atcmd.buffer, &status);
		Assert(count == 1);
		// if no pin required, exit
		if (result == AT_CPIN_READY)
			return AT_OK;
		// if not pin request event, ie error, exit
		if (result != AT_CPIN_SIM_PIN)
			return result;

		result = _atcmd.check_status();
		while (result == EXEC_PENDING)
			result = _atcmd.check_status();
		if (result != AT_OK)
			return result;

		// exec cmd to set pin
		result = _atcmd.exec("AT+CPIN=%s\r\n", pin);
		// if cmd failed, exit
		if (result != EXEC_PENDING)
			return result;
		// wait for answer
		while (result == EXEC_PENDING)
			result = _atcmd.check_status();
		// check if cmd succeded
		result = _atcmd.check_status();
		return result;
	}


	enum at_cmd_result set_echo_mode(bool echo) {
		// make sure modem can communicate
		auto result = set_serial_ok();
		return result;
	}

	enum at_cmd_result set_auto_awswer(uint8_t rings) {
		// make sure modem can communicate
		auto result = set_serial_ok();
		return result;
	}

//	{ "+CFUN:", AT_CFUN, &at_cfun},
//	{"+CGREG:", AT_CGREG, &at_cgreg},
//	{"+CLCC:", AT_CLCC, &at_clcc},
//	{"+CMGF:", AT_CMGF, &at_cmgf},
//	{"+CMGL:", AT_CMGL, &at_cmgl},
//	{"+CPIN:", AT_CPIN, &at_cpin},
//	{"+DDET:", AT_DDET, &at_ddet},
//	{"+DTMF:", AT_DTMF, &at_dtmf},


private:
	ATCmd<T, BUFFER_SIZE> _atcmd;
};

#endif
