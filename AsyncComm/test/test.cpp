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
  MOCK_METHOD2(write, size_t(const byte*, size_t));

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

  std::string msg("1234");
  for (size_t i = 0; i < msg.size(); ++i) {
    EXPECT_CALL(serial, available()).InSequence(s).WillOnce(Return(msg.length() - i));  
    EXPECT_CALL(serial, read()).InSequence(s).WillOnce(Return(msg[i]));
  }
  EXPECT_CALL(serial, available()).InSequence(s).WillRepeatedly(Return(0));
  EXPECT_CALL(callbacks, clbk_event(_)).WillOnce(WithArgs<0>(Invoke([&](StringBuffer<ASYNC_COMM_BUFFER_SIZE>& buff) {
	  ASSERT_EQ(msg.length(), buff.length());
	  ASSERT_TRUE(ArraysMatch(msg.c_str(), reinterpret_cast<const char*>(buff.buffer()), msg.length()));
	})));
  
  comm.tick();
}

TEST_F(AsyncCommTest, exec_tick) {
  Sequence s;
  AsyncComm<MockSerial> comm(serial, callbacks);
  {
    std::string msg("AT\r\n");
    EXPECT_CALL(serial, write(_, msg.length()));
    // send command
    ASSERT_TRUE(comm.exec(reinterpret_cast<const byte*>(msg.c_str()), msg.length()));
    // expect to fail because first command has not completed nor timeout
    ASSERT_FALSE(comm.exec(reinterpret_cast<const byte*>(""), 0));
  }
  {
    std::string msg("OK");
    for (size_t i = 0; i < msg.size(); ++i) {
      EXPECT_CALL(serial, available()).InSequence(s).WillOnce(Return(msg.length() - i));  
      EXPECT_CALL(serial, read()).InSequence(s).WillOnce(Return(msg[i]));
    }
    EXPECT_CALL(serial, available()).InSequence(s).WillRepeatedly(Return(0));
  }
  comm.tick();
  {
    std::string msg("\r\n");
    for (size_t i = 0; i < msg.size(); ++i) {
      EXPECT_CALL(serial, available()).InSequence(s).WillOnce(Return(msg.length() - i));  
      EXPECT_CALL(serial, read()).InSequence(s).WillOnce(Return(msg[i]));
    }
    EXPECT_CALL(serial, available()).InSequence(s).WillRepeatedly(Return(0));
  }
  comm.tick();
}


int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
