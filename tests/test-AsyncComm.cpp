#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string.h>

#include <RingBuffer.h>
#include <AsyncComm.h>

using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::Sequence;
using ::testing::_;
using ::testing::WithArgs;
using ::testing::Invoke;

class MockSerial {
public:

	MOCK_METHOD0(read, int());
	MOCK_METHOD0(available, int());
	MOCK_METHOD2(write, size_t(const void*, size_t));

	MockSerial() {
		ON_CALL(*this, available()).WillByDefault(Return(0));
		ON_CALL(*this, write(_, _)).WillByDefault(ReturnArg<1>());
	}
};

struct MockCallBacks: public AsyncCommCallBacks<> {
	MOCK_METHOD1(clbk_executing, bool(AsyncCommCallBacks<>::Buffer&));
	MOCK_METHOD1(clbk_timeout, void(AsyncCommCallBacks<>::Buffer&));
	MOCK_METHOD1(clbk_buffer_overflow, void(AsyncCommCallBacks<>::Buffer&));
	MOCK_METHOD1(clbk_event, void(AsyncCommCallBacks<>::Buffer&));

	MockCallBacks() {
		ON_CALL(*this, clbk_executing(_)).WillByDefault(Return(true));
	}
};

class AsyncCommTest: public testing::Test {
public:
	size_t fakeSerialDataIn(std::string str) {
		Sequence s;
		for (size_t i = 0; i < str.size(); ++i) {
			EXPECT_CALL(serial, available()).InSequence(s).WillOnce(
					Return(str.length() - i));
			EXPECT_CALL(serial, read()).InSequence(s).WillOnce(Return(str[i]));
		}
		EXPECT_CALL(serial, available()).InSequence(s).WillRepeatedly(
				Return(0));
		return str.length();
	}

	MockSerial serial;
	MockCallBacks callbacks;
};

template<typename T>
::testing::AssertionResult ArraysMatch(const T* expected, const T* actual,
		size_t len) {
	for (size_t i = 0; i < len; ++i) {
		if (expected[i] != actual[i]) {
			return ::testing::AssertionFailure() << "array[" << i << "] ("
					<< actual[i] << ") != expected[" << i << "] ("
					<< expected[i] << ")";
		}
	}
	return ::testing::AssertionSuccess();
}

TEST_F(AsyncCommTest, one_tick_no_data) {
	AsyncComm<MockSerial, MockCallBacks> comm(serial, callbacks);
	EXPECT_CALL(serial, available());
	comm.tick();
}

TEST_F(AsyncCommTest, one_tick_event) {
	AsyncComm<MockSerial, MockCallBacks> comm(serial, callbacks);

	size_t len = fakeSerialDataIn("1234");
	EXPECT_CALL(callbacks, clbk_event(_)).WillOnce(
			WithArgs<0>(
					Invoke(
							[&](MockCallBacks::Buffer &buff) {
								ASSERT_EQ(len, buff.length());
								ASSERT_TRUE(ArraysMatch("1234", reinterpret_cast<const char*>(buff.buffer()), len));
							})));
	comm.tick();
}

TEST_F(AsyncCommTest, exec_tick) {
	AsyncComm<MockSerial, MockCallBacks> comm(serial, callbacks);
	std::string cmd("AT\r\n");
	EXPECT_CALL(serial, write(_, cmd.length()));
	// send command
	ASSERT_TRUE(comm.exec(cmd.c_str(), cmd.length()));
	// expect to fail because first command has not completed nor timeout
	ASSERT_FALSE(comm.exec("", 0));

	size_t len = fakeSerialDataIn("OK");
	EXPECT_CALL(callbacks, clbk_executing(_));
	comm.tick();
	len += fakeSerialDataIn("\r\n");
	EXPECT_CALL(callbacks, clbk_executing(_)).WillOnce(
			DoAll(WithArgs<0>(Invoke([&](MockCallBacks::Buffer &buff) {
				ASSERT_EQ(len, buff.length());
				ASSERT_EQ(buff.indexOf("OK\r\n"), 0);
				ASSERT_EQ(buff.pop_firsts(len), len);
			})), Return(false)));
	comm.tick();
	// should work because last call on executing callback should detect answer arrival
	EXPECT_CALL(serial, write(_, 0));
	ASSERT_TRUE(comm.exec("", 0));
}
