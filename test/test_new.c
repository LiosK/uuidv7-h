#include "uuidv7.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define N_SAMPLES 100000
static char samples[N_SAMPLES][40];

void setup(void) {
  for (int i = 0; i < N_SAMPLES; i++) {
    int status = uuidv7_new_string(samples[i]);
    assert(status >= 0);
  }
}

void test_format(void) {
  for (int i = 0; i < N_SAMPLES; i++) {
    assert(strlen(samples[i]) == 36);
    for (int j = 0; j < 36; j++) {
      char c = samples[i][j];
      if (j == 8 || j == 13 || j == 18 || j == 23)
        assert(c == '-');
      else if (j == 14)
        assert(c == '7');
      else if (j == 19)
        assert(c == '8' || c == '9' || c == 'a' || c == 'b');
      else
        assert(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
               c == '5' || c == '6' || c == '7' || c == '8' || c == '9' ||
               c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' ||
               c == 'f');
    }
  }
}

void test_order(void) {
  for (int i = 1; i < N_SAMPLES; i++)
    assert(strcmp(samples[i - 1], samples[i]) < 0);
}

void test_timestamp_and_counter(void) {
  uint8_t prev[16], curr[16];
  uuidv7_from_string(samples[0], prev);
  for (int i = 1; i < N_SAMPLES; i++) {
    uuidv7_from_string(samples[i], curr);
    assert(memcmp(prev, curr, 6) < 0 ||
           (memcmp(prev, curr, 6) == 0 && memcmp(&prev[6], &curr[6], 6) < 0));
    memcpy(curr, prev, 16);
  }
}

#ifndef NDEBUG
int main(void) {
  setup();

  test_format();
  fprintf(stderr, "  %s: ok\n", "test_format");
  test_order();
  fprintf(stderr, "  %s: ok\n", "test_order");
  test_timestamp_and_counter();
  fprintf(stderr, "  %s: ok\n", "test_timestamp_and_counter");

  return 0;
}
#endif
