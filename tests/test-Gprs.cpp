#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string.h>

#define PRINT_BUFFER(b) fwrite(b.buffer(), 1, b.length(), stdout);

#include <RingBuffer.h>
#include <ATCmd.h>
#include <Gprs.h>

using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::Sequence;
using ::testing::_;
using ::testing::WithArgs;
using ::testing::Invoke;

class GPRSSerial {
public:

	MOCK_METHOD2(write, size_t(const void*, size_t));

	GPRSSerial() {
		ON_CALL(*this, write(_, _)).WillByDefault(ReturnArg<1>());
	}

	int read() {
		return _buffer.pop_first();
	}

	int available() {
		return _buffer.length();
	}

	int add_provision(const char* data) {
		return _buffer.append(data);
	}

private:

	StringBuffer<256> _buffer;

};

class GPRSClient: public testing::Test {

public:

	GPRSSerial serial;
};

TEST_F(GPRSClient, set_serial_ok) {
	GPRS<GPRSSerial, 256> gprs(serial);

	serial.add_provision(
			"\r\n"
			"OK\r\n"
			"\r\n"
			"+CPIN: SIM PIN\r\n"
			"\r\n"
			"OK\r\n"
			"\r\n"
			"OK\r\n"
			"\r\n"
			"+CPIN: READY\r\n"
			"\r\n"
			"OK\r\n"
			);

	ASSERT_EQ(AT_OK, gprs.set_serial_ok());
}
