arduino-ringbuffer
==================

Simple RingBuffer for arduino, providing a StringBuffer container with no dynamic memory allocation and no memmove

```
	StringBuffer<3 /* static allocation of 3 bytes  using template */> buffer;
	buffer.empty(); // -> return bool true
	buffer.append('1'); // -> return bool true
	buffer[0]; // -> return byte '1'
	buffer.append('2'); // -> return bool true
	buffer.append('3'); // -> return bool true
	buffer.indexOf("23"); // -> return uint16_t 1
	buffer.append('4'); // -> return bool false
	buffer.full(); // -> return bool true
```
