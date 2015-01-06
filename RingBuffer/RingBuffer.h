#ifndef __STRING_BUFFER_H__
#define __STRING_BUFFER_H__

#include <Arduino.h>

/*
 * DECLARATION
 */

template<uint16_t Size, typename T>
class RingBuffer {

public:

	RingBuffer();

	uint16_t append(T c);

	bool full() const;

	bool empty() const;

	void clear();

	uint16_t length() const;
	
	static uint16_t capacity();

	const T operator[](uint16_t idx) const;

	T pop_first();

	uint16_t pop_firsts(uint16_t n);

	T pop_last();

	const T* buffer();

	static const uint16_t END = -1;

protected:

	bool _is_continuous() const;

	void _make_continuous();

	T _buffer[Size];
	uint16_t _start, _end;
	uint16_t _length;
	uint16_t _capacity;
};

template<uint16_t Size, typename T>
const uint16_t RingBuffer<Size, T>::END;


template<uint16_t Size>
class StringBuffer: public RingBuffer<Size, byte> {

public:

	using RingBuffer<Size, byte>::END;
	using RingBuffer<Size, byte>::append;

	uint16_t append(String str);

	uint16_t index_of(String substr, uint16_t offset = 0);

	bool starts_with(String substr);
};

/*
 * IMPLEMENTATION
 */

template<uint16_t Size, typename T>
RingBuffer<Size, T>::RingBuffer() :
		_start(0), _end(0), _length(0), _capacity(Size) {
}

template<uint16_t Size, typename T>
uint16_t RingBuffer<Size, T>::append(T c) {
	if (full())
		return 0;
	_buffer[_end] = c;
	_end = (_end + 1) % capacity();
	++_length;
	return 1;
}
template<uint16_t Size, typename T>
bool RingBuffer<Size, T>::full() const {
	return capacity() == length();
}

template<uint16_t Size, typename T>
bool RingBuffer<Size, T>::empty() const {
	return length() == 0;
}

template<uint16_t Size, typename T>
void RingBuffer<Size, T>::clear() {
	_start = 0;
	_end = 0;
	_length = 0;
}

template<uint16_t Size, typename T>
uint16_t RingBuffer<Size, T>::length() const {
	return _length;
}

template<uint16_t Size, typename T>
uint16_t RingBuffer<Size, T>::capacity() {
	return Size;
}

template<uint16_t Size, typename T>
const/* can't modify the returned value */T RingBuffer<Size, T>::operator[](
		uint16_t idx) const {
	if (idx >= length())
		return _buffer[0]; // declared as undefined behavior for the user
	return _buffer[(_start + idx) % capacity()];
}

template<uint16_t Size, typename T>
T RingBuffer<Size, T>::pop_first() {
	if (empty())
		return _buffer[0];
	uint8_t t = _start;
	_start = (_start + 1) % capacity();
	--_length;
	return _buffer[t];
}

template<uint16_t Size, typename T>
uint16_t RingBuffer<Size, T>::pop_firsts(uint16_t n) {
	uint16_t count = 0;
	while (!empty() && n--) {
		pop_first();
		++count;
	}
	return count;
}

template<uint16_t Size, typename T>
T RingBuffer<Size, T>::pop_last() {
	if (empty())
		return _buffer[0];
	--_length;
	_end = (_start + length()) % capacity();
	return _buffer[_end];
}

/* can cause a call to memmove */
template<uint16_t Size, typename T>
const T* RingBuffer<Size, T>::buffer() {
	if (!_is_continuous())
		_make_continuous();
	return _buffer + _start;
}

template<uint16_t Size, typename T>
bool RingBuffer<Size, T>::_is_continuous() const {
	/* is empty */
	bool res = empty();
	/* content is somewhere in the middle of buffer */
	res = res || _start < _end;
	/* buffer has not cyclied yet */
	res = res || _end == 0;
	return res;
}

template<uint16_t Size, typename T>
void RingBuffer<Size, T>::_make_continuous() {
	while (!_is_continuous()) {
		T tmp = _buffer[0];
		memmove(&(_buffer[0]), &(_buffer[1]), (_end) * sizeof (T));
		memmove(&(_buffer[_start - 1]), &(_buffer[_start]), (capacity() - _start) * sizeof (T));
		_buffer[capacity() - 1] = tmp;
		_start = (_start - 1) % capacity();
		_end = (_start + length()) % capacity();
	}
}

template<uint16_t Size>
uint16_t StringBuffer<Size>::append(String str) {
        uint16_t count = 0;
	for (uint16_t i = 0; i < str.length(); ++i) {
		if (!this->append(str[i])) {
			return count;
		}
		++count;
	}
	return count;
}

template<uint16_t Size>
uint16_t StringBuffer<Size>::index_of(String substr, uint16_t offset) {
        if (this->empty())
    		return StringBuffer<Size>::END;
        if (substr.length() + offset > this->length())
		return StringBuffer<Size>::END;
	for (uint16_t i = offset; i <= this->length() - substr.length(); ++i) {
		bool match = true;
		for (uint16_t j = 0; j < substr.length() && match; ++j) {
			match = match && ((*this)[i + j] == substr[j]);
		}
		if (match)
			return i;
	}
	return StringBuffer<Size>::END;
}

template<uint16_t Size>
bool StringBuffer<Size>::starts_with(String substr) {
	return (this->index_of(substr) == 0);
}

#endif
