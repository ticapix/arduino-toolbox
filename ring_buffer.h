#ifndef __STRING_BUFFER_H__
#define __STRING_BUFFER_H__

template<uint16_t Size, typename T>
class RingBuffer {

  public:
  
    RingBuffer()
      : _start(0),
        _end(0),
        _length(0),
        _capacity(Size)
    { }

    bool append(T c) {
      if (full())
        return false;
      _buffer[_end] = c;
      _end = (_end + 1) % _capacity;
      ++_length;
      return true;
    }

    bool full() {
      return _capacity == _length;
    }

    bool empty() {
      return _length == 0;
    }

    void clear() {
      _start = 0;
      _end = 0;
      _length = 0;
    }

    uint16_t length() {
      return _length;
    }

    const /* can't modify the returned value */ T operator[](uint16_t idx) const { /* no modification inside this fct */
      if (idx > _length)
        return (*this)[0]; // declared as undefined behavior for the user
      return _buffer[(_start + idx) % _capacity];
    }

    T pop_first() {
      uint8_t t = _start;
      _start = (_start + 1) % _capacity;
      --_length;
      return _buffer[t];
    }
    
  protected:
    
    T _buffer[Size];
    uint16_t _start, _end;
    uint16_t _length;
    uint16_t _capacity;
};


template<uint16_t Size>
class StringBuffer : public RingBuffer <Size, byte> {
  
  public:
  
  using RingBuffer<Size, byte>::append;
  
  bool append(String str) {
    for (uint16_t i = 0; i < str.length(); ++i) {
      if (!this->append(str[i])) {
  	return false;
      }
    }
    return true;
  }
  
    int indexOf(String substr, uint16_t offset = 0) {
      if (offset > this->length())
        return -1;
      if (substr.length() + offset > this->length())
        return -1;
      for (uint16_t i = offset; i <= this->length() - substr.length(); ++i) {
        bool match = true;
        for (uint16_t j = 0; j < substr.length() && match; ++j) {
          match = match && ((*this)[i + j] == substr[j]);
        }
        if (match)
          return i;
      }
      return -1;
    }
};

#endif
