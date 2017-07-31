#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>



void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{

	// Initialize variables
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	size_t bufferLength = 1024;
	size_t handleLength = 256;
	char *buffer = calloc(bufferLength, sizeof(char));
	char *message = calloc(bufferLength, sizeof(char));
	char *handle = calloc(handleLength, sizeof(char));
	int charsTotal, charsTrans;

	// Check number of arguments  
	if (argc < 3)	{ fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(0); }
	
	// Prompt client for handle
	printf("Please enter user handle: ");
	getline(&handle, &handleLength, stdin);
	char *newLine = strchr(handle, '\n');		// Replace new line character with >
	if (newLine)	*newLine = '>';	
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to address
		fprintf(stderr, "CLIENT: ERROR connecting to port %d\n", portNumber);	
		close(socketFD); // Close the socket
		exit(2);
	}

	printf("Connected to server.\n");

	// Declare variables for ncurses
	WINDOW *input, *output;

	initscr();
	cbreak();
	echo();

	input = newwin(1, COLS, LINES - 1 ,0);		// User inputs at bottom of screen
	output = newwin(LINES - 1, COLS, 0, 0);		// Output displayed above
	wmove(output, LINES - 2, 0);
	scrollok(output, TRUE);

	fd_set readfds;		// Declare read file descriptor set

	// Enter infinite loop
	while (1) {
		memset(buffer, '\0', bufferLength);		// Clear out the buffers
		memset(message, '\0', bufferLength);
		charsTotal = 0;							// Reset variables
		charsTrans = 0;

		FD_ZERO(&readfds);						// Clear out fd set
		FD_SET(socketFD, &readfds);				// Add socket FD
		FD_SET(STDIN_FILENO, &readfds);			// Add stdin 
	
		mvwprintw(input, 0, 0, handle);			// Print handle to screen
		wrefresh(input);	
	
		if(select(socketFD+1, &readfds, NULL, NULL, NULL) < 0) {
			error("Error when calling select function.");
		}

		// Check socket file descriptor
		if (FD_ISSET(socketFD, &readfds)) {
			// Read until terminal character received
			while (strstr(buffer, "&&") == NULL) {	
				charsTrans = recv(socketFD, message, sizeof(message) - 1, 0);
				if (charsTrans < 0) error("CLIENT: ERROR reading from socket");
				// Connection closed by other side
				if (charsTrans == 0) {
					close(socketFD);
					endwin();
					printf("Connection closed by server.\n");
					exit(0);
				}
				strcat(buffer, message);
				memset(message, '\0', sizeof(message));
			}	
			char *termChar = strstr(buffer, "&&");		// Remove terminal characters
			*termChar = '\n';
			*(termChar+1) = '\0';

			// Print message to output window
			waddstr(output, buffer);
			wrefresh(output);
		}

		// Check stdin
		if (FD_ISSET(STDIN_FILENO, &readfds)) {
		
			// Get input from user
			if (wgetnstr(input, message, bufferLength) != OK) {
				continue;
			}
			werase(input);
			wrefresh(input);			
	
			// Check if message is greater than 500 characters
			if (strlen(message) > 500) {
				waddstr(output, "Message should be less than 500 characters.\n");
				wrefresh(output);
				continue;
			}
			// If user types \quit, close socket and exit program
			if (strcmp(message, "\\quit") == 0) {
				close(socketFD);
				endwin();
				exit(0);
			}
					
			strcpy(buffer, handle);		// Copy handle to buffer
			strcat(buffer, message);	// Concatenate message to buffer
			strcat(buffer, "\n");		// COncatenate new line

			// Send message over socket
			while (charsTotal < strlen(buffer)) {
				charsTrans = send(socketFD, buffer + charsTotal*sizeof(char), sizeof(buffer), 0);
				if (charsTrans < 0) error("CLIENT: ERROR writing to socket");
				charsTotal += charsTrans;
			}

			// Print message to output window
			waddstr(output, buffer);
			wrefresh(output);
		}
	}	
}

