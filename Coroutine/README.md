# The Coroutine Macros

The goal is to have a light Coroutine toolbox.

## Examples

### basic coroutine

Note: the goal is *not* to implement generators, so in the current implementation, `YIELD()` doesn't return a value.

```cpp
COROUTINE(int, CountNumber);
CORO_ARG(CountNumber, int, limit);
CORO_VAR(int, count);
CORO_START();
{
	count = 0;
	while (count < limit) {
		printf("%d\n", count++);
		YIELD();
	}
}
CORO_RETURN(count);
CORO_END();
```

### doing an asynchronous call

```cpp
COROUTINE(int, CountChar);
CORO_ARG(CountChar, char, from);
CORO_ARG(CountChar, char, to);
CORO_START();
{
	AWAIT((new CountNumber())->set_limit(to - from));
	printf("%c-%c = %d\n", to, from, RESULT(CountNumber));
}
CORO_RETURN(0);
CORO_END();
```

### a timing out coroutine

```cpp
COROUTINE(int, NeverEnding);
CORO_START();
{
	while (true)
		YIELD();
}
CORO_RETURN(0);
CORO_END();
```

## How to use the above examples

On the Arduino, the code would look like
```cpp
#define NUM_CORO 3
ICoroutine* coroutines[NUM_CORO];

void setup() {
	coroutines[0] = new NeverEnding();
	coroutines[1] = (new CountChar())->set_from('A')->set_to('B');
	coroutines[2] = (new CountChar())->set_from('B')->set_to('F');
}

void loop() {
  for (int8_t count = 0; count < NUM_CORO; ++count) {
    if (coroutines[i] == nullptr)
      continue;
    ICoroutine* coro = coroutines[i];
		if (coro->live()) {
			coro->run();
		} else {
			if (coro->has_timeout()) {
			  // DO SOMETHING
			} else {
				auto result = ((CountNumber*)(*it))->result(); // because in this particular case, we know what cast to apply.
				// DO SOMETHING
			}
			coro = nullptr;
		}
	}
}
```
