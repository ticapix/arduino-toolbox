#ifndef __ASYNC_COMM_H__
#define __ASYNC_COMM_H__

#include <Arduino.h>

#ifndef ASYNC_COMM_TIMEOUT_MS
#define ASYNC_COMM_TIMEOUT_MS 500
#endif

#ifndef ULONG_MAX
#define ULONG_MAX ((1 << 8) * sizeof(unsigned long))
#endif


/*
 * DECLARATION
 */
template<uint16_t BUFFER_SIZE>
struct AsyncCommCallBacks {
	typedef StringBuffer<BUFFER_SIZE> Buffer;

	virtual bool clbk_executing(Buffer&) = 0;
	virtual void clbk_timeout(Buffer&) = 0;
	virtual void clbk_buffer_overflow(Buffer&) = 0;
	virtual void clbk_event(Buffer&) = 0;
};


template<typename COMM, typename CALLBACKS>
class AsyncComm {
public:
	AsyncComm(COMM& serial, CALLBACKS& clbks);

	void tick();

	bool exec(const void* str, size_t len);

private:
	// function pointers
	COMM& _serial;
	CALLBACKS& _clbks;
	typedef typename CALLBACKS::Buffer Buffer;
	Buffer _buff;
	bool _is_executing;
	unsigned long _exec_start;
};

/*
 * IMPLEMENTATION
 */

template<typename COMM, typename CALLBACKS>
AsyncComm<COMM, CALLBACKS>::AsyncComm(COMM& serial, CALLBACKS& clbks) :
		_serial(serial), _clbks(clbks), _is_executing(false), _exec_start(0) {
}

template<typename COMM, typename CALLBACKS>
void AsyncComm<COMM, CALLBACKS>::tick() {
	// read all avaiable data
	while (_serial.available() && !_buff.full()) {
		_buff.append(_serial.read());
	}
	// if command in execution
	if (_is_executing) {
		_is_executing = _clbks.clbk_executing(_buff);
	}
	// if command has not completed AND timeout
	if (_is_executing
			&& ((millis() - _exec_start) % ULONG_MAX) > ASYNC_COMM_TIMEOUT_MS) {
		_clbks.clbk_timeout(_buff);
		_is_executing = false;
	}
	// if command has not completed AND buffer is full
	if (_is_executing && _buff.full()) {
		_clbks.clbk_buffer_overflow(_buff);
		if (_buff.full())
			_buff.pop_first();
	}
	// if not expecting command result AND buffer not empty
	if (!_is_executing && !_buff.empty()) {
		_clbks.clbk_event(_buff);
	}
}

template<typename COMM, typename CALLBACKS>
bool AsyncComm<COMM, CALLBACKS>::exec(const void* str, size_t len) {
	// if already executing a command
	if (_is_executing) {
		return false;
	}
	// consume event from buffer if any
	if (!_buff.empty()) {
		_clbks.clbk_event(_buff);
	}
	// discard all info in the buffer
	_buff.clear();
	_is_executing = true;
	_exec_start = millis();
	_serial.write(str, len);
	return true;
}

#endif
