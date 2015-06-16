CFLAGS = -g3 -Wall -Werror

glex_tests: glex_tests_main.o glex_test_noopts.o
	$(CC) -o glex_tests $+

glex_tests_main.c: glex.h glex_tests.h
glex_tests_noopts.c: glex.h glex_tests.h

clean:
	rm -f glex_tests *.o
