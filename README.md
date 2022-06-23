# uuidv7.h - Single-file C/C++ UUIDv7 Library

Examples:

```c
#include "uuidv7.h"

#include <stdio.h>
#include <sys/random.h>
#include <time.h>

int main(void) {
  // use high-level APIs that require concrete `uuidv7_new()` implementation
  char text[37];
  for (int i = 0; i < 8; i++) {
    uuidv7_new_string(text);
    printf("%s\n", text);
  }

  // generate a UUIDv7 with the current Unix time using the low-level APIs
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  uint64_t unix_ts_ms = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

  uint8_t rand_bytes[10];
  getentropy(rand_bytes, 10);

  uint8_t uuid[16];
  uuidv7_generate(uuid, unix_ts_ms, rand_bytes, NULL);
  uuidv7_to_string(uuid, text);
  printf("%s\n", text);

  // generate another while guaranteeing ascending order of UUIDs
  clock_gettime(CLOCK_REALTIME, &tp);
  unix_ts_ms = (uint64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
  getentropy(rand_bytes, 10);

  int status = uuidv7_generate(uuid, unix_ts_ms, rand_bytes, uuid);
  if (status == UUIDV7_STATUS_CLOCK_ROLLBACK)
    return 1; // error: clock moved backward by more than 10 seconds
  uuidv7_to_string(uuid, text);
  printf("%s\n", text);

  return 0; // success
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

  getentropy(rand_bytes, n_rand_consumed);
  int8_t status = uuidv7_generate(uuid_prev, unix_ts_ms, rand_bytes, uuid_prev);
  n_rand_consumed = uuidv7_status_n_rand_consumed(status);

  for (int i = 0; i < 16; i++)
    uuid_out[i] = uuid_prev[i];
  return status;
}
```

See [draft-peabody-dispatch-new-uuid-format-04](https://www.ietf.org/archive/id/draft-peabody-dispatch-new-uuid-format-04.html).

## Primary function

```c
static inline int8_t uuidv7_generate(uint8_t *uuid_out, uint64_t unix_ts_ms,
                                     const uint8_t *rand_bytes,
                                     const uint8_t *uuid_prev);
```

Generates a new UUIDv7 from the given Unix time, random bytes, and previous UUID.

- **Parameters:**
  - `uuid_out`: 16-byte byte array where the generated UUID is stored.
  - `unix_ts_ms`: Current Unix time in milliseconds.
  - `rand_bytes`: At least 10-byte byte array filled with random bytes. This
    function consumes the leading 4 bytes or the whole 10 bytes per call
    depending on the conditions. `uuidv7_status_n_rand_consumed()` maps the
    return value of this function to the number of random bytes consumed.
  - `uuid_prev`: 16-byte byte array representing the immediately preceding UUID,
    from which the previous timestamp and counter are extracted. This may be
    NULL if the caller does not care the ascending order of UUIDs within the
    same timestamp. This may point to the same location as `uuid_out`; this
    function reads the value before writing.
- **Returns:**
  - One of the `UUIDV7_STATUS_*` codes that describe the characteristics of
    generated UUIDs. Callers can usually ignore the status unless they need to
    guarantee the monotonic order of UUIDs or fine-tune the generation process.

See [API reference](https://liosk.github.io/uuidv7-h/uuidv7_8h.html) for the
full list of provided functions.

## Field and bit layout

This implementation produces identifiers with the following bit layout:

```text
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          unix_ts_ms                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          unix_ts_ms           |  ver  |        counter        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|var|                        counter                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                             rand                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

Where:

- The 48-bit `unix_ts_ms` field is dedicated to the Unix timestamp in
  milliseconds.
- The 4-bit `ver` field is set at `0111`.
- The 42-bit `counter` field accommodates the sequence counter that ensures the
  monotonic order of IDs generated within the same millisecond. The counter is
  incremented by one for each new ID generated within the same timestamp and is
  randomly initialized whenever the `unix_ts_ms` changes. The `counter` is
  filled with a random number if `NULL` is passed as `uuid_prev` parameter.
- The 2-bit `var` field is set at `10`.
- The remaining 32 `rand` bits are filled with a random number.

In the rare circumstances where the 42-bit `counter` field reaches its maximum
value, this library increments the `unix_ts_ms` ahead of the actual time;
therefore, the `unix_ts_ms` may have a larger value than that of the real-time
clock. This library goes on with such larger `unix_ts_ms` values caused by
counter overflows and system clock rollbacks as long as the difference from the
system clock is small enough. If the system clock moves back by more than ten
seconds, this library ignores the timestamp and counter embedded in the
`uuid_prev` and thus breaks the monotonic order of generated identifiers.

## License

Licensed under the Apache License, Version 2.0.
