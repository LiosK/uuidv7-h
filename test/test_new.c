#include "uuidv7.h"

#include <assert.h>
#include <math.h>
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

void test_constant_and_random_bits(void) {
  // count '1' of each bit
  uint32_t bins[128] = {0};
  for (int i = 0; i < N_SAMPLES; i++) {
    uint8_t uuid[16];
    uuidv7_from_string(samples[i], uuid);
    for (int j = 0; j < 16; j++) {
      for (int k = 7; k >= 0; k--) {
        bins[j * 8 + k] += uuid[j] & 1;
        uuid[j] >>= 1;
      }
    }
  }

  // test if constant bits are all set to 1 or 0
  assert(bins[48] == 0);         // version bit 48
  assert(bins[49] == N_SAMPLES); // version bit 49
  assert(bins[50] == N_SAMPLES); // version bit 50
  assert(bins[51] == N_SAMPLES); // version bit 51
  assert(bins[64] == N_SAMPLES); // variant bit 64
  assert(bins[65] == 0);         // variant bit 65

  // test if random bits are set to 1 at ~50% probability
  // set margin based on binom dist 99.999% confidence interval
  double margin = 4.417173 * sqrt(0.5 * 0.5 / (double)N_SAMPLES);
  for (int i = 96; i < 128; i++)
    assert(fabs((double)bins[i] / (double)N_SAMPLES - 0.5) < margin);
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
  test_constant_and_random_bits();
  fprintf(stderr, "  %s: ok\n", "test_constant_and_random_bits");

  return 0;
}
#endif
