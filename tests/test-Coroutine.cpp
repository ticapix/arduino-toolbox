#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include <Arduino.h>
#include <Coroutine.h>

using ::testing::_;
using ::testing::Field;
using ::testing::Pointee;
using ::testing::Property;
using ::testing::InSequence;
using ::testing::SafeMatcherCast;
using ::testing::Invoke;
using ::testing::WithArg;
using ::testing::Unused;


class SchedulerClkMock {
public:
	MOCK_METHOD2(callback, void(uint8_t idx, const ICoroutine*));
};

SchedulerClkMock* g_sched;
void clk_forward(uint8_t idx, const ICoroutine* coro) {
	g_sched->callback(idx, coro);
}

/////////////////////////////////////////////////////////////////////

TEST(Coroutine, simple_delay) {
	g_sched = new SchedulerClkMock();
	ICoroutine* coroutines[] = {new Delay(100)};

	EXPECT_CALL(*g_sched, callback(0, _));
	schedule_coro(coroutines, sizeof (coroutines) / sizeof(coroutines[0]), clk_forward);
	delete g_sched;
}

TEST(Coroutine, two_delay) {
	g_sched = new SchedulerClkMock();
	ICoroutine* coroutines[] = {new Delay(100), new Delay(150), new Delay(50)};

	InSequence s;
	EXPECT_CALL(*g_sched, callback(2, _));
	EXPECT_CALL(*g_sched, callback(0, _));
	EXPECT_CALL(*g_sched, callback(1, _));
	schedule_coro(coroutines, sizeof (coroutines) / sizeof(coroutines[0]), clk_forward);
	delete g_sched;
}

/////////////////////////////////////////////////////////////////////

COROUTINE(int, CountNumber,
	CORO_ARG(CountNumber, int, limit)
	CORO_VAR(int, count)
)
CORO_START(CountNumber);
{
	count = 0;
	while (count < limit) {
		++count;
		YIELD();
	}
}
CORO_RETURN(count);
CORO_END();

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

#define CORO_RESULT(ctx) (ctx._has_timeout ? reinterpret_cast<decltype(ctx._result)>(0) : ctx._result)

#define CORO_HAS_TIMEOUT(ctx) ctx._has_timeout

///////////////
CORO_CTX(bool, Delay2, )
CORO_BEGIN_CTX(Delay2)
{
	while (true) {
		YIELD_CTX();
	}
}
CORO_END_CTX()
///////////////
CORO_CTX(int, Count,
	int limit;
	int count;
)
CORO_BEGIN_CTX(Count)
{
	Delay2Ctx d;
	ctx.count = 0;
	while (ctx.count < ctx.limit) {
		++ctx.count;
		CORO_INIT(d, ctx.count * 10);
		AWAIT_CTX(Delay2, d);
	}
}
CORO_RETURN_CTX(ctx.count);
CORO_END_CTX()
///////////////

TEST(Coroutine, count_ctx) {
//	unsigned long timeout = 1000 * 60;
	CountCtx a, b;
	CORO_INIT(a, 10*10*10);
	a.limit = 10;
	CORO_INIT(b, 10*10*10);
	b.limit = 5;
	while (a._live || b._live) {
		Count(a);
		Count(b);
	}
	ASSERT_FALSE(CORO_HAS_TIMEOUT(a));
	ASSERT_EQ(10, CORO_RESULT(a));
	ASSERT_FALSE(CORO_HAS_TIMEOUT(b));
	ASSERT_EQ(5, CORO_RESULT(b));
}

TEST(Coroutine, two_counters) {
	g_sched = new SchedulerClkMock();
	ICoroutine* coroutines[] = {(new CountNumber())->set_limit(10), (new CountNumber())->set_limit(5)};

	InSequence s;
	// TODO: replace with WhenDynamicCastTo when available
	EXPECT_CALL(*g_sched, callback(1, SafeMatcherCast<const ICoroutine*>(coroutines[1]))).WillOnce(Invoke(
		[](Unused, const ICoroutine* coro) {ASSERT_EQ(5, ((CountNumber*)coro)->result());}
	));
	EXPECT_CALL(*g_sched, callback(0, SafeMatcherCast<const ICoroutine*>(coroutines[0]))).WillOnce(Invoke(
		[](Unused, const ICoroutine* coro) {ASSERT_EQ(10, ((CountNumber*)coro)->result());}
	));
	schedule_coro(coroutines, sizeof (coroutines) / sizeof(coroutines[0]), clk_forward);
	delete g_sched;
}
/////////////////////////////////////////////////////////////////////

COROUTINE(int, CountChar,
	CORO_ARG(CountChar, char, from)
	CORO_ARG(CountChar, char, to)
)
CORO_START(CountChar);
{
	AWAIT(new Delay(50 * (to - from)));
	ASSERT_TRUE(HAS_TIMEOUT());
	AWAIT((new CountNumber())->set_limit(to - from));
	ASSERT_EQ(to - from, RESULT(CountNumber));
	AWAIT(new Delay(0)); // just to check that it gets properly deleted
}
CORO_RETURN(0);
CORO_END();

TEST(Coroutine, two_countchar) {
	g_sched = new SchedulerClkMock();
	ICoroutine* coroutines[] = {
			(new CountChar())->set_from('A')->set_to('B'),
			(new CountChar())->set_from('B')->set_to('F')};

	InSequence s;
	// TODO: replace with WhenDynamicCastTo when available
	EXPECT_CALL(*g_sched, callback(0, SafeMatcherCast<const ICoroutine*>(coroutines[0])));
	EXPECT_CALL(*g_sched, callback(1, SafeMatcherCast<const ICoroutine*>(coroutines[1])));
	schedule_coro(coroutines, sizeof (coroutines) / sizeof(coroutines[0]), clk_forward);
	delete g_sched;
}
