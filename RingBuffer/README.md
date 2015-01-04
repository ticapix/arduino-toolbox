# RingBuffer

Simple RingBuffer for arduino, providing a StringBuffer container with no dynamic memory allocation.
The purpose is to avoid heap fragmentation, out of memory crashes with multiple malloc/free new/delete.

## Sample

```C++
	StringBuffer<3> buffer; // static allocation
	buffer.empty(); // -> return bool true | | | |
	buffer.append('1'); // -> return bool true |1| | |
	buffer[0]; // -> return byte '1'
	buffer.append("23"); // -> return bool true |1|2|3|
    buffer.starts_with("23"); // -> return bool true |1|2|3|
	buffer.index_of("23"); // -> return uint16_t 1
	buffer.append('4'); // -> return bool false
	buffer.full(); // -> return bool true
	buffer.pop_first(); // return byte '1' | |2|3|
	buffer.pop_last(); // return byte '3' | |2| |
	buffer.length(); // -> return uint16_t 1
	buffer.buffer(); // return T* on a continuous memory mapping (no dynamic allocation, only memmove)
```

## Under the hood

`StringBuffer<Size>` is a specialization of `RingBuffer<Size, T = byte>`
Internally, an array of type `T` and size `Size` is statically allocated.
Two indexes are pointing to the beginning of the data and respectively end of the data, in the array.
```
	 start              end
	   |                 |
	   v                 v
	+-----------------------+
	| | | | | | | | | | | | |  buffer with capacity of 11 items, containing 9 elements.
	+-----------------------+
```
 After several `append` and `pop_first`, the pointers will cycle back to the beginning to the buffer. All arithmetics operations on indexes are using `% (mod)` operation.
 
```
	    end            start
	     |               |
	     v               v
	+-----------------------+
	| | | | | | | | | | | | |  buffer with capacity of 11 items, containing 4 elements.
	+-----------------------+
```

The arithmetic is not visible by the user. It appears as continuous.

```C++
	for (uint16_t i = 0; i < buffer.length(); ++i) {
		printf("%c", b[i]);
	}
	printf("\n");
	// OR
	printf("%*s\n", buff.length(), buff.buffer()); // Calling buffer() MAY cause a call to memmove
```

## Tests

A unittest suite is available under `/test` using GTest framework. Please use it as extented documentation.
