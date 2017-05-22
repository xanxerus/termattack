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
	for(int row = player==1?0:9; player==1 && row < 10 || player==2 && row >= 0; player==1?row++:row--){
		int printCol = colStart;
		for(int col = player==1?0:9; player==1 && col < 10 || player==2 && col>=0; player==1?col++:col--){
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
			
			if(player==1 && !BOARD[row][col].known1 || player==2 && !BOARD[row][col].known2){ //draw spaces for unknown pieces
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
					mvaddch(printRow, printCol, BOARD[row][col].rank + 47);
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
	
	if(player==1 && !BOARD[row][col].known1 || player==2 && !BOARD[row][col].known2){ //draw spaces for unknown pieces
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
	
	if(player==1 && !BOARD[row][col].known1 || player==2 && !BOARD[row][col].known2){ //draw spaces for unknown pieces
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
	drawPiece(row-rowStart, col-colStart>>1, player);
}

void selectPiece2(int row, int col, int player){
	selectPiece(row-rowStart, col-colStart>>1, player);
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
		else if('1' <= ch && ch <= '9' || ch == 'f' || ch == 'b' || ch == 's' || ch == ' '){
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
			int c = player==1? printCol-colStart>>1 : 9+((colStart-printCol)>>1);
			
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
			mvaddstr(rowStart+11, colStart, "Are you done? (y/n)");
			ch = getch();
			if(ch == 'y' || ch == 'Y')
				return;
			drawDistr(distr);
		}
		else{
			clearMsg();
			char* s[21];
			sprintf(s, "%d", ch);
			//~ sprintf(s, "%d", KEY_ENTER);
			mvaddstr(rowStart+11, colStart, s);
		}
	}
}
