#include "uuidv7.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_unprecedented(void) {
  struct TestCase {
    uint64_t unix_ts_ms;
    uint8_t rand_bytes[10];
    char string[37];
  } cases[] = {{0x0123456789ab,
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                "01234567-89ab-7000-8000-000000000000"},
               {0xba9876543210,
                {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
                "ba987654-3210-7fff-bfff-ffffffffffff"},
               {0x17f22e279b0,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                "017f22e2-79b0-7cc3-98c4-dc0c0c07398f"}};
  int n_cases = sizeof(cases) / sizeof(struct TestCase);

  struct TestCase *e = cases;
  for (int i = 0; i < n_cases; i++, e++) {
    uint8_t uuid[16];
    int status = uuidv7_generate(uuid, e->unix_ts_ms, e->rand_bytes, NULL);
    assert(status == UUIDV7_STATUS_UNPRECEDENTED);
    assert(uuidv7_status_n_rand_consumed(status) == 10);

    char buffer[37];
    uuidv7_to_string(uuid, buffer);
    assert(strcmp(buffer, e->string) == 0);
  }
}

void test_with_prev(void) {
  struct TestCase {
    uint64_t unix_ts_ms;
    uint8_t rand_bytes[10];
    int8_t status;
    char string[37];
  } cases[] = {{0x0123456789ab,
                {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                UUIDV7_STATUS_NEW_TIMESTAMP,
                "01234567-89ab-7000-8000-000000000000"},
               {0x17f22e279b0,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_NEW_TIMESTAMP,
                "017f22e2-79b0-7cc3-98c4-dc0c0c07398f"},
               {0x17f22e279b0,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_COUNTER_INC,
                "017f22e2-79b0-7cc3-98c4-dc0dfcc358c4"},
               {0x17f22e279b0,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_COUNTER_INC,
                "017f22e2-79b0-7cc3-98c4-dc0efcc358c4"},
               {0x17f22e279b0 - 1,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_COUNTER_INC,
                "017f22e2-79b0-7cc3-98c4-dc0ffcc358c4"},
               {0x17f22e279b0 - 10000,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_COUNTER_INC,
                "017f22e2-79b0-7cc3-98c4-dc10fcc358c4"},
               {0x17f22e279b0 - 10001,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_CLOCK_ROLLBACK,
                "017f22e2-529f-7cc3-98c4-dc0c0c07398f"},
               {0xba9876543210,
                {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
                UUIDV7_STATUS_NEW_TIMESTAMP,
                "ba987654-3210-7fff-bfff-ffffffffffff"},
               {0xba9876543210,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_TIMESTAMP_INC,
                "ba987654-3211-7cc3-98c4-dc0c0c07398f"},
               {0xba9876543210,
                {0xfc, 0xc3, 0x58, 0xc4, 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                UUIDV7_STATUS_COUNTER_INC,
                "ba987654-3211-7cc3-98c4-dc0dfcc358c4"}};
  int n_cases = sizeof(cases) / sizeof(struct TestCase);

  uint8_t uuid[16] = {0};
  struct TestCase *e = cases;
  for (int i = 0; i < n_cases; i++, e++) {
    int status = uuidv7_generate(uuid, e->unix_ts_ms, e->rand_bytes, uuid);
    assert(status == e->status);

    char buffer[37];
    uuidv7_to_string(uuid, buffer);
    assert(strcmp(buffer, e->string) == 0);
  }
}

void test_to_string(void) {
  struct TestCase {
    uint8_t bytes[16];
    char string[37];
  } cases[] = {{{0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0x70, 0x00, 0x80, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                "01234567-89ab-7000-8000-000000000000"},
               {{0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x7f, 0xff, 0xbf, 0xff,
                 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
                "ba987654-3210-7fff-bfff-ffffffffffff"},
               {{0x01, 0x7f, 0x22, 0xe2, 0x79, 0xb0, 0x7c, 0xc3, 0x98, 0xc4,
                 0xdc, 0x0c, 0x0c, 0x07, 0x39, 0x8f},
                "017f22e2-79b0-7cc3-98c4-dc0c0c07398f"}};
  int n_cases = sizeof(cases) / sizeof(struct TestCase);

  struct TestCase *e = cases;
  for (int i = 0; i < n_cases; i++, e++) {
    char buffer[37];
    uuidv7_to_string(e->bytes, buffer);
    assert(strcmp(buffer, e->string) == 0);
  }
}

int main(void) {
#ifndef NDEBUG
  test_unprecedented();
  fprintf(stderr, "  %s: ok\n", "test_unprecedented");
  test_with_prev();
  fprintf(stderr, "  %s: ok\n", "test_with_prev");
  test_to_string();
  fprintf(stderr, "  %s: ok\n", "test_to_string");

  return 0;
#else
  fprintf(stderr, "error: NDEBUG must be unset\n");
  return 1;
#endif
}
