#ifndef BOARD_H
#define BOARD_H

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

//struct
typedef struct {
	int player;
	int rank;
	int known1;
	int known2;
} Piece;

//global variables (!)
Piece BOARD[10][10];
int maxx, maxy, colStart, rowStart;

//forward declarations
void initsize();
void resetBoard();

void drawBoard(int);
void drawPiece(int, int, int);
void selectPiece(int, int, int);

void drawPiece2(int, int, int);
void selectPiece2(int, int, int);

void setupBoard(int);

#endif