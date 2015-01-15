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
		if (_buffer.length() != 0)
			return _buffer.length();
		if (_serial_events.size() == 0)
			return 0;
		_buffer.append(_serial_events[0].c_str());
		_serial_events.erase(_serial_events.begin());
		// start consuming next time
		return 0;
	}

	void add_provision(const std::string data) {
		if (_serial_events.size() == 0 && _buffer.length() == 0) {
			// this the first add_provision or everything has been consumed
			// pushing new data in the buffer directly
			_buffer.append(data.c_str());
		}
		_serial_events.push_back(data);
	}

private:
	std::vector<std::string> _serial_events;
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
			"OK\r\n");
	serial.add_provision(
			"\r\n"
			"+CPIN: SIM PIN\r\n"
			"\r\n"
			"OK\r\n");
	serial.add_provision(
			"\r\n"
			"OK\r\n");
	serial.add_provision(
			"\r\n"
			"+CPIN: READY\r\n"
			"\r\n"
			"OK\r\n"
			);
	ASSERT_EQ(AT_OK, gprs.set_serial_ok());
}
