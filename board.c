#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "board.h"

void initsize(){
	getmaxyx(stdscr, maxy, maxx);
	colStart = (maxx-20)>>1;
	rowStart = (maxy-10)>>1;
}

void resetBoard(){
	//The top 4 rows are player 2
	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 10; c++){
			BOARD[r][c].player = 2;
			BOARD[r][c].rank = -1;
			BOARD[r][c].known1 = 0;
			BOARD[r][c].known2 = 0;
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
			BOARD[r][c].rank = -1;
			BOARD[r][c].known1 = 0;
			BOARD[r][c].known2 = 0;
		}
	}
}

void drawBoard(int player){
	int printRow = rowStart;
	for(int row = player==1?0:9; (player==1 && row < 10) || (player==2 && row >= 0); player==1?row++:row--){
		int printCol = colStart;
		for(int col = player==1?0:9; (player==1 && col < 10) || (player==2 && col>=0); player==1?col++:col--){
			if(BOARD[row][col].player == 1){ //red for player 1
				attron(COLOR_PAIR(1));
			}
			else if(BOARD[row][col].player == 2){ //blue for player 2
				attron(COLOR_PAIR(2));
			}
			else if(!BOARD[row][col].rank){ //black for empty spaces
				attron(COLOR_PAIR(5));
			}
			else{ //cyan for everything else
				attron(COLOR_PAIR(3));
			}
			
			if((player==1 && !BOARD[row][col].known1) || (player==2 && !BOARD[row][col].known2)){ //draw spaces for unknown pieces
				mvaddch(printRow, printCol, ' ');
				mvaddch(printRow, 1 + printCol, ' ');
				printCol+=2;
				continue;
			}

			switch(BOARD[row][col].rank){
				case 0: //empty
					break;
				case 10: //spy
					mvaddch(printRow, printCol, 'S');
					break;
				case 11: //bomb
					mvaddch(printRow, printCol, 'B');
					break;
				case 12: //flag
					mvaddch(printRow, printCol, 'F');
					break;
				case 13: //lake
					break;
				default: //1 through 9
					mvaddch(printRow, printCol, BOARD[row][col].rank + 48);
					break;
			}
			
			//draw the second space
			mvaddch(printRow, 1 + printCol, ' ');
			printCol+=2;
		}
		printRow++;
	}
	
	refresh();
}

void drawPiece(int row, int col, int player){
	int printRow = rowStart + row; //always print in the spot asked 
	int printCol = colStart + (col<<1);

	if(player == 2){ //access is rotated for player 2
		row = 9-row;
		col = 9-col;
	}
	
	
	if(BOARD[row][col].player == 1){ //red for player 1
		attron(COLOR_PAIR(1));
	}
	else if(BOARD[row][col].player == 2){ //blue for player 2
		attron(COLOR_PAIR(2));
	}
	else if(!BOARD[row][col].rank){ //black for empty spaces
		attron(COLOR_PAIR(5));
	}
	else{ //cyan for everything else
		attron(COLOR_PAIR(3));
	}
	
	if((player==1 && !BOARD[row][col].known1) || (player==2 && !BOARD[row][col].known2)){ //draw spaces for unknown pieces
		mvaddch(printRow, printCol, ' ');
		mvaddch(printRow, 1 + printCol, ' ');
		return;
	}

	switch(BOARD[row][col].rank){
		case 0: //empty
		case 13: //lake
			break;
		case 10: //spy
			mvaddch(printRow, printCol, 'S');
			break;
		case 11: //bomb
			mvaddch(printRow, printCol, 'B');
			break;
		case 12: //flag
			mvaddch(printRow, printCol, 'F');
			break;
		default: //1 through 9
			mvaddch(printRow, printCol, BOARD[row][col].rank + 48);
			break;
	}
	
	//draw the second space
	mvaddch(printRow, 1 + printCol, ' ');
}

void selectPiece(int row, int col, int player){
	int printRow = rowStart + row; //always print in the spot asked 
	int printCol = colStart + (col<<1);

	if(player == 2){ //access is rotated for player 2
		row = 9-row;
		col = 9-col;
	}

	attron(COLOR_PAIR(4));
	
	if((player==1 && !BOARD[row][col].known1) || (player==2 && !BOARD[row][col].known2)){ //draw spaces for unknown pieces
		mvaddch(printRow, printCol, ' ');
		mvaddch(printRow, 1 + printCol, ' ');
		return;
	}

	switch(BOARD[row][col].rank){
		case 0: //empty
		case 13: //lake
			break;
		case 10: //spy
			mvaddch(printRow, printCol, 'S');
			break;
		case 11: //bomb
			mvaddch(printRow, printCol, 'B');
			break;
		case 12: //flag
			mvaddch(printRow, printCol, 'F');
			break;
		default: //1 through 9
			mvaddch(printRow, printCol, BOARD[row][col].rank + 48);
			break;
	}
	
	//draw the second space
	mvaddch(printRow, 1 + printCol, ' ');
}

void drawPiece2(int row, int col, int player){
	drawPiece(row-rowStart, (col-colStart)>>1, player);
}

void selectPiece2(int row, int col, int player){
	selectPiece(row-rowStart, (col-colStart)>>1, player);
}

void clearPiece(Piece* piece){
	piece->player = 0;
	piece->rank = 0;
	piece->known1 = 0;
	piece->known2 = 0;
}
