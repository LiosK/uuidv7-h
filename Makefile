CFLAGS   = -Wall -Wextra -pedantic-errors -std=c99
CXXFLAGS = -Wall -Wextra -pedantic-errors -std=c++98

test: uuidv7_test.c.out uuidv7_test.cxx.out
	./uuidv7_test.c.out
	./uuidv7_test.cxx.out

clean:
	$(RM) *.out

%.c.out: %.c uuidv7.h
	$(CC) $(CFLAGS) -o$@ $<

%.cxx.out: %.c uuidv7.h
	$(CXX) $(CXXFLAGS) -o$@ $<
