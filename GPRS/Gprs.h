#ifndef __GPRS_H__
#define __GPRS_H__

#include <Arduino.h>

#define ASSERT(a, b) b

template<typename T, uint16_t BUFFER_SIZE>
class GPRS {
public:

	GPRS(T& serial) :
			_atcmd(serial) {
	}

	enum at_cmd_result set_serial_ok() {
		uint8_t attempt = 5;
		auto result = AT_OK;
		while (attempt--) {
			// exec at cmd
			result = _atcmd.exec("AT\r\n");
			// if cmd failed, exit
			if (result != EXEC_PENDING) {
				continue;
			}
			// wait for answer
			while (result == EXEC_PENDING) {
				result = _atcmd.check_status();
			}
			// if got another event or error, exit
			if (result != AT_OK) {
				continue;
			}
			return AT_OK;
		}
		return result;
	}

	enum at_cmd_result set_pin_code(int pin) {
		uint8_t attempt = 5;
		while (attempt--) {
			// exec at cmd
			auto result = _atcmd.exec("AT+CPIN?\r\n");
			// if cmd failed, exit
			if (result != EXEC_PENDING) {
				return result;
			}
			// wait for answer
			while (_atcmd.check_status() == EXEC_PENDING) {
				delay(1);
			}
			result = _atcmd.check_status();
			// if got another event or error, exit
			if (result != AT_CPIN) {
				return result;
			}
			// parse CPIN message
			result = _atcmd.parse_event(AT_CPIN);
			// if no pin required, exit
			if (result == AT_CPIN_READY) {
				return AT_OK;
			}
			// if not pin request event, ie error, exit
			if (result != AT_CPIN_SIM_PIN) {
				return result;
			}
			// exec cmd to set pin
			result = _atcmd.exec("AT+CPIN=%d\r\n", pin);
			// if cmd failed, exit
			if (result != EXEC_PENDING) {
				return result;
			}
			// wait for answer
			while (_atcmd.check_status() == EXEC_PENDING) {
				delay(1);
			}
			// check if cmd succeded
			result = _atcmd.check_status();
			if (result != AT_OK) {
				return result;
			}
			// and loop
		}
		return EXEC_TIMEOUT;
	}

private:
	ATCmd<T, BUFFER_SIZE> _atcmd;
};

#endif
