all: termattack

termattack: termattack.o
	gcc termattack.o board.o -lncurses -o termattack 

termattack.o: board.o
	gcc -c termattack.c -lncurses

board.o: board.c board.h
	gcc -c board.c board.h -lncurses

clean:
	rm *.o
	rm *.gch
	rm termattack

go: clean all
	./termattack
