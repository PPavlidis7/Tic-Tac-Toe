#include <stdlib.h>
#include <stdio.h>
#include <cnaiapi.h>
#include <time.h>
#include <string.h>

#define BUFFSIZE		256

int recvln(connection, char *, int);
int readln(char *, int);
void printboard(char board[3][3]);  // Print the board
int isWinner(char board[3][3], int player);		// Look for winner

/*-----------------------------------------------------------------------
*
* Program: TrilizaC
* Purpose: contact a server and allow users to play the Tic - tac toe game online
* Usage:   TrilizaC
*
*-----------------------------------------------------------------------
*/
int
main(int argc, char *argv[])
{
	computer	comp;
	connection	conn;
	char buff[BUFFSIZE];
	int	len;

	int i = 0;                                   // Loop counter                       
	int player = 0;                              // Player number - 1 or 2               
	int go = 0;                                  // Square selection number     
	int row = 0;								 // Row index for a square               
	int column = 0;                              // Column index for a square            
	int line = 0;                                // Row or column index in checking loop 
	int winner = 0;								 // The winning player                   
	char board[3][3];							 // The board                            
												 // Initial values are reference numbers 
												 // used to select a vacant square for   
												 // a turn.                              
	int playerID, opponentID;					// playerID =1 and opponentID=2;
	char IP[32];								// A table where I save the IP
	int port;									// The port number which client use to connect with server
	FILE *fp;									// A variable to read from file server's IP and port's number
												// my file has IP=127.0.0.1 and port number=20000
	time_t start;								// The second that the client see the message in console
												// which tells him to put X or O
	time_t stop;								// The second that client pressed enter
	double diff;								// The difference between star and stop 
	boolean flag = TRUE;						// A flag that will become FALSE if client press a number out of time
	

	fp = fopen("triliza.conf", "r");
	if (fp == NULL) {
		fprintf(stderr, "\nFailed to open config file %s", "triliza.conf");
		exit(1);
	}
	else {
		fscanf(fp, "%s%d", IP, &port);
	}

	// Convert the compname to binary form comp 
	comp = cname_to_comp(IP);
	if (comp == -1)
		exit(1);

	// Make a connection to the chatserver 
	conn = make_contact(comp, port);
	if (conn < 0)
		exit(1);

	// Wait for the other player and update the variables player & opponent with the correct values
	(void)recv(conn, buff, BUFFSIZE, 0);
	playerID = atoi(buff);
	opponentID = (playerID == 1) ? 2 : 1;
	if (playerID == 1)
		printf("\nPlease Wait For the Second Player.\n");

	// Connection with TrilizaS server established
	(void)recv(conn, buff, BUFFSIZE, 0);
	printf("%s\n", buff);
	
	// Board at the beginning 
	(void)recv(conn, buff, BUFFSIZE, 0);
	strcpy(board, buff);
	printboard(board);

	for (i = 0; i < 9 && winner == 0; i++)
	{
		// If i is bigger or equal to five then we may have a winner so I have to ckeck the board */
		if (i >= 5)
		winner = isWinner(board, player);
		
		player = i % 2 + 1; // Select player

		// if winnes is found , "for" will break
		if (winner != 0)
			break;

		if (player == playerID)
		{
			printf("\nPlease enter the number of the square "
				"where you want to place your %c: ", (player == 1) ? 'X' : 'O');
			time(&start);		// The sec I start the time counter
			scanf("%d", &go);
			time(&stop);		// The sec I stop the time counter
			diff = difftime(stop, start);
			// If diff> 20 sec then player is OUT OF TIME
			if (diff <= 20) { 
				row = --go / 3; // Get row index of square
				column = go % 3; // Get column index of square 
			}
			else {
				printf("\nOUT OF TIME\n");
				sprintf(buff, "OUT OF TIME");
				send(conn, buff, BUFFSIZE, 0);  // Tell the server that client is OUT OF TIME
				send_eof(conn);					// Tlose the connection
				flag = FALSE;					// Make the flag FALSE so my program can take appropriate actions 
				i = 10;							// Make i=10 to stop for-loop
			}
			// if client selected in time
			if (flag)
			{
				sprintf(buff, "%d", ++go);
				send(conn, buff, BUFFSIZE, 0); // Send your selection 
			}
		}
		else
		{
			printf("\nWaiting for opponent...\n");
			len = recv(conn, buff, BUFFSIZE, 0);	 // Receive peer's selection 
			// If opponent answered out of time
			if (strcmp(buff, "OUT OF TIME") == 0)	
			{
				printf("\nOpponet left the game . YOU ARE THE WINNER!\n");
				(void)send_eof(conn);
				i = 10;
				flag = FALSE;
			}
			else  // Print opponent's choice
			{
				go = atoi(buff);
				if (go == 0)
					printf("\nOpponent made wrong choice\n");
				else
					printf("\nOpponent chose %d\n", go);
			}
		}
		if (flag) // If player answered in time
		{
			(void)recv(conn, buff, BUFFSIZE, 0); // Receive a message from server
			if (strcmp(buff, "First fault") == 0)	// If first player make a wrong choice
			{
				i = i - 1;
				(void)recv(conn, buff, BUFFSIZE, 0);
				strcpy(board, buff);
				if (player == playerID)
					printf("\nYou made a wrong choice!!!");
			}
			else if (strcmp(buff, "Second fault") == 0)  // If second player make a wrong choice
			{
				i = i - 2;
				(void)recv(conn, buff, BUFFSIZE, 0);
				strcpy(board, buff);
				if (player == playerID)
					printf("You made a wrong choice!!!");
			}
			else                 // If no one make a wrong choice 
				strcpy(board, buff);

			printboard(board); // Print board
		}

		if (i == 8) // If players played 9 rounds we may have a winner
		{
			winner = isWinner(board, player);
		}

	}

	if (flag)  // If players answered in time every time the have to 
	{
		// Display result message 
		if (winner == 0)
		{
			printf("\nHow boring, it is a draw.\n");
			(void)send_eof(conn);
		}
		else if (winner == playerID)
		{
			printf("\nCongratulations first, YOU ARE THE WINNER!\n");
			(void)send_eof(conn);
		}
		else {
			printf("\nopponent wins this round...\n");
			(void)send_eof(conn);
		}

		(void)send_eof(conn);
	}
	return 0;
}

void printboard(char board[3][3])
{
	printf("\n\n");
	printf(" %c | %c | %c\n", board[0][0], board[0][1], board[0][2]);
	printf("---+---+---\n");
	printf(" %c | %c | %c\n", board[1][0], board[1][1], board[1][2]);
	printf("---+---+---\n");
	printf(" %c | %c | %c\n", board[2][0], board[2][1], board[2][2]);
}

int isWinner(char board[3][3], int player)
{
	int winner = 0;
	int line;

	// Check for a winning line - diagonals first 
	if ((board[0][0] == board[1][1] && board[0][0] == board[2][2]) ||
		(board[0][2] == board[1][1] && board[0][2] == board[2][0]))
		winner = player;
	else
		// Check rows and columns for a winning line 
		for (line = 0; line <= 2; line++)
		{
			if ((board[line][0] == board[line][1] && board[line][0] == board[line][2]) ||
				(board[0][line] == board[1][line] && board[0][line] == board[2][line]))
				winner = player;
		}
	return winner;
}