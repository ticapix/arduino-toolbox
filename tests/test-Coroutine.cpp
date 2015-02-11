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
