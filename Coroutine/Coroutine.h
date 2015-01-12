
#ifndef INC_COROUTINES_H
#define INC_COROUTINES_H

/*
 * Coroutines are implemented as C++ functors. They are a shorthand for
 * writing explicit finite state machines.
 *
 * cResume will return false once all yields are done.
 * The return type of the coroutine must be default constructable.
 * cYield will not function correctly within a switch block.
 *
 * Example Usage: ( print 123456789 over serial )
 * -------------
 * #include "coroutines.h"
 *
 * defCoroutine( int, sequence ) {
 *   int i;
 *   cBegin;
 *     i = 0;
 *     for ( true )
 *       cYield( ++i );
 *   cEnd;
 * };
 *
 * sequence s1;
 * int n;
 *
 * void setup() {
 *   n = 0;
 *   Serial.begin(9600);
 * }
 *
 * void loop() {
 *   int x;
 *   while ( n < 9 && cResume( x, func ) )
 *     ++n;
 *     Serial.print( x );
 *     delay( 200 );
 * }
 */

#define defCoroutine(T, name) struct name : Coroutine<T>

#define cBegin() virtual RetType operator()() { switch (_state) { case 0:;

#define cYield(x) do { _state = __LINE__; return x;	\
  case __LINE__:; } while (0)

#define cStop { _live = false; return _defRet; }

#define cResume(v, C) (v = C(), C.Live())

#define cEnd(ret) _defRet = ret; }; cStop; }

template<class T>
class Coroutine {
 protected:
  typedef T RetType;
  int _state;
  bool _live;
  RetType _defRet;
 Coroutine() : _state( 0 ), _live( true ) {}
 public:
  bool Live() { return _live; }
  /* bool Reset() {_live = true; _state = 0;} */
};

#endif
