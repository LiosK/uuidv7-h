#include "uuidv7.h"

#include <assert.h>
#include <ctype.h>
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

void test_from_to_string(void) {
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
    char string[37];
    uuidv7_to_string(e->bytes, string);
    assert(strcmp(string, e->string) == 0);

    uint8_t uuid[16];
    int err = uuidv7_from_string(e->string, uuid);
    assert(err == 0);
    assert(memcmp(uuid, e->bytes, 16) == 0);

    for (int i = 0; e->string[i] != '\0'; i++) {
      string[i] = toupper(e->string[i]);
    }
    string[36] = '\0';
    err = uuidv7_from_string(string, uuid);
    assert(err == 0);
    assert(memcmp(uuid, e->bytes, 16) == 0);
  }
}

void test_from_string_error(void) {
  char cases[][40] = {
      "",
      " 01815515-70af-73bb-a597-725c2086f131",
      "01815515-70af-73bb-a597-725d90351474 ",
      " 01815515-70af-73bb-a597-725eb688399d ",
      "+01815515-70af-73bb-a597-725f825e57d7",
      "-01815515-70af-73bb-a597-72602859e906",
      "+1815515-70af-73bb-a597-7261115ae9fd",
      "-1815515-70af-73bb-a597-7262c43b6483",
      "0181551570af73bba5977263d33e981a",
      "01815515_70af-73bb-a597-726461ccf7f4",
      "01815515-70af 73bb-a597-726523b5414d",
      "01815515-70af-73bb-a597-7266_8e4c2c3",
      "0181-515-70af-73bb-a597-726778070225",
      "01815515-70af-7 bb-a597-726849072769",
      "0181g515-70af-73bb-a597-72699ab58e63",
      "01815515-70af-73bb-a597-726ladf25973",
      "01815515-70af-73zb-a597-726b873b50aa",
  };
  const int n_cases = sizeof(cases) / sizeof(cases[0]);

  for (int i = 0; i < n_cases; i++) {
    uint8_t uuid[16];
    int err = uuidv7_from_string(cases[i], uuid);
    assert(err != 0);
  }
}

#ifndef NDEBUG
int main(void) {
  test_unprecedented();
  fprintf(stderr, "  %s: ok\n", "test_unprecedented");
  test_with_prev();
  fprintf(stderr, "  %s: ok\n", "test_with_prev");
  test_from_to_string();
  fprintf(stderr, "  %s: ok\n", "test_from_to_string");
  test_from_string_error();
  fprintf(stderr, "  %s: ok\n", "test_from_string_error");

  return 0;
}
#endif
