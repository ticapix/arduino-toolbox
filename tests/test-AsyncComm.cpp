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

template <uint16_t BUFFER_SIZE>
struct MockCallBacks: public AsyncCommCallBacks<BUFFER_SIZE> {
	typedef StringBuffer<BUFFER_SIZE> Buffer;

	MOCK_METHOD1_T(clbk_executing, bool(Buffer&));
	MOCK_METHOD1_T(clbk_timeout, void(Buffer&));
	MOCK_METHOD1_T(clbk_buffer_overflow, void(Buffer&));
	MOCK_METHOD1_T(clbk_event, void(Buffer&));

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
  MockCallBacks<64> callbacks;
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
	AsyncComm<MockSerial, decltype(callbacks)> comm(serial, callbacks);
	EXPECT_CALL(serial, available());
	comm.tick();
}

TEST_F(AsyncCommTest, one_tick_timeout) {
  AsyncComm<MockSerial, decltype(callbacks)> comm(serial, callbacks);

  EXPECT_CALL(serial, write(_, _));
  ASSERT_TRUE(comm.exec("", 0));
  const unsigned long timeout = ASYNC_COMM_TIMEOUT_MS + 10;
  unsigned long start = millis();
  bool has_timeout = false;

  EXPECT_CALL(callbacks, clbk_executing(_)).WillRepeatedly(Return(true));
  EXPECT_CALL(callbacks, clbk_timeout(_)).WillOnce(Invoke([&has_timeout](decltype(callbacks)::Buffer&) {
	has_timeout = true;
      }));
  EXPECT_CALL(serial, available()).WillRepeatedly(Return(0));
  
  while(millis() - start < timeout) {
    comm.tick();
  }
  ASSERT_TRUE(has_timeout);
}

TEST_F(AsyncCommTest, one_tick_buffer_overflow) {
  AsyncComm<MockSerial, decltype(callbacks)> comm(serial, callbacks);

  std::string str;
  bool has_overflow = false;
  str.resize(decltype(callbacks)::Buffer::capacity());
  fakeSerialDataIn(str);
  EXPECT_CALL(callbacks, clbk_event(_));
  EXPECT_CALL(callbacks, clbk_buffer_overflow(_)).WillOnce(Invoke([&has_overflow](decltype(callbacks)::Buffer&) {
	has_overflow = true;
      }));
  comm.tick();
  ASSERT_TRUE(has_overflow);
}

TEST_F(AsyncCommTest, one_tick_event) {
	AsyncComm<MockSerial, decltype(callbacks)> comm(serial, callbacks);

	size_t len = fakeSerialDataIn("1234");
	EXPECT_CALL(callbacks, clbk_event(_)).WillOnce(
			WithArgs<0>(
					Invoke(
							[&](decltype(callbacks)::Buffer &buff) {
								ASSERT_EQ(len, buff.length());
								ASSERT_TRUE(ArraysMatch("1234", reinterpret_cast<const char*>(buff.buffer()), len));
							})));
	comm.tick();
}

TEST_F(AsyncCommTest, exec_tick) {
	AsyncComm<MockSerial, decltype(callbacks)> comm(serial, callbacks);
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
			DoAll(WithArgs<0>(Invoke([&](decltype(callbacks)::Buffer &buff) {
				ASSERT_EQ(len, buff.length());
				ASSERT_EQ(buff.index_of("OK\r\n"), 0);
				ASSERT_EQ(buff.pop_firsts(len), len);
			})), Return(false)));
	comm.tick();
	// should work because last call on executing callback should detect answer arrival
	EXPECT_CALL(serial, write(_, 0));
	ASSERT_TRUE(comm.exec("", 0));
}
