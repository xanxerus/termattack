#include <stdio.h>
#include <string.h>
#include <ncurses.h>

void resetBoard();
void drawBoard();
void setupBoard();
void drawPiece();
void drawDistr();

/*
 *    Piece    Rank    Quantity
 *    Empty      0       12
 *      1        1        1
 *      2        2        1
 *      3        3        2
 *      4        4        3
 *      5        5        4
 *      6        6        4
 *      7        7        4
 *      8        8        5
 *      9        9        8
 *     Spy      10        1
 *     Bomb     11        6
 *     Flag     12        1
 *     Lake     13        8
 * 
 * A lake or empty tile has player 0.
 * Before setup, all tiles are rank -1.
 * 
 */

typedef struct {
	int player;
	int rank;
	int known;
} Piece;

Piece BOARD[10][10];
int maxx, maxy, colStart, rowStart;

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
	
	start_color();
	/*
	 * 1 is red bg
	 * 2 is blue bg
	 * 3 is cyan bg
	 * 4 is white bg
	 * 5 is black bg
	 */
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_BLACK, COLOR_CYAN);
	init_pair(4, COLOR_BLACK, COLOR_WHITE);
	init_pair(5, COLOR_WHITE, COLOR_BLACK);
	
	getmaxyx(stdscr, maxy, maxx);
	colStart = (maxx-20)>>1;
	rowStart = (maxy-10)>>1;

	if( maxx < 20 || maxy < 10 ){
		endwin();
		printf("Your terminal (%dx%d) is too small. Must be at least 20x10\n", maxx, maxy);
		return 2;
	}
	
	resetBoard();
	drawBoard();
	setupBoard();
	
	getch();
	endwin();
	return 0;
}

void resetBoard(){
	//The top 4 rows are player 2
	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 10; c++){
			BOARD[r][c].player = 2;
			BOARD[r][c].rank = -1;
			BOARD[r][c].known = 0;
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
			BOARD[r][c].known = 0;
		}
	}
	
	//The last 4 rows are player 1
	for(int r = 6; r < 10; r++){
		for(int c = 0; c < 10; c++){
			BOARD[r][c].player = 1;
			BOARD[r][c].rank = -1;
			BOARD[r][c].known = 0;
		}
	}
}

void drawBoard(){
	for(int row = 0; row < 10; row++){
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
			
			if(!BOARD[row][col].known){ //draw spaces for unknown pieces
				mvaddch(rowStart + row, colStart + (col<<1), ' ');
				mvaddch(rowStart + row, 1 + colStart + (col<<1), ' ');
				continue;
			}

			switch(BOARD[row][col].rank){
				case 0: //empty
					break;
				case 10: //spy
					mvaddch(rowStart + row, colStart + (col<<1), 'S');
					break;
				case 11: //bomb
					mvaddch(rowStart + row, colStart + (col<<1), 'B');
					break;
				case 12: //flag
					mvaddch(rowStart + row, colStart + (col<<1), 'F');
					break;
				case 13: //lake
					break;
				default: //1 through 9
					mvaddch(rowStart + row, colStart + (col<<1), BOARD[row][col].rank + 47);
					break;
			}
			
			//draw the second space
			mvaddch(rowStart + row, 1 + colStart + (col<<1), ' ');
		}
	}
	
	refresh();
}

void drawPiece(int row, int col){
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
	
	if(!BOARD[row][col].known){ //draw spaces for unknown pieces
		mvaddch(rowStart + row, colStart + (col<<1), ' ');
		mvaddch(rowStart + row, 1 + colStart + (col<<1), ' ');
		return;
	}

	switch(BOARD[row][col].rank){
		case 0: //empty
		case 13: //lake
			break;
		case 10: //spy
			mvaddch(rowStart + row, colStart + (col<<1), 'S');
			break;
		case 11: //bomb
			mvaddch(rowStart + row, colStart + (col<<1), 'B');
			break;
		case 12: //flag
			mvaddch(rowStart + row, colStart + (col<<1), 'F');
			break;
		default: //1 through 9
			mvaddch(rowStart + row, colStart + (col<<1), BOARD[row][col].rank + 48);
			break;
	}
	
	//draw the second space
	mvaddch(rowStart + row, 1 + colStart + (col<<1), ' ');
}

void selectPiece(int row, int col){
	attron(COLOR_PAIR(4));
	
	if(!BOARD[row][col].known){ //draw spaces for unknown pieces
		mvaddch(rowStart + row, colStart + (col<<1), ' ');
		mvaddch(rowStart + row, 1 + colStart + (col<<1), ' ');
		return;
	}

	switch(BOARD[row][col].rank){
		case 0: //empty
		case 13: //lake
			break;
		case 10: //spy
			mvaddch(rowStart + row, colStart + (col<<1), 'S');
			break;
		case 11: //bomb
			mvaddch(rowStart + row, colStart + (col<<1), 'B');
			break;
		case 12: //flag
			mvaddch(rowStart + row, colStart + (col<<1), 'F');
			break;
		default: //1 through 9
			mvaddch(rowStart + row, colStart + (col<<1), BOARD[row][col].rank + 48);
			break;
	}
	
	//draw the second space
	mvaddch(rowStart + row, 1 + colStart + (col<<1), ' ');
}

void drawPiece2(int row, int col){
	drawPiece(row-rowStart, col-colStart>>1);
}

void selectPiece2(int row, int col){
	selectPiece(row-rowStart, col-colStart>>1);
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

void setupBoard(){
	int c = colStart, r = rowStart+6;
	//flag, 1-9, spy, bomb
	int distr[12] = {1, 1, 1, 2, 3, 4, 4, 4, 5, 8, 1, 6};
	drawDistr(distr);

	attron(COLOR_PAIR(4));
	mvaddch(r, c, ' ');
	mvaddch(r, c+1, ' ');
	
	for(;;){
		int ch = getch();
		if(ch == KEY_DOWN || ch == KEY_UP || ch == KEY_LEFT || ch == KEY_RIGHT){ //arrow keys
			drawPiece2(r, c);
			switch(ch){
				case KEY_DOWN:
					if(r-rowStart+1 < 10)
						r++;
					break;
				case KEY_UP:
					if(r-rowStart-1 >= 6)
						r--;
					break;
				case KEY_RIGHT:
					if(c-colStart+2 < 20)
						c+=2;
					break;
				case KEY_LEFT:
					if(c-colStart-1 >= 0)
						c-=2;
					break;
			}
			selectPiece2(r, c);
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
			
			if(BOARD[r-rowStart][c-colStart>>1].rank >= 0)
				distr[BOARD[r-rowStart][c-colStart>>1].rank%12]++;
			
			if(rank == 0){
				BOARD[r-rowStart][c-colStart>>1].rank = -1;
				BOARD[r-rowStart][c-colStart>>1].known = 0;

			}
			else if(distr[rank%12] > 0){
				BOARD[r-rowStart][c-colStart>>1].rank = rank;
				BOARD[r-rowStart][c-colStart>>1].known = 1;
				distr[rank%12]--;
			}
			
			selectPiece2(r, c);
			drawDistr(distr);
		}
		

	}
}
