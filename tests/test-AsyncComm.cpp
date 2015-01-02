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

struct MockCallBacks : public AsyncComm<MockSerial>::CallBacks {
  MOCK_METHOD1(clbk_executing, bool(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&));
  MOCK_METHOD1(clbk_timeout, void(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&));
  MOCK_METHOD1(clbk_buffer_overflow, void(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&));
  MOCK_METHOD1(clbk_event, void(StringBuffer<ASYNC_COMM_BUFFER_SIZE>&));

  MockCallBacks() {
    ON_CALL(*this, clbk_executing(_)).WillByDefault(Return(true));
  }
};

class AsyncCommTest : public testing::Test { 
public:
	size_t fakeSerialDataIn(std::string str) {
		Sequence s;
		for (size_t i = 0; i < str.size(); ++i) {
			EXPECT_CALL(serial, available()).InSequence(s).WillOnce(Return(str.length() - i));
		    EXPECT_CALL(serial, read()).InSequence(s).WillOnce(Return(str[i]));
		}
		EXPECT_CALL(serial, available()).InSequence(s).WillRepeatedly(Return(0));
		return str.length();
	}

  MockSerial serial;
  MockCallBacks callbacks;
};


template<typename T>
::testing::AssertionResult ArraysMatch(const T* expected, const T* actual, size_t len){
  for (size_t i = 0; i < len; ++i) {
    if (expected[i] != actual[i]) {
      return ::testing::AssertionFailure() << "array[" << i
					   << "] (" << actual[i] << ") != expected[" << i
					   << "] (" << expected[i] << ")";
    }
  }  
  return ::testing::AssertionSuccess();
}


TEST_F(AsyncCommTest, one_tick_no_data) {
  AsyncComm<MockSerial> comm(serial, callbacks);
  EXPECT_CALL(serial, available());
  comm.tick();
}



TEST_F(AsyncCommTest, one_tick_event) {
  Sequence s;
  AsyncComm<MockSerial> comm(serial, callbacks);

  size_t len = fakeSerialDataIn("1234");
  EXPECT_CALL(callbacks, clbk_event(_)).WillOnce(WithArgs<0>(Invoke([&](StringBuffer<ASYNC_COMM_BUFFER_SIZE>& buff) {
	  ASSERT_EQ(len, buff.length());
	  ASSERT_TRUE(ArraysMatch("1234", reinterpret_cast<const char*>(buff.buffer()), len));
	})));

  comm.tick();
}

TEST_F(AsyncCommTest, exec_tick) {
  Sequence s;
  AsyncComm<MockSerial> comm(serial, callbacks);
  std::string cmd("AT\r\n");
  EXPECT_CALL(serial, write(_, cmd.length()));
  // send command
  ASSERT_TRUE(comm.exec(cmd.c_str(), cmd.length()));
  // expect to fail because first command has not completed nor timeout
  ASSERT_FALSE(comm.exec("", 0));

  fakeSerialDataIn("OK");
  comm.tick();
  fakeSerialDataIn("\r\n");
  comm.tick();
}
