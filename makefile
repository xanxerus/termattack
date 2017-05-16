all: Test

Test: Test.c
	gcc Test.c -lncurses -o Test

clean:
	rm Test
	rm *.o
