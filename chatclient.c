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

	// Declare/initialize  variables
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	size_t bufferLength = 1024;
	size_t handleLength = 256;
	char *buffer = calloc(bufferLength, sizeof(char));
	char *message = calloc(bufferLength, sizeof(char));
	char *outputMessage = calloc(bufferLength, sizeof(char));
	char *handle = calloc(handleLength, sizeof(char));
	int charsTotal, charsTrans;
	char inputChar;	
	int msgIndex = 0;

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
	WINDOW *inputW, *outputW;

	initscr();
	cbreak();
	echo();

	inputW = newwin(1, COLS, LINES - 1 ,0);		// User inputs at bottom of screen
	outputW = newwin(LINES - 1, COLS, 0, 0);		// Output displayed above
	wmove(outputW, LINES - 2, 0);
	scrollok(outputW, TRUE);
	keypad(inputW, true);

	fd_set readfds;		// Declare read file descriptor set
	mvwprintw(inputW, 0, 0, handle);			// Print handle to screen
	wrefresh(inputW);	

	// Enter infinite loop
	while (1) {
		memset(buffer, '\0', bufferLength);		// Clear out the buffers
		memset(message, '\0', bufferLength);
		charsTotal = 0;							// Reset variables
		charsTrans = 0;

		FD_ZERO(&readfds);						// Clear out fd set
		FD_SET(socketFD, &readfds);				// Add socket FD
		FD_SET(STDIN_FILENO, &readfds);			// Add stdin 
	
	
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
			waddstr(outputW, buffer);
			wrefresh(outputW);
		}

		// Check stdin
		if (FD_ISSET(STDIN_FILENO, &readfds)) {
		
			// Read character from user
			inputChar = wgetch(inputW);
		
			if (inputChar == KEY_UP || inputChar == KEY_DOWN || inputChar == KEY_LEFT || inputChar == KEY_RIGHT) {
				continue;
			}
			if (inputChar == KEY_BACKSPACE || inputChar == '\b' || inputChar == 8 ||
					inputChar == KEY_DC || inputChar == 127) {
				wdelch(inputW);
				wrefresh(inputW);
				if (msgIndex != 0) {
					outputMessage[msgIndex - 1] = 0;
					msgIndex--;
				}
			} else {
				outputMessage[msgIndex] = inputChar;
				msgIndex++;
			}
	
			if (outputMessage[msgIndex-1] == '\n') {

					// Check if message is greater than 500 characters
					if (strlen(outputMessage) > 500) {
						waddstr(outputW, "Message should be less than 500 characters.\n");
						wrefresh(outputW);
						continue;
					}
					// If user types \quit, close socket and exit program
					if (strcmp(outputMessage, "\\quit\n") == 0) {
						close(socketFD);
						endwin();
						exit(0);
					}
							
					strcpy(buffer, handle);		// Copy handle to buffer
					strcat(buffer, outputMessage);	// Concatenate message to buffer

					// Send message over socket
					while (charsTotal < strlen(buffer)) {
						charsTrans = send(socketFD, buffer + charsTotal*sizeof(char), sizeof(buffer), 0);
						if (charsTrans < 0) error("CLIENT: ERROR writing to socket");
						charsTotal += charsTrans;
					}

					// Clear output message buffer
					memset(outputMessage, '\0', bufferLength);
					//Reset msgIndex
					msgIndex = 0;
					// Print message to output window
					waddstr(outputW, buffer);
					wrefresh(outputW);

					// Update input window
					werase(inputW);
					mvwprintw(inputW, 0, 0, handle);			// Print handle to screen
					wrefresh(inputW);
			}
		}
	}	
}

