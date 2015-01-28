#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string.h>

#define PRINT_BUFFER(b) fwrite(b.buffer(), 1, b.length(), stdout);

#include <RingBuffer.h>
#include <AT_CFUN.h>
#include <AT_CPIN.h>
#include <AT_DDET.h>
#include <ATCmd.h>

using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::Sequence;
using ::testing::_;
using ::testing::WithArgs;
using ::testing::Invoke;

class ATMockSerial {
public:

	MOCK_METHOD2(write, size_t(const void*, size_t));

	ATMockSerial() {
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


class ATMockSerialNetwork {
public:
	ATMockSerialNetwork(const char* hostname, int portno) :
			_sockfd(-1) {
		ON_CALL(*this, read()).WillByDefault(
				Invoke(this, &ATMockSerialNetwork::_read));
		ON_CALL(*this, available()).WillByDefault(
				Invoke(this, &ATMockSerialNetwork::_available));
		ON_CALL(*this, write(_, _)).WillByDefault(
				Invoke(this, &ATMockSerialNetwork::_write));

		_init(hostname, portno);
	}

	MOCK_METHOD0(read, int());
	MOCK_METHOD0(available, int());
	MOCK_METHOD2(write, size_t(const void*, size_t));

private:
	int _sockfd;

	int _read() {
		unsigned char c;
		assert(::read(_sockfd, &c, 1) == 1);
		return c;
	}

	int _available() {
		int count;
		ioctl(_sockfd, FIONREAD, &count);
		return count;
	}

	size_t _write(const void* buf, size_t count) {
		return ::write(_sockfd, buf, count);
	}

	void _init(const char* hostname, int portno) {
		struct sockaddr_in serv_addr;
		struct hostent *server;

		_sockfd = socket(AF_INET, SOCK_STREAM, 0);

		ASSERT_GE(0, _sockfd);

		server = gethostbyname(hostname);
		if (server == NULL) {
			fprintf(stderr, "ERROR, no such host\n");
			exit(0);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *) server->h_addr,
				(char *)&serv_addr.sin_addr.s_addr,
				server->h_length);
		serv_addr.sin_port = htons(portno);
		ASSERT_GE(0, connect(_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)));
	}
};

class ATCmdClient: public testing::Test {

public:

	ATMockSerial serial;
	static const size_t buff_len = 255;
	char buffer[buff_len];
};

TEST_F(ATCmdClient, at_timeout) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_OK::test(buffer, buff_len)));
	ASSERT_EQ(ERROR_EXEC_TIMEOUT, atcmd.check_status(0));
}

TEST_F(ATCmdClient, at_already_running) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(" ", 1));
	ASSERT_EQ(ERROR_EXEC_ALREADY_RUNNING, atcmd.exec(" ", 1));
}

TEST_F(ATCmdClient, at_write_error) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	EXPECT_CALL(serial, write(_ , _)).WillOnce(Return(0));
	ASSERT_EQ(ERROR_EXEC_WRITING, atcmd.exec(" ", 1));
}

TEST_F(ATCmdClient, at_ok) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	serial.add_provision("\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_OK::test(buffer, buff_len)));
	ASSERT_EQ(EXEC_OK, atcmd.check_status());
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(" ", 1));
	ASSERT_EQ(0, atcmd.buffer.length());
}

TEST_F(ATCmdClient, at_error) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	serial.add_provision("\r\nERROR\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_OK::test(buffer, buff_len)));
	ASSERT_EQ(EXEC_ERROR, atcmd.check_status());
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(" ", 1));
	ASSERT_EQ(0, atcmd.buffer.length());
}

TEST_F(ATCmdClient, at_echo) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	serial.add_provision("\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_ECHO::write(buffer, buff_len, AT_ECHO::OFF)));
	ASSERT_EQ(EXEC_OK, atcmd.check_status());
}

TEST_F(ATCmdClient, at_cfun) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	serial.add_provision("OK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CFUN::test(buffer, buff_len)));
	ASSERT_EQ(EXEC_OK, atcmd.check_status());

	serial.add_provision("\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CFUN::write(buffer, buff_len, AT_CFUN::FULL)));
	ASSERT_EQ(EXEC_OK, atcmd.check_status());

	enum AT_CFUN::fun fun = AT_CFUN::MINIMAL;

	serial.add_provision("\r\n+CFUN: 1\r\n\r\nOK\r\n");
	EXPECT_CALL(serial, write(_, _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CFUN::read(buffer, buff_len)));
	ASSERT_EQ(EVT_CFUN, atcmd.check_status());
	ASSERT_EQ(1, AT_CFUN::parse<256>(atcmd.buffer, &fun));
	ASSERT_EQ(AT_CFUN::FULL, fun);
	ASSERT_EQ(EXEC_OK, atcmd.check_status());
}

TEST_F(ATCmdClient, at_cpin) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CPIN::test(buffer, buff_len)));
	ASSERT_EQ(EXEC_PENDING, atcmd.check_status());
	serial.add_provision("OK\r\n");
	ASSERT_EQ(EXEC_OK, atcmd.check_status());

	enum AT_CPIN::status status;

	serial.add_provision("\r\n+CPIN: SIM PUK\r\n\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CPIN::read(buffer, buff_len)));
	ASSERT_EQ(EVT_CPIN, atcmd.check_status());
	ASSERT_EQ(1, AT_CPIN::parse<256>(atcmd.buffer, &status));
	ASSERT_EQ(AT_CPIN::SIM_PUK, status);
	ASSERT_EQ(EXEC_OK, atcmd.check_status());

	serial.add_provision("\r\n+CPIN: SIM PIN\r\n\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CPIN::read(buffer, buff_len)));
	ASSERT_EQ(EVT_CPIN, atcmd.check_status());
	ASSERT_EQ(1, AT_CPIN::parse<256>(atcmd.buffer, &status));
	ASSERT_EQ(AT_CPIN::SIM_PIN, status);
	ASSERT_EQ(EXEC_OK, atcmd.check_status());

	serial.add_provision("\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CPIN::write(buffer, buff_len, "1234")));
	ASSERT_EQ(EXEC_OK, atcmd.check_status());

	serial.add_provision("\r\n+CPIN: READY\r\n\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_CPIN::read(buffer, buff_len)));
	ASSERT_EQ(EVT_CPIN, atcmd.check_status());
	ASSERT_EQ(1, AT_CPIN::parse<256>(atcmd.buffer, &status));
	ASSERT_EQ(AT_CPIN::READY, status);
	ASSERT_EQ(EXEC_OK, atcmd.check_status());
}

TEST_F(ATCmdClient, at_dtmf) {
	ATCmd<ATMockSerial, 256> atcmd(serial);

	serial.add_provision("\r\nOK\r\n");
	EXPECT_CALL(serial, write(_ , _));
	ASSERT_EQ(EXEC_PENDING, atcmd.exec(buffer, AT_DTMF::write(buffer, buff_len, AT_DTMF::ENABLE)));
	ASSERT_EQ(EXEC_OK, atcmd.check_status());

	std::string tones[] = {"1", "9", "*", "#"};
	for (auto t: tones) {
		ASSERT_EQ(8, serial.add_provision("\r\n+DTMF:"));
		ASSERT_EQ(1, serial.add_provision(t.c_str()));
		ASSERT_EQ(2, serial.add_provision("\r\n"));
	}
	for (auto t: tones) {
		char tone;
		ASSERT_EQ(EVT_DTMF, atcmd.check_status());
		ASSERT_EQ(1, AT_DTMF::parse<256>(atcmd.buffer, &tone));
		ASSERT_EQ(t[0], tone);
	}
}
