#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "board.h"

//for read
#include <unistd.h>

//for socket and send
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//for inet_pton
#include <arpa/inet.h>

void setupBoard(int);
Move takeTurn(int);
int makeMove(Move);
void defaultSetup();
void clearMsg();
void drawDistr(int* distr);
int offlineGame(int);
int onlineGame(int, int, int);
int startClient(int, char*);
int startServer(int);
int sendMove(int, Move);
int readMove(int);

int main(int argc, char** argv){
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
		printf("Your terminal (%dx%d) is too small. Must be at least 20x13\n", maxx, maxy);
		return 2;
	}
	
	int naiveFlag = 0, serverFlag = 0, clientFlag = 0, helpFlag = 0;
	char* IP;
	int PORT;
	
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "--naive") == 0){
			naiveFlag = 1;
		}
		else if(strcmp(argv[i], "--server") == 0){
			serverFlag = 1;
			
			if(i+1 >= argc || clientFlag){
				helpFlag = 1;
			}
			else{
				PORT = atoi(argv[i+1]);
			}
		}
		else if(strcmp(argv[i], "--client") == 0){
			clientFlag = 1;
			
			if(i+2 >= argc || serverFlag){
				helpFlag = 1;
			}
			else{
				IP = argv[i+1];
				PORT = atoi(argv[i+2]);
			}
		}
		else if(strcmp(argv[i], "--help") == 0)
			helpFlag = 1;
	}
	
	int winner = -1;
	if(helpFlag){
		endwin();
		printf("Usage:\n\ttermattack [--naive] --server PORT\n\tor\n\ttermattack [--naive] [--client IP PORT]\n");
		return 0;
	}
	else if(serverFlag || clientFlag){
		int sock = serverFlag ? startServer(PORT) : startClient(PORT, IP);
		
		if(sock < 0){
			endwin();
			printf("Something TCP related failed.\n");
			return -1;
		}
		
		winner = onlineGame(naiveFlag, sock, !serverFlag+1);
		
	}
	else{
		winner = offlineGame(naiveFlag);
	}
	
	if(winner < 0){
		clear();
		mvaddstr(rowStart+11, colStart+7, "Connection lost!");
		getch();
		return -1;
	}
	else{
		clear();
		drawRevelation();
		attron(COLOR_PAIR(5));
		mvaddstr(rowStart+11, colStart+3, "Player ");
		mvaddch(rowStart+11, colStart+10, winner + 48);
		mvaddstr(rowStart+11, colStart+11, " wins!");
		refresh();
		getch();
	}
	endwin();
	return 0;
}

int startServer(int port){
	//Make socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket < 0){
		return -1;
	}
	
	//Bind to port
	struct sockaddr_in address;
	address.sin_family = AF_INET; //IPv4
	address.sin_addr.s_addr = INADDR_ANY; //no idea what this does
	address.sin_port = htons(port); //set port
	
	if(bind(server_socket, (struct sockaddr*) &address, sizeof(address))){
		return -1;
	}
	
	//listen for clients
	if(listen(server_socket, 1)){
		return -1;
	}
	
	mvaddstr(rowStart+12, colStart, "Waiting on client...");
	refresh();
	int addrlen = sizeof(address); //manpage says I need this
	
	return accept(server_socket, (struct sockaddr*) &address, (socklen_t*) &addrlen);
}

int startClient(int port, char* ip){
	//Make socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket < 0){
		return -1;
	}

	//Set IP address for the server 
	struct sockaddr_in address;
	memset(&address, '0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	if(inet_pton(AF_INET, ip, &address.sin_addr)<=0){
		return -1;
	}

	//try to connect to server
	if (connect(client_socket, (struct sockaddr *)&address, sizeof(address)) < 0){
		return -1;
	}
	return client_socket;
}

/**
 * Play an online game. If naive is true, default setup is used.
 * Returns the winning player's number.
 */
int onlineGame(int naive, int socket, int truePlayer){
	resetBoard();
	
	if(naive){
		defaultSetup();
	}
	else{
		drawBoard(1);
		setupBoard(1);
	}
	
	int turn = 0;
	for(int player = 1; ; player = player==1?2:1){
		//print the board for truePlayer
		clear();
		attron(COLOR_PAIR(5));
		drawBoard(truePlayer);
		
		if(player == truePlayer){
			if(turn < 0){ //readMove failed
				return -1;
			}

			//print what was just attacked
			if(turn > 0){
				attron(COLOR_PAIR(5));
				mvaddstr(rowStart+11, colStart+3, "Opponent hit ");
				mvaddch(rowStart+11, colStart+16, decodeRank(turn));
			}
			if(turn == 12)
				return player;
			
			refresh();
			Move move = takeTurn(player);
			turn = sendMove(socket, move);
			
			if(turn < 0){ //sendMove failed
				return -1;
			}
			
			drawPiece(move.sr, move.sc, player);
			drawPiece(move.dr, move.dc, player);
			
			if(turn == 12){
				return player;
			}
			else{
				drawBoard(player);
				if(turn){
					attron(COLOR_PAIR(5));
					mvaddstr(rowStart+11, colStart+5, "Attacked ");
					mvaddch(rowStart+11, colStart+14, decodeRank(turn));
					getch();
				}
			}
		}
		else{
			mvaddstr(rowStart+11, colStart+5, "Waiting on opponent.");
			refresh();
			turn = readMove(socket);
			if(turn == 12)
				return player;
		}
	}
	
	return 0;
}

/**
 * Sends a move and the rank of the source tile via TCP socket.
 * Recieves a rank from the other player and updates the board.
 * Finally, performs the given move.
 * 
 * Returns the rank of the piece overtaken, or -1 if the socket failed.
 */
int sendMove(int socket, Move move){
	if(send(socket, &move, sizeof(Move), 0) < 0)
		return -1;
	if(send(socket, &BOARD[move.sr][move.sc], sizeof(int), 0) < 0)
		return -1;
	if(read(socket, &BOARD[move.dr][move.dc].rank, sizeof(int)) < 0)
		return -1;
	
	//perform the move
	return makeMove(move);
}

/**
 * Reads a move and a rank via TCP packet and updates the board.
 * Sends a rank to the other player.
 * Finally, performs the given move.
 * 
 * Returns the rank of the piece overtaken, or -1 if the socket failed.
 */
int readMove(int socket){
	//read a move and rank
	Move move;
	int rank;
	
	if(read(socket, &move, sizeof(Move)) < 0)
		return -1;
	if(read(socket, &rank, sizeof(int)) < 0)
		return -1;

	//update the board
	BOARD[move.sr][move.sc].rank = rank;
	
	//send a rank to the other player
	if(send(socket, &BOARD[move.dr][move.dc], sizeof(int), 0) < 0)
		return -1;
	
	//perform the move
	return makeMove(move);
}

/**
 * Play an offline game. If naive is true, default setup is used.
 * Returns the winning player's number.
 */
int offlineGame(int naive){
	resetBoard();
	
	if(naive){
		defaultSetup();
	}
	else{
		drawBoard(1);
		setupBoard(1);
		drawBoard(2);
		setupBoard(2);
	}

	int turn = 0;
	for(int player = 1; ; player = player==1?2:1){
		clear();
		attron(COLOR_PAIR(5));
		mvaddstr(rowStart+5, colStart+6, "Player ");
		mvaddch(rowStart+5, colStart+13, player + 48);
		getch();

		
		mvaddstr(rowStart+5, colStart+6, "        ");
		drawBoard(player);
		
		if(turn > 0){
			attron(COLOR_PAIR(5));
			mvaddstr(rowStart+11, colStart+5, "Attacked ");
			mvaddch(rowStart+11, colStart+14, decodeRank(turn));
		}

		Move move = takeTurn(player);
		turn = makeMove(move);
		drawPiece(move.sr, move.sc, player);
		drawPiece(move.dr, move.dc, player);

		if(turn == 12){
			return player;
		}
		else{
			drawBoard(player);
			if(turn){
				attron(COLOR_PAIR(5));
				mvaddstr(rowStart+11, colStart+5, "Attacked ");
				mvaddch(rowStart+11, colStart+14, decodeRank(turn));
				getch();
			}
		}
	}
}

/**
 * Draw the number of remaining pieces of each type to the message
 * portion of the screen.
 */
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

/**
 * Erase the message portion of the screen.
 */
void clearMsg(){
	attron(COLOR_PAIR(5));
	mvaddstr(rowStart+11, colStart, "                    ");
	mvaddstr(rowStart+12, colStart, "                    ");
	mvaddstr(rowStart+13, colStart, "                    ");
}

/**
 * Populate the board with a naive arrangement of pieces.
 * Only used in debugging.
 */
void defaultSetup(){
	//The top 4 rows are player 2
	int distr[12] = {1, 1, 1, 2, 3, 4, 4, 4, 5, 8, 1, 6}; //flag, 1-9, spy, bomb
	int i = 0;
	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 10; c++){
			BOARD[r][c].player = 2;
			BOARD[r][c].rank = i==0?12:i;
			BOARD[r][c].known1 = 0;
			BOARD[r][c].known2 = 1;
			
			distr[i]--;
			if(distr[i] == 0)
				i++;
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
	int distr2[12] = {1, 1, 1, 2, 3, 4, 4, 4, 5, 8, 1, 6}; //flag, 1-9, spy, bomb
	i = 0;
	for(int r = 6; r < 10; r++){
		for(int c = 0; c < 10; c++){
			BOARD[r][c].player = 1;
			BOARD[r][c].rank = i==0?12:i;
			BOARD[r][c].known1 = 1;
			BOARD[r][c].known2 = 0;
			
			distr2[i]--;
			if(distr2[i] == 0)
				i++;
		}
	}

}

/**
 * Allow a given player to set up the board.
 */
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
			
			if(BOARD[r][c].rank >= 0 && (rank == 0 || distr[rank] > 0))
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
				mvaddstr(rowStart+11, colStart+2, "                ");
			}
			else{
				mvaddstr(rowStart+11, colStart, "Are you done? (y/n)");
				ch = getch();
				if(ch == 'y' || ch == 'Y'){
					mvaddstr(rowStart+11, colStart, "                   ");
					return;
				}
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

/**
 * Allow a given player to move a piece.
 * Returns the Move selected by the player.
 */
Move takeTurn(int player){
	int printCol = colStart, printRow = rowStart+6;
	int selRow = -1, selCol = -1; //print coordinates, not board coordinates
	selectPiece2(printRow, printCol, player);
	refresh();

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
		else if(ch == ' '){
			//these are board coordinates
			int r = player==1? printRow-rowStart : 9-(printRow-rowStart);
			int c = player==1? (printCol-colStart)>>1 : 9-((printCol-colStart)>>1);
			int sr = selRow == -1 ? -1 : player==1 ? selRow-rowStart : 9 - (selRow-rowStart);
			int sc = selCol == -1 ? -1 : player==1 ? (selCol-colStart)>>1 : 9 - ((selCol-colStart)>>1);
			
			if(BOARD[r][c].player == player){ //we selected our own piece
				if(1 <= BOARD[r][c].rank && BOARD[r][c].rank <= 10){ //check that it is movable
					if(selRow >= 0){ //we already selected a piece. Deselect it.
						drawPiece2(selRow, selCol, player);
					}
					selRow = printRow; //select our new piece
					selCol = printCol;
					selectPiece2(selRow, selCol, player);
				}
			}
			else if(selRow >= 0){ //we chose a non-player piece and have a piece selected
				if(checkValidity(sr, sc, r, c)){
					Move move;
					move.sr = sr;
					move.sc = sc;
					move.dr = r;
					move.dc = c;
					return move;
				}
				else{ //we are trying to violate movement permission.
					attron(COLOR_PAIR(5));
					mvaddstr(rowStart+11, colStart+3, "Invalid move");
					getch();
					mvaddstr(rowStart+11, colStart+3, "            ");
				}
			}
		}
	}
}

/**
 * Performs a given move. Assumes move is valid and source and
 * destination ranks are known. Does not draw to screen.
 */
int makeMove(Move move){
	int us = BOARD[move.sr][move.sc].rank;
	int them = BOARD[move.dr][move.dc].rank;
	
	if(them == 12){ //we win the game
		BOARD[move.dr][move.dc].known1 = 1; //reveal to all
		BOARD[move.dr][move.dc].known2 = 1;
	}
	else if(us == them || //it's a draw
		   (them == 11 && us != 8)){ //or they're a bomb and we're not a miner
		clearPiece(&BOARD[move.sr][move.sc]);
		clearPiece(&BOARD[move.dr][move.dc]);
	}
	else if(us < them || //we win the fight
		   (them == 11 && us == 8) || //or they're a bomb and we're a miner
		   (them == 1 && us == 10) || //or they're a 1 and we're a spy
		   (them == 0)){ //or they're empty space
		BOARD[move.dr][move.dc] = BOARD[move.sr][move.sc]; //we take over
		clearPiece(&BOARD[move.sr][move.sc]); //clear the old spot
	}
	else if(us > them){ //we lose the fight
		clearPiece(&BOARD[move.sr][move.sc]); //we die
		BOARD[move.dr][move.dc].known1 = 1; //reveal to all
		BOARD[move.dr][move.dc].known2 = 1;
	}
	
	return them;
}
