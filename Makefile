CFLAGS   = -Wall -Wextra -pedantic-errors -std=c99
CXXFLAGS = -Wall -Wextra -pedantic-errors -std=c++98

.PHONY: test test_core test_unix clean docs

test: test_core test_unix

test_core: uuidv7_test_core.c.out uuidv7_test_core.cxx.out
	./uuidv7_test_core.c.out
	./uuidv7_test_core.cxx.out

test_unix: uuidv7_test_unix.c.out uuidv7_test_unix.cxx.out
	./uuidv7_test_unix.c.out
	./uuidv7_test_unix.cxx.out

clean:
	$(RM) *.out

docs:
	$(RM) -r docs
	doxygen

%.c.out: %.c uuidv7.h
	$(CC) $(CFLAGS) -o$@ $<

%.cxx.out: %.c uuidv7.h
	$(CXX) $(CXXFLAGS) -o$@ $<
