#ifndef __COROUTINE__
#define __COROUTINE__

#include <Arduino.h>

#define TIMEOUT_MS 5 * 1000

class ICoroutine {
public:
	ICoroutine(unsigned long timeout_ms) :
			_live(true), _state(0), _has_timeout(false), _start(millis()), _timeout_ms(
					timeout_ms), _subtask(nullptr) {
	}

	bool live() {
		return _live;
	}

	virtual void run()  = 0;

	bool has_timeout() {
		return _has_timeout;
	}

	virtual ~ICoroutine() {
		if (_subtask != nullptr) {
			delete _subtask;
		}
	}

protected:
	bool _live;  // is it running ?
	int _state;  // line to jump to
	bool _has_timeout;  // has timed out ?
	unsigned long _start;  // start timestamp
	unsigned long _timeout_ms;  // timeout value
	ICoroutine* _subtask;  // holder for a sub-coroutine inside a coroutine
};

#define COROUTINE(return_type, class_name, source) 									\
class class_name : public ICoroutine { 												\
public: 																			\
	class_name(unsigned long timeout_ms = TIMEOUT_MS) : ICoroutine(timeout_ms) {} 	\
	return_type result() { return _result; } 										\
	virtual void run(); 															\
private: 																			\
	return_type _result;															\
	source																			\
};

#define CORO_ARG(class_name, arg_type, arg_name) 													\
public: 																							\
	class_name* set_##arg_name(arg_type _##arg_name) { this->arg_name = _##arg_name; return this; } \
private: 																							\
arg_type arg_name;

#define CORO_VAR(var_type, var_name)	\
private: 								\
	var_type var_name;

#define CORO_START(class_name)	 													\
void class_name::run() { 															\
	if (millis() - _start > _timeout_ms) { _has_timeout = true; _live = false; }	\
	if (!_live) return; 															\
	switch (_state) { 																\
	case 0:;

#define CORO_END()	\
	_live = false;	\
	return; 		\
	}				\
}

#define CORO_RETURN(result) _result = result;

#define YIELD() { _state = __LINE__; return; case __LINE__:; }

#define AWAIT(coro) 						\
if (_subtask != nullptr) delete _subtask; 	\
_subtask = coro; 							\
while (_subtask->live()) { 					\
	_subtask->run(); 						\
	YIELD(); 								\
}

#define RESULT(class_name) 								\
[this](){												\
	if (this->_subtask == nullptr)						\
		return _result;									\
	auto tmp = ((class_name*)this->_subtask)->result();	\
	delete this->_subtask;								\
	this->_subtask = nullptr;							\
	return tmp;											\
}()

#define HAS_TIMEOUT() _subtask == nullptr ? false : _subtask->has_timeout()


//////////// EXAMPLE OF ASYNC Delay implementation ////////////
// Implementation in the corresponding .cpp file //
COROUTINE(int, Delay,) // NOTE the ', ' because there is no argument or variable

//////////// EXAMPLE OF MAIN SCHEDULING LOOP implementation ////////////
// Implementation in the corresponding .cpp file //

typedef void (*coro_callback)(uint8_t idx, const ICoroutine*);
void schedule_coro(ICoroutine* coroutines[], uint8_t size, coro_callback clbk);


#endif
