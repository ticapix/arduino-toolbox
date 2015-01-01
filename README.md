# arduino-ringbuffer

Simple RingBuffer for arduino, providing a StringBuffer container with no dynamic memory allocation and no memmove

## Sample

```C++
	StringBuffer<3 /* static allocation of 3 bytes  using template */> buffer;
	buffer.empty(); // -> return bool true
	buffer.append('1'); // -> return bool true
	buffer[0]; // -> return byte '1'
	buffer.append("23"); // -> return bool true
	buffer.indexOf("23"); // -> return uint16_t 1
	buffer.append('4'); // -> return bool false
	buffer.full(); // -> return bool true
	buffer.pop_first(); // return byte '1'
	buffer.length(); // -> return uint16_t 2
```

## Tests

A unittest suite is available under /test using GTest framework. Please use it as extented documentation.
