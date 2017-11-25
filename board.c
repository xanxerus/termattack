#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "board.h"

/**
 * Check if a the piece at BOARD[sr][sc] can move to BOARD[r][c].
 */
int checkValidity(int sr, int sc, int r, int c){
	if(BOARD[r][c].rank == 13) //we tried to jump in a lake
		return 0;
	
	if(BOARD[sr][sc].rank == 9){ //we picked a 9
		if(r == sr){ //our move is on the row
			if(c == sc){ //we didn't move. Invalid
				return 0;
			}
			else{
				for(int pCol = c<sc?c+1:sc+1; (c<sc && pCol < sc) || (sc<c && pCol < c); pCol++){
					if(BOARD[r][pCol].rank != 0){ //there is a piece in our way. Invalid
						return 0;
					}
				}
			}
		}
		else if(c == sc){ //our move is on the column
			for(int pRow = r<sr?r+1:sr+1; (r<sr && pRow < sr) || (sr<r && pRow < r); pRow++){
				if(BOARD[pRow][c].rank != 0){ //there is a piece in our way. Invalid
					return 0;
				}
			}
		}
		else{ //our move did not maintain a row or a column
			return 0;
		}
		return 1;
	}
	else{ //we picked not a 9
		if((abs(r-sr) == 1 && c==sc) || (abs(c-sc) == 1 && r==sr))
			return 1;
		return 0;
	}
}

/**
 * Return a char denoting a given rank.
 */
char decodeRank(int rank){
	switch(rank){
		case 0: //empty
		case 13: //lake
			return ' ';
		case 10: //spy
			return 'S';
		case 11: //bomb
			return 'B';
		case 12: //flag
			return 'F';
		default: //1 through 9
			return rank + 48;
	}
}

/**
 * Initialize global variables pertaining to window size
 */
void initsize(){
	getmaxyx(stdscr, maxy, maxx);
	colStart = (maxx-20)>>1;
	rowStart = (maxy-10)>>1;
}

/**
 * Set all board pieces to default values. Namely:
 * 
 * The top four rows are player 2, -1 rank, unknown to both players. 
 * The middle two rows are either lake or empty.
 * The bottom four rows are player 1, -1 rank, unknown to both players.
 */
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

/**
 * Print the board sideways with all pieces revealed.
 */
void drawRevelation(){
	attron(COLOR_PAIR(5));
	int printCol = colStart;
	for(int row = 0; row < 10; row++){
		int printRow = rowStart+9;
		for(int col = 0; col < 10; col++){
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
			
			mvaddch(printRow, printCol, decodeRank(BOARD[row][col].rank));
			mvaddch(printRow, printCol + 1, ' ');
			printRow--;
		}
		printCol+=2;
	}
	
	refresh();
}

/**
 * Print the board from the player's perspective
 */
void drawBoard(int player){
	attron(COLOR_PAIR(5));
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

			mvaddch(printRow, printCol, decodeRank(BOARD[row][col].rank));
			mvaddch(printRow, printCol + 1, ' ');
			printCol+=2;
		}
		printRow++;
	}
	
	refresh();
}

/**
 * Given a row, col, and player, print the piece at BOARD[row][col]
 * from the player's perspective. 
 * 
 * Here, row and col are relative to the board.
 */
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

	mvaddch(printRow, printCol, decodeRank(BOARD[row][col].rank));
	mvaddch(printRow, printCol + 1, ' ');
}

/**
 * Given a row, col, and player, highlight the piece at BOARD[row][col]
 * from the player's perspective. 
 * 
 * Here, row and col are relative to the board.
 */
void selectPiece(int row, int col, int player){
	attron(COLOR_PAIR(4));
	int printRow = rowStart + row; //always print in the spot asked 
	int printCol = colStart + (col<<1);

	if(player == 2){ //access is rotated for player 2
		row = 9-row;
		col = 9-col;
	}
	
	if((player==1 && !BOARD[row][col].known1) || (player==2 && !BOARD[row][col].known2)){ //draw spaces for unknown pieces
		mvaddch(printRow, printCol, ' ');
		mvaddch(printRow, 1 + printCol, ' ');
		return;
	}

	mvaddch(printRow, printCol, decodeRank(BOARD[row][col].rank));
	mvaddch(printRow, printCol + 1, ' ');
}

/**
 * Given a row, col, and player, print the piece at BOARD[row][col]
 * from the player's perspective.
 * 
 * Here, row and col are relative to the graphics, not the BOARD,
 * so row becomes row - rowStart and col becomes (col - colStart) / 2.
 */
void drawPiece2(int row, int col, int player){
	drawPiece(row-rowStart, (col-colStart)>>1, player);
}

/**
 * Given a row, col, and player, highlight the piece at BOARD[row][col]
 * from the player's perspective.
 * 
 * Here, row and col are relative to the graphics, not the BOARD,
 * so row becomes row - rowStart and col becomes (col - colStart) / 2.
 */
void selectPiece2(int row, int col, int player){
	selectPiece(row-rowStart, (col-colStart)>>1, player);
}

/**
 * Zero out the values of a piece. Could also do memset, but eh.
 */
void clearPiece(Piece* piece){
	piece->player = 0;
	piece->rank = 0;
	piece->known1 = 0;
	piece->known2 = 0;
}
