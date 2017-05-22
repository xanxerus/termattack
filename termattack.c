#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "board.h"

int main(){
	initscr();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	
	if(has_colors() == FALSE){
		endwin();
		printf("Your terminal does not support color\n");
		return 1;
	}
	
	/*
	 * 1 is red bg
	 * 2 is blue bg
	 * 3 is cyan bg
	 * 4 is white bg
	 * 5 is black bg
	 */
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_BLACK, COLOR_CYAN);
	init_pair(4, COLOR_BLACK, COLOR_WHITE);
	init_pair(5, COLOR_WHITE, COLOR_BLACK);
	
	initsize();
	if( maxx < 20 || maxy < 10 ){
		endwin();
		printf("Your terminal (%dx%d) is too small. Must be at least 20x10\n", maxx, maxy);
		return 2;
	}
	
	resetBoard();
	
	drawBoard(1);
	setupBoard(1);
	drawBoard(2);
	setupBoard(2);
	
	getch();
	endwin();
	return 0;
}

void drawDistr(int* distr){
	char s[21];
	attron(COLOR_PAIR(5));

	sprintf(s, "F: %d B: %d S: %d 1: %d", distr[0], distr[11], distr[10], distr[1]);
	mvaddstr(rowStart+11, colStart, s);

	sprintf(s, "2: %d 3: %d 4: %d 5: %d", distr[2], distr[3], distr[4], distr[5]);
	mvaddstr(rowStart+12, colStart, s);

	sprintf(s, "6: %d 7: %d 8: %d 9: %d", distr[6], distr[7], distr[8], distr[9]);
	mvaddstr(rowStart+13, colStart, s);

}

void clearMsg(){
	attron(COLOR_PAIR(5));
	mvaddstr(rowStart+11, colStart, "                    ");
	mvaddstr(rowStart+12, colStart, "                    ");
	mvaddstr(rowStart+13, colStart, "                    ");
	mvaddstr(rowStart+14, colStart, "                    ");
}
