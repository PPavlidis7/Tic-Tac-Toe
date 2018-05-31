
#include <stdlib.h>
#include <stdio.h>
#include <cnaiapi.h>

#define BUFFSIZE		256


int recvln(connection, char *, int);
int readln(char *, int);
void printboard(char board[3][3]);		// Pprint the board

/*-----------------------------------------------------------------------
*
* Program: TrillizaS
* Purpose: wait for a connection from a TrilizaC & allow users to play the Tic - tac toe game online
* Usage:   TrilizaS <appnum>
*
*-----------------------------------------------------------------------
*/
int
main(int argc, char *argv[])
{
	connection	conn1, conn2;
	int	len;
	char buff[BUFFSIZE];
	char board[3][3] = {                 // The board                            
		{ '1','2','3' },				 // Initial values are reference numbers 
		{ '4','5','6' },				 // used to select a vacant square for   
		{ '7','8','9' }					 // a turn.                             
	};
	int playerfirst = 1;				// First player's ID
	int playersecond = 2;				// Second player's ID
	int row;							// Row index for a square 
	int column;							// Column index for a square 
	int choice;							// Square selection number
	int i = 0;							// Loop counter
	boolean flag = TRUE;				// A flag that will become FALSE if client press a number out of time
	char lastboard[3][3];				// Board's last condition. This table change every time both players take a
										// valid choice 

	if (argc != 2)
	{
		(void)fprintf(stderr, "usage: %s <appnum>\n", argv[0]);
		exit(1);
	}

	(void)printf("\nTriliza Server Waiting For First Player.\n");

	// Wait for a connection from first TrilizSclient
	conn1 = await_contact((appnum)atoi(argv[1]));
	if (conn1 < 0)
		exit(1);

	// Send the number of player1 to  first client 
	sprintf(buff, "%d", playerfirst);
	(void)send(conn1, buff, BUFFSIZE, 0);

	// Wait for a connection from second client
	conn2 = await_contact((appnum)atoi(argv[1]));
	if (conn2 < 0)
		exit(1);

	// Send the number of player2 to second client
	sprintf(buff, "%d", playersecond);
	(void)send(conn2, buff, BUFFSIZE, 0);

	// Tell the clients that they have a connection with the server
	sprintf(buff, "Connection with TrilizaS Server Established.\n");
	(void)send(conn1, buff, BUFFSIZE, 0);
	(void)send(conn2, buff, BUFFSIZE, 0);

	// Send to clients the Board at the beginning
	strcpy(buff, board);
	strcpy(lastboard, board);		//keep board's last condition
	(void)send(conn1, buff, BUFFSIZE, 0);
	(void)send(conn2, buff, BUFFSIZE, 0);

	// Iteration ends after i become bigger than 9 because board have 9 squares
	while (i < 9)
	{
		// Iterate, reading from the first client
		if (len = recv(conn1, buff, BUFFSIZE, 0) > 0) 
		{
			if (strcmp(buff, "OUT OF TIME") == 0) // If first client answered OUT OF TIME
			{
				(void)send(conn2, buff, BUFFSIZE, 0);	// Send to second client that his opponent answered out of time
				send_eof(conn1);				// Close his connection with server
				send_eof(conn2);				// Close second's player connection with server
				i = 10;							// Make i=10 to stop while-loop
				flag = FALSE;
				printf("\nFirst player answered OUT OF TIME");
			}
			else								// If first client answered IN OF TIME
			{
				choice = atoi(buff);			// Get players choice
				row = --choice / 3;				// Get row index of square 
				column = choice % 3;			// Get column index of square
				// If player made a valid choice
				if (choice >= 0 && choice < 9 && board[row][column] <= '9') 
				{
					// Send players choice to opponent
					(void)send(conn2, buff, BUFFSIZE, 0); 
					printf("\nFirst player chose %d\n", choice + 1);
					board[row][column] = 'X';	// Insert player symbol 
					strcpy(buff, board);
					(void)send(conn1, buff, BUFFSIZE, 0);
					(void)send(conn2, buff, BUFFSIZE, 0);
					printboard(board);
				}
				else							// If player didn't make a valid choice
				{
					sprintf(buff, "0");
					(void)send(conn2, buff, BUFFSIZE, 0);
					sprintf(buff, "First fault");
					(void)send(conn2, buff, BUFFSIZE, 0);
					(void)send(conn1, buff, BUFFSIZE, 0); 
					strcpy(board, lastboard);
					strcpy(buff, lastboard);
					(void)send(conn2, buff, BUFFSIZE, 0);
					(void)send(conn1, buff, BUFFSIZE, 0);
					continue;
				}
			}
		}
		// If player answered OUT OF TIME 
		if (!flag)
			continue;

		// Iterate, reading from the second client
		if (len = recv(conn2, buff, BUFFSIZE, 0) > 0)		
		{
			if (strcmp(buff, "OUT OF TIME") == 0)		// If second client answered OUT OF TIME
			{
				(void)send(conn1, buff, BUFFSIZE, 0);	// Send to first client that his opponent answered OUT OF TIME
				send_eof(conn1);						// Close first's player connection with server
				send_eof(conn2);						// Close his connection with server
				i = 10;									// Make i=10 to stop while-loop
				flag = FALSE;
				printf("\nSecond player answered OUT OF TIME\n");
			}
			else                                        // If second client answered IN OF TIME
			{
				choice = atoi(buff);					// Get players choice
				row = --choice / 3;						// Get row index of square 
				column = choice % 3;					// Get column index of square 
				// If player made a valid choice
				if (choice >= 0 && choice < 9 && board[row][column] <= '9') 
				{
					// Send players choice to opponent
					(void)send(conn1, buff, BUFFSIZE, 0);
					printf("\nSecond player chose %d\n", choice + 1);
					board[row][column] = 'O';			// Insert player symbol 
					strcpy(buff, board);
					(void)send(conn1, buff, BUFFSIZE, 0);
					(void)send(conn2, buff, BUFFSIZE, 0);
					printboard(board);
					strcpy(lastboard, board);
				}
				else                                    // If player didn't make a valid choice
				{
					sprintf(buff, "0");
					(void)send(conn1, buff, BUFFSIZE, 0);
					sprintf(buff, "Second fault");
					(void)send(conn1, buff, BUFFSIZE, 0);
					(void)send(conn2, buff, BUFFSIZE, 0);
					i = i - 2;							// i = i-2 because if second player make a wrong choice 
														// First player have to play again . If i don't change ,
														// The game may end before we have draw or a winner
					strcpy(board, lastboard);
					strcpy(buff, lastboard);
					(void)send(conn1, buff, BUFFSIZE, 0);
					(void)send(conn2, buff, BUFFSIZE, 0);
				}
			}
		}
		i = i + 2;
	}

	// Game is over 
	(void)printf("\nTrilizS Connection Closed.\n\n");
	// If no one made a choice OUT OF TIME flag is TRUE .That's mean that i>10 and server has to close his connections
	//with his clients
	if (flag) 
	{
		send_eof(conn1);
		send_eof(conn2);
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
