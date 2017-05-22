all: Test

Test: Test.c
	gcc Test.c -lncurses -o Test

clean:
	rm Test
#~ 	rm *.o

go: clean all
	./Test
