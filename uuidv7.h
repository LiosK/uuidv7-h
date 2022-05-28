/**
 * @file uuidv7.h Single-file C/C++ UUIDv7 Library
 *
 * Repository: https://github.com/LiosK/uuidv7-h
 *
 * Copyright 2022 LiosK
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef UUIDV7_H_BAEDKYFQ
#define UUIDV7_H_BAEDKYFQ

#include <stddef.h>
#include <stdint.h>

/**
 * Status code returned by `uuidv7_generate()`, indicating that the `unix_ts_ms`
 * passed was used because no preceding UUID was specified.
 */
#define UUIDV7_STATUS_UNPRECEDENTED (0)

/**
 * Status code returned by `uuidv7_generate()`, indicating that the `unix_ts_ms`
 * passed was used because it was greater than the previous one.
 */
#define UUIDV7_STATUS_NEW_TIMESTAMP (1)

/**
 * Status code returned by `uuidv7_generate()`, indicating that the counter was
 * incremented because the `unix_ts_ms` passed was no greater than the previous
 * one.
 */
#define UUIDV7_STATUS_COUNTER_INC (2)

/**
 * Status code returned by `uuidv7_generate()`, indicating that the previous
 * `unix_ts_ms` was incremented because the counter reached its maximum value.
 */
#define UUIDV7_STATUS_TIMESTAMP_INC (3)

/**
 * Status code returned by `uuidv7_generate()`, indicating that the monotonic
 * order of generated UUIDs was broken because the `unix_ts_ms` passed was less
 * than the previous one by more than ten seconds.
 */
#define UUIDV7_STATUS_CLOCK_ROLLBACK (4)

/**
 * Status code returned by `uuidv7_generate()`, indicating that an invalid
 * `unix_ts_ms` is passed.
 */
#define UUIDV7_STATUS_ERR_TIMESTAMP (-1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generates a new UUIDv7 with the current Unix time.
 *
 * This declaration suggests a recommended interface to generate a new UUIDv7
 * with the current time, default random number generator, and global shared
 * state holding the previously generated UUID. Users of this single-file
 * library need to prepare a concrete implementation (if necessary) by
 * integrating a real-time clock, cryptographically strong random number
 * generator, and shared state storage available in the target platform.
 *
 * @param uuid_out 16-byte byte array where the generated UUID is stored.
 * @return One of the `UUIDV7_STATUS_*` codes that describe the characteristics
 * of generated UUIDs or an implementation-dependent error code. Callers can
 * usually ignore the `UUIDV7_STATUS_*` code unless they need to guarantee the
 * monotonic order of UUIDs or fine-tune the generation process.
 */
int uuidv7_new(uint8_t *uuid_out);

/**
 * Generates a new UUIDv7 with the given Unix time, random number generator, and
 * previous UUID.
 *
 * @param uuid_out 16-byte byte array where the generated UUID is stored.
 * @param unix_ts_ms Current Unix time in milliseconds.
 * @param rand_bytes At least 10-byte byte array filled with random bytes. This
 * function consumes the leading 4 bytes or the whole 10 bytes per call
 * depending on the conditions. `uuidv7_status_n_rand_consumed()` maps the
 * return value of this function to the number of random bytes consumed.
 * @param uuid_prev 16-byte byte array representing the immediately preceding
 * UUID, from which the previous timestamp and counter are extracted. This may
 * be NULL if the caller does not care the ascending order of UUIDs within the
 * same timestamp. This may point to the same location as `uuid_out`; this
 * function reads the value before writing.
 * @return One of the `UUIDV7_STATUS_*` codes that describe the characteristics
 * of generated UUIDs. Callers can usually ignore the status unless they need to
 * guarantee the monotonic order of UUIDs or fine-tune the generation process.
 */
int8_t uuidv7_generate(uint8_t *uuid_out, uint64_t unix_ts_ms,
                       const uint8_t *rand_bytes, const uint8_t *uuid_prev) {
  if (unix_ts_ms > 0xffffffffffff) {
    return UUIDV7_STATUS_ERR_TIMESTAMP;
  }

  int8_t status;
  uint64_t timestamp = 0;
  if (uuid_prev == NULL) {
    status = UUIDV7_STATUS_UNPRECEDENTED;
    timestamp = unix_ts_ms;
  } else {
    for (int i = 0; i < 6; i++) {
      timestamp = (timestamp << 8) | uuid_prev[i];
    }

    if (unix_ts_ms > timestamp) {
      status = UUIDV7_STATUS_NEW_TIMESTAMP;
      timestamp = unix_ts_ms;
    } else if (unix_ts_ms + 10000 < timestamp) {
      // ignore prev if clock moves back by more than ten seconds
      status = UUIDV7_STATUS_CLOCK_ROLLBACK;
      timestamp = unix_ts_ms;
    } else {
      // increment prev counter
      uint64_t counter = uuid_prev[6] & 0x0f; // skip ver
      counter = (counter << 8) | uuid_prev[7];
      counter = (counter << 6) | (uuid_prev[8] & 0x3f); // skip var
      counter = (counter << 8) | uuid_prev[9];
      counter = (counter << 8) | uuid_prev[10];
      counter = (counter << 8) | uuid_prev[11];

      if (counter++ < 0x3ffffffffff) {
        status = UUIDV7_STATUS_COUNTER_INC;
        uuid_out[6] = counter >> 38; // ver + bits 0-3
        uuid_out[7] = counter >> 30; // bits 4-11
        uuid_out[8] = counter >> 24; // var + bits 12-17
        uuid_out[9] = counter >> 16; // bits 18-25
        uuid_out[10] = counter >> 8; // bits 26-33
        uuid_out[11] = counter;      // bits 34-41
      } else {
        // increment prev timestamp at counter overflow
        status = UUIDV7_STATUS_TIMESTAMP_INC;
        timestamp++;
      }
    }
  }

  uuid_out[0] = timestamp >> 40;
  uuid_out[1] = timestamp >> 32;
  uuid_out[2] = timestamp >> 24;
  uuid_out[3] = timestamp >> 16;
  uuid_out[4] = timestamp >> 8;
  uuid_out[5] = timestamp;

  for (int i = (status == UUIDV7_STATUS_COUNTER_INC) ? 12 : 6; i < 16; i++) {
    uuid_out[i] = *rand_bytes++;
  }

  uuid_out[6] = 0x70 | (uuid_out[6] & 0x0f); // set ver
  uuid_out[8] = 0x80 | (uuid_out[8] & 0x3f); // set var

  return status;
}

/**
 * Encodes a UUID in the 8-4-4-4-12 hexadecimal string representation.
 *
 * @param uuid 16-byte byte array representing the UUID to encode.
 * @param string_out Character array where the encoded string is stored. Its
 * length must be 37 (36 digits + NUL) or longer.
 */
void uuidv7_to_string(const uint8_t *uuid, char *string_out) {
  static const char DIGITS[] = "0123456789abcdef";
  for (int i = 0; i < 16; i++) {
    uint_fast8_t e = uuid[i];
    *string_out++ = DIGITS[e >> 4];
    *string_out++ = DIGITS[e & 15];
    if (i == 3 || i == 5 || i == 7 || i == 9) {
      *string_out++ = '-';
    }
  }
  *string_out = '\0';
}

/**
 * Determines the number of random bytes consumsed by `uuidv7_generate()` from
 * the `UUIDV7_STATUS_*` code returned.
 *
 * @param status `UUIDV7_STATUS_*` code returned by `uuidv7_generate()`.
 * @return `4` if `status` is `UUIDV7_STATUS_COUNTER_INC` or `10` otherwise.
 */
int uuidv7_status_n_rand_consumed(int8_t status) {
  return status == UUIDV7_STATUS_COUNTER_INC ? 4 : 10;
}

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* #ifndef UUIDV7_H_BAEDKYFQ */
