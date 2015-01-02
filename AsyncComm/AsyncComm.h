#ifndef __GRPS_H__
#define __GRPS_H__

enum EvtType {
  EXECUTING,
  TIMEOUT,
  BUFFER_OVERFLOW,
  EVENT,
};

#ifndef ASYNC_COMM_BUFFER_SIZE
#define ASYNC_COMM_BUFFER_SIZE 1024
#endif

#ifndef ASYNC_COMM_TIMEOUT_MS
#define ASYNC_COMM_TIMEOUT_MS 500
#endif

template<typename T>
class AsyncComm {
  public:
  
  struct CallBacks {
    virtual bool clbk_executing(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&) = 0;
    virtual void clbk_timeout(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&) = 0;
    virtual void clbk_buffer_overflow(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&) = 0;
    virtual void clbk_event(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&) = 0;
  };

    AsyncComm(T& serial, CallBacks& clbks) :
      _serial(serial),
      _clbks(clbks),
      _is_executing(false)
    {
    }

    void tick() {
      // read all avaiable data
      while (_serial.available() && !_buff.full()) {
        _buff.append(_serial.read());
      }
      // if command in execution
      if (_is_executing) {
        _is_executing = _clbks.clbk_executing(_buff);
      }
      // if command has not completed AND timeout
      if (_is_executing && ((millis() - _exec_start) % ULONG_MAX) > ASYNC_COMM_TIMEOUT_MS) {
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

    bool exec(const byte* str, size_t len) {
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

 private:
    // function pointers
    T& _serial;
    CallBacks& _clbks;
    StringBuffer<ASYNC_COMM_BUFFER_SIZE> _buff;
    bool _is_executing;
    unsigned long _exec_start;
};

#endif
