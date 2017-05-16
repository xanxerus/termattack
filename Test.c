#include <stdio.h>
#include <string.h>
#include <ncurses.h>

int main(){
	initscr();

	if(has_colors() == FALSE){
		endwin();
		printf("Your terminal does not support color\n");
		return 1;
	}
	
	start_color();
	/*
	 * 1 is red bg
	 * 2 is blue bg
	 * 3 is white bg
	 */
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);
	
	int maxx, maxy;
	getmaxyx(stdscr, maxy, maxx);
	
	if( maxx < 20 || maxy < 10 ){
		endwin();
		printf("Your terminal (%dx%d) is too small. Must be at least 20x10\n", maxx, maxy);
		return 2;
	}
	
	int colStart = (maxx-20)>>1;
	int rowStart = (maxy-10)>>1;
	
	
	attron(COLOR_PAIR(1));
	for(int r = rowStart; r < rowStart+4; r++){
		for(int c = colStart; c < colStart+20; c++){
			mvaddch(r, c, ' ');
		}
	}
	attroff(COLOR_PAIR(1));

	attron(COLOR_PAIR(3));
	for(int r = rowStart+4; r < rowStart+6; r++){
		for(int c = colStart+4; c < colStart+8; c++){
			mvaddch(r, c, ' ');
		}
		for(int c = colStart+12; c < colStart+16; c++){
			mvaddch(r, c, ' ');
		}
	}
	attroff(COLOR_PAIR(3));
	
	attron(COLOR_PAIR(2));
	for(int r = rowStart+6; r < rowStart+10; r++){
		for(int c = colStart; c < colStart+20; c++){
			mvaddch(r, c, ' ');
		}
	}
	attroff(COLOR_PAIR(2));
	
	refresh();
	getch();
	endwin();
	return 0;
}
