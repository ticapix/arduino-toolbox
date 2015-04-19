#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include <Arduino.h>
#include <CoroutineCtx.h>

using ::testing::_;
using ::testing::Field;
using ::testing::Pointee;
using ::testing::Property;
using ::testing::InSequence;
using ::testing::SafeMatcherCast;
using ::testing::Invoke;
using ::testing::WithArg;
using ::testing::Unused;


//class SchedulerClkMock {
//public:
//	MOCK_METHOD2(callback, void(uint8_t idx, const ICoroutine*));
//};
//
//SchedulerClkMock* g_sched;
//void clk_forward(uint8_t idx, const ICoroutine* coro) {
//	g_sched->callback(idx, coro);
//}

/////////////////////////////////////////////////////////////////////
CORO_CTX(bool, Delay, )
CORO_BEGIN_CTX(Delay)
{
	while (true) {
		YIELD_CTX();
	}
}
CORO_END_CTX()


TEST(CoroutineCtx, two_delay) {
	DelayCtx a, b, c;
	CORO_INIT(a, 100);
	CORO_INIT(b, 150);
	CORO_INIT(c, 50);

	while (CORO_ALIVE(a) && CORO_ALIVE(b) && CORO_ALIVE(c)) {
		Delay(a);
		Delay(b);
		Delay(c);
	}
	ASSERT_TRUE(CORO_HAS_TIMEOUT(c));
	while (CORO_ALIVE(a) && CORO_ALIVE(b)) {
		Delay(a);
		Delay(b);
	}
	ASSERT_TRUE(CORO_HAS_TIMEOUT(a));
	while (CORO_ALIVE(b)) {
		Delay(b);
	}
	ASSERT_TRUE(CORO_HAS_TIMEOUT(b));
}

/////////////////////////////////////////////////////////////////////
CORO_CTX(int, CountNumber,
	int limit;
	int count;
	DelayCtx d;
)
CORO_BEGIN_CTX(CountNumber)
{
	ctx.count = 0;
	while (ctx.count < ctx.limit) {
		++ctx.count;
		CORO_INIT(ctx.d, 10);
		AWAIT_CTX(Delay, ctx.d);
	}
}
CORO_RETURN_CTX(ctx.count);
CORO_END_CTX()

TEST(CoroutineCtx, two_counters) {
	CountNumberCtx a, b;
	CORO_INIT(a, 10*10*10);
	a.limit = 10;
	CORO_INIT(b, 10*10*10);
	b.limit = 5;
	while (CORO_ALIVE(a) || CORO_ALIVE(b)) {
		CountNumber(a);
		CountNumber(b);
	}
	ASSERT_FALSE(CORO_HAS_TIMEOUT(a));	ASSERT_EQ(10, CORO_RESULT(a));
	ASSERT_FALSE(CORO_HAS_TIMEOUT(b));	ASSERT_EQ(5, CORO_RESULT(b));
}

/////////////////////////////////////////////////////////////////////

CORO_CTX(bool, CountChar,
	char from;
	char to;
	DelayCtx d;
	CountNumberCtx c;
)
CORO_BEGIN_CTX(CountChar)
{
	CORO_RETURN_CTX(false);
	CORO_INIT(ctx.d, 50 * (ctx.to - ctx.from));
	AWAIT_CTX(Delay, ctx.d);
	ASSERT_TRUE(CORO_HAS_TIMEOUT(ctx.d));

	CORO_INIT(ctx.c, TIMEOUT_MS * 100);
	ctx.c.limit = ctx.to - ctx.from;
	AWAIT_CTX(CountNumber, ctx.c);
	ASSERT_FALSE(CORO_HAS_TIMEOUT(ctx.c));
	ASSERT_EQ(ctx.to - ctx.from, CORO_RESULT(ctx.c));
}
CORO_RETURN_CTX(true);
CORO_END_CTX()

TEST(CoroutineCtx, two_countchar) {
	CountCharCtx a, b;
	CORO_INIT(a, TIMEOUT_MS * 100);
	a.from = 'A';
	a.to = 'B';
	CORO_INIT(b, TIMEOUT_MS * 100);
	a.from = 'B';
	a.to = 'F';
	while (CORO_ALIVE(a) || CORO_ALIVE(b)) {
		CountChar(a);
		CountChar(b);
	}
	ASSERT_FALSE(CORO_HAS_TIMEOUT(a));	ASSERT_TRUE(CORO_RESULT(a));
	ASSERT_FALSE(CORO_HAS_TIMEOUT(b));	ASSERT_TRUE(CORO_RESULT(b));
}

