#ifndef __COROUTINE__
#define __COROUTINE__

#include <Arduino.h>

#define TIMEOUT_MS 5 * 1000


struct CoroCtx {
	bool _live;
	int16_t _state;
	bool _has_timeout;
	unsigned long _start;
	unsigned long _timeout_ms;
};

#define CORO_CTX(return_type, fct_name, source)					\
		struct fct_name ## _ctx : CoroCtx {						\
			return_type _result;								\
			source												\
		};														\
		typedef struct fct_name ## _ctx fct_name ## Ctx;

#define CORO_INIT(ctx, timeout_ms)	\
	memset(&ctx, 0, sizeof(ctx));	\
	ctx._live = true;				\
	ctx._state = 0;					\
	ctx._has_timeout = false;		\
	ctx._start = millis();			\
	ctx._timeout_ms = timeout_ms;

#define CORO_BEGIN_CTX(fct_name)	 												\
void fct_name(fct_name ## _ctx &ctx) { 															\
	if (millis() - ctx._start > ctx._timeout_ms) { ctx._has_timeout = true; ctx._live = false; }	\
	if (!ctx._live) return; 																	\
	switch (ctx._state) { 																		\
	case 0:;

#define CORO_END_CTX()		\
		ctx._live = false;	\
	return; 				\
	}						\
}

#define CORO_RETURN_CTX(result) ctx._result = result;

#define YIELD_CTX() { ctx._state = __LINE__; return; case __LINE__:; }

#define AWAIT_CTX(fct_name, ctx)		\
		while(ctx._live) {				\
			fct_name(ctx);				\
			YIELD_CTX();				\
		}

//#define CORO_RESULT(ctx) (ctx._has_timeout ? reinterpret_cast<decltype(ctx._result)>(0) : ctx._result)
#define CORO_RESULT(ctx) ctx._result

#define CORO_HAS_TIMEOUT(ctx) ctx._has_timeout

#define CORO_ALIVE(ctx) ctx._live

#endif
