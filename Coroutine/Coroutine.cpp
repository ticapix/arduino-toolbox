// Coroutines.cpp : Defines the entry point for the console application.
//

#include <list>
#include <stdio.h>
#include "Coroutine.h"
#include "Arduino.h"

//////////// EXAMPLE OF ASYNC Delay implementation ////////////
/*
 * This coroutine will simply exit when timing out
 */

CORO_START(Delay)
{
	while (true)
		YIELD()
}
CORO_RETURN(0)
CORO_END()


//////////// EXAMPLE OF MAIN SCHEDULING LOOP implementation ////////////
/*
 * It can be the only thing you have the call from the arduino loop() function
 */

void schedule_coro(ICoroutine* coroutines[], uint8_t size, coro_callback clbk) {
	bool has_coro = true;
	while (has_coro) {
		has_coro = false;
		for (uint8_t idx = 0; idx < size; ++idx) {
			ICoroutine* coro = coroutines[idx];
			if (coro == nullptr)
				continue;
			has_coro = true;
			if (coro->live()) {
				coro->run();
			} else {
				clbk(idx, coro);
				delete coro;
				coroutines[idx] = nullptr;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////

//http://en.cppreference.com/w/cpp/memory/new/operator_delete
/*
*  TODO: overload new.
* if free_ram() too small, redirect to default TIMEOUT error result
* overload delete
* if default TIMEOUT error, do not free
*/

///////////////////////////////////////////////////////////////////////
