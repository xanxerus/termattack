#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "board.h"

void setupBoard(int);
int takeTurn(int);
void defaultSetup();

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
	if( maxx < 25 || maxy < 10 ){
		endwin();
		printf("Your terminal (%dx%d) is too small. Must be at least 25x10\n", maxx, maxy);
		return 2;
	}
	
	//~ resetBoard();
	//~ drawBoard(1);
	//~ setupBoard(1);
	//~ drawBoard(2);
	//~ setupBoard(2);
	defaultSetup();

	for(int player = 1;; player = player==1?2:1){
		drawBoard(player);
		takeTurn(player);
	}
	
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

void defaultSetup(){
	//The top 4 rows are player 2
	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 10; c++){
			BOARD[r][c].player = 2;
			BOARD[r][c].rank = 9;
			BOARD[r][c].known1 = 0;
			BOARD[r][c].known2 = 1;
		}
	}

	//The middle two rows are lake and empty spots
	for(int r = 4; r < 6; r++){
		for(int c = 0; c < 10; c++){
			if((2 <= c && c < 4) || (6 <= c && c < 8)){
				BOARD[r][c].rank = 13; //lake spaces 
			}
			else{
				BOARD[r][c].rank = 0; //free spaces
			}
			BOARD[r][c].player = 0;
			BOARD[r][c].known1 = 0;
			BOARD[r][c].known2 = 0;
		}
	}
	
	//The last 4 rows are player 1
	for(int r = 6; r < 10; r++){
		for(int c = 0; c < 10; c++){
			BOARD[r][c].player = 1;
			BOARD[r][c].rank = 9;
			BOARD[r][c].known1 = 1;
			BOARD[r][c].known2 = 0;
		}
	}
}

void setupBoard(int player){
	int printCol = colStart, printRow = rowStart+6;
	int distr[12] = {1, 1, 1, 2, 3, 4, 4, 4, 5, 8, 1, 6}; //flag, 1-9, spy, bomb
	drawDistr(distr);
	selectPiece2(printRow, printCol, player);

	for(;;){
		int ch = getch();
		if(ch == KEY_DOWN || ch == KEY_UP || ch == KEY_LEFT || ch == KEY_RIGHT){ //arrow keys
			drawPiece2(printRow, printCol, player);
			switch(ch){
				case KEY_DOWN:
					if(printRow-rowStart+1 < 10)
						printRow++;
					break;
				case KEY_UP:
					if(printRow-rowStart-1 >= 6)
						printRow--;
					break;
				case KEY_RIGHT:
					if(printCol-colStart+2 < 20)
						printCol+=2;
					break;
				case KEY_LEFT:
					if(printCol-colStart-1 >= 0)
						printCol-=2;
					break;
			}
			selectPiece2(printRow, printCol, player);
		}
		else if(('1' <= ch && ch <= '9') || ch == 'f' || ch == 'b' || ch == 's' || ch == ' '){
			int rank = ch;
			switch(ch){
				case ' ':
					rank = 0;
					break;
				case 's':
					rank = 10;
					break;
				case 'b':
					rank = 11;
					break;
				case 'f':
					rank = 12;
					break;
				default:
					rank = ch-'0';
					break;
			}
			
			int r = player==1? printRow-rowStart : 9+rowStart-printRow;
			int c = player==1? (printCol-colStart)>>1 : 9+((colStart-printCol)>>1);
			
			if(BOARD[r][c].rank >= 0)
				distr[BOARD[r][c].rank%12]++;
			
			if(rank == 0){
				BOARD[r][c].rank = -1;
				if(player==1)
					BOARD[r][c].known1 = 0;
				else if(player==2)
					BOARD[r][c].known2 = 0;

			}
			else if(distr[rank%12] > 0){
				BOARD[r][c].rank = rank;
				if(player==1)
					BOARD[r][c].known1 = 1;
				else if(player==2)
					BOARD[r][c].known2 = 1;
				distr[rank%12]--;
			}
			
			selectPiece2(printRow, printCol, player);
			drawDistr(distr);
		}
		else if(ch == KEY_ENTER || ch == '\n'){
			clearMsg();
			int pieces = 0;
			for(int x = 0; x < 12; x++){
				if(distr[x]){
					pieces = 1;
					break;
				}
			}
			
			if(pieces){
				mvaddstr(rowStart+11, colStart+2, "You are not done");
				getch();
			}
			else{
				mvaddstr(rowStart+11, colStart, "Are you done? (y/n)");
				ch = getch();
				if(ch == 'y' || ch == 'Y')
					return;
			}
			drawDistr(distr);
		}
		else{
			clearMsg();
			char s[21];
			sprintf(s, "%d", ch);
			mvaddstr(rowStart+11, colStart, s);
		}
	}
}


int takeTurn(int player){
	int printCol = colStart, printRow = rowStart+6;
	int selRow = -1, selCol = -1; //print coordinates, not board coordinates
	selectPiece2(printRow, printCol, player);

	for(;;){
		int ch = getch();
		if(ch == KEY_DOWN || ch == KEY_UP || ch == KEY_LEFT || ch == KEY_RIGHT){ //arrow keys
			if(printRow != selRow || printCol != selCol)
				drawPiece2(printRow, printCol, player);
			switch(ch){
				case KEY_DOWN:
					if(printRow-rowStart+1 < 10)
						printRow++;
					break;
				case KEY_UP:
					if(printRow-rowStart-1 >= 0)
						printRow--;
					break;
				case KEY_RIGHT:
					if(printCol-colStart+2 < 20)
						printCol+=2;
					break;
				case KEY_LEFT:
					if(printCol-colStart-1 >= 0)
						printCol-=2;
					break;
			}
			selectPiece2(printRow, printCol, player);
		}
		else if(' '){
			int r = player==1? printRow-rowStart : 9-(printRow-rowStart);
			int c = player==1? (printCol-colStart)>>1 : 9-((printCol-colStart)>>1);
			
			if(BOARD[r][c].player == player){ //we selected our own piece
				if(selRow >= 0){ //we already selected a piece. Deselect it.
					drawPiece2(selRow, selCol, player);
				}
				selRow = printRow;
				selCol = printCol;
				selectPiece2(selRow, selCol, player); //select our new piece
			}
			else if(BOARD[r][c].player == 0){ //we chose a non-player piece
				if(BOARD[r][c].rank == 0 && selRow >= 0){ //we chose an empty space and have a piece selected
					int sr = player==1 ? selRow-rowStart : 9 - (selRow-rowStart);
					int sc = player==1 ? (selCol-colStart)>>1 : 9 - ((selCol-colStart)>>1);
					
					
					BOARD[r][c] = BOARD[sr][sc]; //move our selected piece there
					clearPiece(&BOARD[sr][sc]); //empty the space we were at
					drawPiece2(selRow, selCol, player);
					drawPiece2(printRow, printCol, player);
					return 0;
				}
			}
			else{ //we chose an opponent piece
				
				//~ if(player==1) //reveal it
					//~ BOARD[r][c].known1 = 1;
				//~ else
					//~ BOARD[r][c].known2 = 1;
				
				//~ if(BOARD[r][c].rank == 12) //we won
					//~ return 1;
				//~ else if(BOARD[r][c].rank == 11){ //we hit a bomb
					//~ BOARD[selRow][selCol] = {0,0,0,0}; //kill us
					//~ BOARD[r][c] = {0,0,0,0}; //kill them
				//~ }
				//~ else if(BOARD[r][c]
			}
		}
		else if(ch == 'q'){
			return 0;
		}
		else{
			clearMsg();
			char s[21];
			sprintf(s, "%d", ch);
			mvaddstr(rowStart+11, colStart, s);
		}
	}
}
