#include <gtest/gtest.h>

#include <Arduino.h>
#include <RingBuffer.h>

TEST(StringBuffer, error_handling) {
	StringBuffer<2> buff;
	// just init mem to a known value
	ASSERT_EQ(buff.append('B'), 1);
	// and clear buffer
	buff.clear();
	// should return undefined value if previously not init to 'B'
	ASSERT_EQ(buff[0], 'B');
	ASSERT_EQ(buff.pop_first(), 'B');
	ASSERT_EQ(buff.pop_last(), 'B');
	// try to append more than possible
	ASSERT_EQ(buff.append("123"), decltype(buff)::capacity());
}

TEST(StringBuffer, error_index_of) {
	StringBuffer<2> buff;
	ASSERT_EQ(buff.append('B'), 1);
	// try to find unexisting string
	ASSERT_EQ(buff.index_of("B", 1), decltype(buff)::END);
	buff.clear();
	ASSERT_EQ(buff.index_of("", 0), decltype(buff)::END);
}

TEST(StringBuffer, capacity) {
	StringBuffer<2> buff;
	ASSERT_EQ(decltype(buff)::capacity(), 2);
}

TEST(StringBuffer, append) {
	StringBuffer<2> buff;
	ASSERT_EQ(buff.index_of("1"), decltype(buff)::END);
	ASSERT_EQ(buff.append('1'), 1);
	ASSERT_EQ(buff.index_of("1"), 0);
}

TEST(StringBuffer, starts_with) {
	StringBuffer<4> buff;
	ASSERT_EQ(buff.append("1234"), 4);
	ASSERT_EQ(buff.pop_first(), '1');
	ASSERT_TRUE(buff.starts_with("234"));
}


TEST(StringBuffer, pop_first) {
	StringBuffer<2> buff;
	ASSERT_EQ(buff.length(), 0);
	ASSERT_EQ(buff.append('1'), 1);
	ASSERT_EQ(buff.length(), 1);
	ASSERT_EQ(buff.append('2'), 1);
	ASSERT_EQ(buff.length(), 2);
	ASSERT_EQ(buff.append('3'), 0);
	ASSERT_EQ(buff.pop_first(), '1');
	ASSERT_EQ(buff.length(), 1);
	ASSERT_EQ(buff.pop_first(), '2');
	ASSERT_EQ(buff.length(), 0);
}

TEST(StringBuffer, pop_firsts) {
	StringBuffer<3> buff;
	ASSERT_EQ(buff.append("123"), 3);
	ASSERT_EQ(buff.pop_firsts(3), 3);
	ASSERT_TRUE(buff.empty());
}

TEST(StringBuffer, pop_last) {
	StringBuffer<3> buff;
	ASSERT_EQ(buff.append("123"), 3);
	ASSERT_EQ(buff.pop_last(), '3');
	ASSERT_EQ(buff.length(), 2);
	ASSERT_EQ(buff.append('4'), 1);
	ASSERT_EQ(buff[2], '4');
	ASSERT_EQ(strncmp((const char* )buff.buffer(), "124", buff.length()), 0);
}

TEST(StringBuffer, accessor) {
	StringBuffer<3> buff;
	ASSERT_EQ(buff.append("123"), 3);
	ASSERT_EQ(buff.length(), 3);
	ASSERT_EQ(buff[0], '1');
	ASSERT_EQ(buff[1], '2');
	ASSERT_EQ(buff[2], '3');
	ASSERT_EQ(buff.pop_first(), '1');
	ASSERT_EQ(buff.append('4'), 1);
	ASSERT_EQ(buff[2], '4');
	ASSERT_EQ(buff.length(), 3);
	ASSERT_EQ(buff.append('5'), 0);
	ASSERT_EQ(buff.pop_first(), '2');
	ASSERT_EQ(buff.pop_first(), '3');
	ASSERT_EQ(buff.length(), 1);
}

TEST(StringBuffer, index_of_not_found) {
	StringBuffer<10> buff;
	ASSERT_EQ(buff.index_of("0"), decltype(buff)::END);
}

TEST(StringBuffer, index_of) {
	StringBuffer<10> buff;
	for (auto c : { '1', '2', '3', '4', 'O', 'K', 'A', 'Y', '5', '6' })
	        ASSERT_EQ(buff.append(c), 1);
	ASSERT_EQ(buff.index_of("1"), 0);
	ASSERT_EQ(buff.index_of("2"), 1);
	ASSERT_EQ(buff.index_of("12"), 0);
	ASSERT_EQ(buff.index_of("23"), 1);
	ASSERT_EQ(buff.index_of("34"), 2);
	ASSERT_EQ(buff.index_of("OKAY"), 4);
	ASSERT_EQ(buff.pop_first(), '1');
	ASSERT_EQ(buff.pop_first(), '2');
	ASSERT_EQ(buff.index_of("OKAY"), 2);
	{
		uint16_t pos = buff.index_of("OKAY") + 4;
		while (pos--)
			buff.pop_first();
		ASSERT_EQ(buff.length(), 2);
	}
	ASSERT_EQ(buff.index_of("56"), 0);
}

TEST(StringBuffer, index_of_offset) {
	StringBuffer<10> buff;
	for (auto c : { '1', '2', '3', '1', '2', '3', '1', '2', '3', '1' })
	        ASSERT_EQ(buff.append(c), 1);
	int pos = 0;
	uint16_t count = 0;
	while (true) {
		pos = buff.index_of("1", pos);
		if (pos == decltype(buff)::END)
			break;
		ASSERT_EQ(buff[pos], '1');
		++pos;
		++count;
	}
	ASSERT_EQ(count, 4);
}

TEST(StringBuffer, full_empty) {
	StringBuffer<1> buff;
	ASSERT_TRUE(buff.empty());
	buff.append('1');
	ASSERT_TRUE(buff.full());
	buff.clear();
	ASSERT_TRUE(buff.empty());
}

TEST(StringBuffer, append_string) {
	StringBuffer<3> buff;
	buff.append("123");
	ASSERT_TRUE(buff.full());
	ASSERT_EQ(strncmp((const char* )buff.buffer(), "123", buff.length()), 0);
}

TEST(StringBuffer, buffer) {
	StringBuffer<3> buff;
	buff.append("123");
	ASSERT_TRUE(buff.full());
	buff.pop_first();
	ASSERT_EQ(strncmp((const char* )buff.buffer(), "23", buff.length()), 0);
}

TEST(RingBuffer, memmove_with_int) {
	RingBuffer<3, int> buff;
	for (auto c : { 1, 2, 3 })
        	  ASSERT_EQ(buff.append(c), 1); // |1|2|3|
	ASSERT_EQ(buff.pop_firsts(2), 2); // | | |3|
	ASSERT_EQ(buff.append(4), 1); // |4| |3|
	ASSERT_EQ(buff.append(5), 1); // |4|5|3|
	ASSERT_EQ(buff.length(), 3);
	ASSERT_EQ(buff[0], 3);
	ASSERT_EQ(buff[1], 4);
	ASSERT_EQ(buff[2], 5);
	const int *arr = buff.buffer(); // should trigger an internal memmove |3|4|5|
	ASSERT_EQ(arr[0], 3);
	ASSERT_EQ(arr[1], 4);
	ASSERT_EQ(arr[2], 5);
}

TEST(RingBuffer, memmove_append) {
	RingBuffer<3, short> buff;
	for (auto c : { 1, 2, 3 })  // |1|2|3|
	        ASSERT_EQ(buff.append(c), 1);
	buff.pop_first(); // | |2|3|
	buff.pop_first(); // | | |3|
	ASSERT_EQ(buff.append(4), 1); // |4| |3|
	// should not be continous anymore in internal mem
	ASSERT_EQ(buff.length(), 2);
	// should trigger an internal memmove
	buff.buffer(); // | |3|4|
	// check that indexes are correct avec memmove
	ASSERT_EQ(buff.append(5), 1); // |5|3|4|
	// should trigger an internal memmove
	const short *arr = buff.buffer(); // |3|4|5|
	ASSERT_EQ(arr[0], 3);
	ASSERT_EQ(arr[1], 4);
	ASSERT_EQ(arr[2], 5);
}
