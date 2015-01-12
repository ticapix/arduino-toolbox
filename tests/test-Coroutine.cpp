#include <gtest/gtest.h>

#include <Arduino.h>
#include <Coroutine.h>

defCoroutine(int, sequence1) {
  int i;
  cBegin();
  for (i = 0; i < 5; i += 2) {
    cYield( i );
    cYield( i+1 );
  }
  cEnd(-1);
};

defCoroutine(int, sequence) {
  int i, j;
  sequence1 s1;
  
  cBegin(); // TODO put extra arg in the cBegin() macro
  // cYield(s1)
  
  while(cResume(j, s1))
    cYield(j);
  for (i = 0; i < 5; i += 2) {
    cYield(i);
    cYield(i+1);
  }
  cEnd(-1);
};

TEST(Coroutine, simple) {
  sequence corout1;

  int n;
  while (cResume(n, corout1)) {
    printf("%d\n", n);
    delay(100);
  }
}

