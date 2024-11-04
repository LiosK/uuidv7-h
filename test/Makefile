CFLAGS   = -I.. -Wall -Wextra -pedantic-errors
CXXFLAGS = -I.. -Wall -Wextra -pedantic-errors

.PHONY: test test_core test_new_unix clean

test: test_core test_new_unix

test_core: test_core.c.out test_core.cxx.out
	./test_core.c.out
	./test_core.cxx.out

test_new_unix: test_new_unix.c.out test_new_unix.cxx.out
	./test_new_unix.c.out
	./test_new_unix.cxx.out

clean:
	$(RM) *.out

test_core.c.out: test_core.c ../uuidv7.h
	$(CC) $(CFLAGS) -std=c99 -o$@ $<

test_core.cxx.out: test_core.c ../uuidv7.h
	$(CXX) $(CXXFLAGS) -std=c++98 -o$@ $<

test_new_%.c.out: impl_new_%.c test_new.c ../uuidv7.h
	$(CC) $(CFLAGS) -o$@ $< test_new.c

test_new_%.cxx.out: impl_new_%.c test_new.c ../uuidv7.h
	$(CXX) $(CXXFLAGS) -o$@ $< test_new.c
