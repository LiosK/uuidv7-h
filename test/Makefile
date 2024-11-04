CFLAGS   = -I.. -Wall -Wextra -pedantic-errors -std=c99
CXXFLAGS = -I.. -Wall -Wextra -pedantic-errors -std=c++98

.PHONY: test test_core test_unix clean

test: test_core test_unix

test_core: test_core.c.out test_core.cxx.out
	./test_core.c.out
	./test_core.cxx.out

test_unix: test_unix.c.out test_unix.cxx.out
	./test_unix.c.out
	./test_unix.cxx.out

clean:
	$(RM) *.out

%.c.out: %.c ../uuidv7.h
	$(CC) $(CFLAGS) -o$@ $<

%.cxx.out: %.c ../uuidv7.h
	$(CXX) $(CXXFLAGS) -o$@ $<
