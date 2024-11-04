#include "uuidv7.h"

#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/random.h> // for macOS getentropy()

int uuidv7_new(uint8_t *uuid_out) {
  static uint8_t uuid_prev[16] = {0};
  static uint8_t rand_bytes[256] = {0};
  static size_t n_rand_consumed = sizeof(rand_bytes);

  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  uint64_t unix_ts_ms = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

  if (n_rand_consumed > sizeof(rand_bytes) - 10) {
    getentropy(rand_bytes, n_rand_consumed);
    n_rand_consumed = 0;
  }

  int8_t status = uuidv7_generate(uuid_prev, unix_ts_ms,
                                  &rand_bytes[n_rand_consumed], uuid_prev);
  n_rand_consumed += uuidv7_status_n_rand_consumed(status);

  memcpy(uuid_out, uuid_prev, 16);
  return status;
}
