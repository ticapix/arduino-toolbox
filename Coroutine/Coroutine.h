#ifndef __COROUTINE__
#define __COROUTINE__

unsigned long millis();

class ICoroutine {
public:
	ICoroutine() {
		reset();
	};

	bool live() { return _live; };

	virtual void run() { _live = false; };

	bool has_timeout() { return _timeout; };

protected:
	void reset() { _state = 0; _timeout = false; _live = true; _start = millis(); }
	bool _live;
	int _state;
	bool _timeout;
	unsigned long _start;
	ICoroutine* _subtask;
};


#define COROUTINE(return_type, class_name) \
class class_name : public ICoroutine { \
public: \
	return_type result() { return _result; } \
private:\
	return_type _result;

#define CORO_ARG(class_name, arg_type, arg_name) \
public: \
	class_name* set_##arg_name(arg_type _##arg_name) { reset();  this->##arg_name = _##arg_name; return this; } \
	arg_type arg_name;
//	private: \  FIXME

#define CORO_VAR(var_type, var_name) \
	var_type var_name;
//  private: \ FIXME

#define TIMEOUT_MS 500

#define CORO_START() \
	void run() { \
if (millis() - _start > TIMEOUT_MS) { _timeout = true; _live = false; } \
if (!_live) return; \
	switch (_state) { \
	case 0:;

#define CORO_END() \
	_live = false; \
	return; \
} \
	} \
};


#define CORO_RETURN(result) _result = result;

#define YIELD() { _state = __LINE__; return; case __LINE__:; }

#define AWAIT(coro) \
	_subtask = coro; \
while (_subtask->live()) { \
	_subtask->run(); \
	YIELD(); \
}

#define RESULT(class_name) \
	[this](){auto tmp = ((class_name*)this->_subtask)->result(); delete this->_subtask; return tmp; }()

#endif
