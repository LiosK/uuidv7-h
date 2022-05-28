# uuidv7.h - Single-file C/C++ UUIDv7 Library

Examples:

```c
#include "uuidv7.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
  // generate a UUIDv7 with the current Unix time
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  uint64_t unix_ts_ms = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

  uint8_t rand_bytes[10];
  arc4random_buf(rand_bytes, 10);

  uint8_t uuid[16];
  char text[37];
  uuidv7_generate(uuid, unix_ts_ms, rand_bytes, NULL);
  uuidv7_to_string(uuid, text);
  printf("%s\n", text);

  // generate another while guaranteeing ascending order of UUIDs
  clock_gettime(CLOCK_REALTIME, &tp);
  unix_ts_ms = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  arc4random_buf(rand_bytes, 10);

  int status = uuidv7_generate(uuid, unix_ts_ms, rand_bytes, uuid);
  if (status == UUIDV7_STATUS_CLOCK_ROLLBACK)
    return 1; // clock moved backward by more than 10 seconds
  uuidv7_to_string(uuid, text);
  printf("%s\n", text);

  // use user-defined `uuidv7_new()` function
  for (int i = 0; i < 8; i++) {
    uuidv7_new(uuid);
    uuidv7_to_string(uuid, text);
    printf("%s\n", text);
  }

  return 0;
}

/**
 * Generates a new UUIDv7 with the current Unix time.
 *
 * `uuidv7.h` provides the primitive `uuidv7_generate()` function only. Users
 * have to integrate a real-time clock, cryptographically strong random number
 * generator, and shared state storage available in the target platform.
 *
 * @warning This example uses static variables and is NOT thread-safe.
 */
int uuidv7_new(uint8_t *uuid_out) {
  static uint8_t uuid_prev[16] = {0};
  static uint8_t rand_bytes[10] = {0};
  static int n_rand_consumed = 10;

  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  uint64_t unix_ts_ms = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

  arc4random_buf(rand_bytes, n_rand_consumed);
  int8_t status = uuidv7_generate(uuid_prev, unix_ts_ms, rand_bytes, uuid_prev);
  n_rand_consumed = uuidv7_status_n_rand_consumed(status);

  for (int i = 0; i < 16; i++)
    uuid_out[i] = uuid_prev[i];
  return status;
}
```
