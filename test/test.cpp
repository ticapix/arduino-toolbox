#include <gtest/gtest.h>

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <assert.h>

// define some type to the closest one available on arduino
typedef uint8_t byte;
typedef std::string String;

#include "../ring_buffer.h"

#define PRINT_BUFFER(b) { for (uint16_t i = 0; i < b.length(); ++i) { \
      printf("%c", b[i]);					      \
    }								      \
    printf("\n");						      \
  }


TEST(StringBuffer, append) {
  StringBuffer<2> buff;
  assert(buff.indexOf("1") == -1);
  assert(buff.append('1') == true);
  assert(buff.indexOf("1") == 0);
}

TEST(StringBuffer, pop) {
  StringBuffer<2> buff;
  assert(buff.length() == 0);
  assert(buff.append('1') == true);
  assert(buff.length() == 1);
  assert(buff.append('2') == true);
  assert(buff.length() == 2);
  assert(buff.append('3') == false);
  assert(buff.pop_first() == '1');
  assert(buff.length() == 1);
  assert(buff.pop_first() == '2');
  assert(buff.length() == 0);
}

TEST(StringBuffer, accessor) {
  StringBuffer<3> buff;
  assert(buff.append('1') == true);
  assert(buff.append('2') == true);
  assert(buff.append('3') == true);
  PRINT_BUFFER(buff);
  assert(buff.length() == 3);
  assert(buff[0] == '1');
  assert(buff[1] == '2');
  assert(buff[2] == '3');
  assert(buff.pop_first() == '1');
  assert(buff.append('4') == true);
  assert(buff[2] == '4');    
  assert(buff.length() == 3);
  assert(buff.append('5') == false);
  assert(buff.pop_first() == '2');
  assert(buff.pop_first() == '3');
  assert(buff.length() == 1);
  PRINT_BUFFER(buff);
}

TEST(StringBuffer, indexOf) {
  StringBuffer<10> buff;
  for (auto c: {'1', '2', '3', '4', 'O', 'K', 'A', 'Y', '5', '6'})
    assert(buff.append(c) == true);
  PRINT_BUFFER(buff);
  assert(buff.indexOf("1") == 0);
  assert(buff.indexOf("2") == 1);
  assert(buff.indexOf("12") == 0);
  assert(buff.indexOf("23") == 1);
  assert(buff.indexOf("34") == 2);
  assert(buff.indexOf("OKAY") == 4);
  assert(buff.pop_first() == '1');
  assert(buff.pop_first() == '2');
  assert(buff.indexOf("OKAY") == 2);
  {
    uint16_t pos = buff.indexOf("OKAY") + 4;
    while (pos--)
      buff.pop_first();
    PRINT_BUFFER(buff);
    assert(buff.length() == 2);
  }
  assert(buff.indexOf("56") == 0);
}

TEST(StringBuffer, indexOf_offset) {
  StringBuffer<10> buff;
  for (auto c: {'1', '2', '3', '1', '2', '3', '1', '2', '3', '1'})
    assert(buff.append(c) == true);
  PRINT_BUFFER(buff);
  int pos = 0;
  uint16_t count = 0;
  while (true) {
    pos = buff.indexOf("1", pos);
    if (pos == -1)
      break;
    assert(buff[pos] == '1');
    ++pos;
    ++count;
  }
  assert(count == 4);
}

TEST(StringBuffer, full_empty) {
  StringBuffer<1> buff;
  assert(buff.empty() == true);
  buff.append('1');
  assert(buff.full() == true);
  buff.clear();
  assert(buff.empty() == true);
}

TEST(StringBuffer, append_string) {
  StringBuffer<3> buff;
  buff.append("123");
  assert(buff.full() == true);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
