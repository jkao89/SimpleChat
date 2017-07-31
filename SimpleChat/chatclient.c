#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	size_t bufferLength = 1024;
	size_t handleLength = 512;
	char *buffer = calloc(bufferLength, sizeof(char));
	char *message = calloc(bufferLength, sizeof(char));
	char *handle = calloc(handleLength, sizeof(char));
	char state[32];
	int charsTotal, charsTrans;

	// Check number of arguments  
	if (argc < 3)	{ fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(0); }
	
	memset(state, '\0', sizeof(state));
	
	// Prompt client for handle
	printf("Please enter user handle: ");
	getline(&handle, &handleLength, stdin);
	char *newLine = strchr(handle, '\n');		// Remove new line character
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
	strcpy(state, "send");
	
	while (1) {
		memset(buffer, '\0', bufferLength);
		memset(message, '\0', bufferLength);
		charsTotal = 0;
		charsTrans = 0;
		

		// Sending state
		if (strcmp(state, "send") == 0) {
			printf("%s", handle);
			getline(&message, &bufferLength, stdin);
			
			// Check if message is greater than 500 characters
			if (strlen(message) > 500) {
				printf("Message should be less than 500 characters.\n");
				continue;
			}
			// If user types \quit, close socket and exit program
			if (strcmp(message, "\\quit\n") == 0) {
				close(socketFD);
				printf("Closed Connection.\n");
				exit(0);
			}
					
			strcpy(buffer, handle);		// Copy handle to buffer
			strcat(buffer, message);	// Concatenate message to buffer
			
			while (charsTotal < strlen(buffer)) {
				charsTrans = send(socketFD, buffer + charsTotal*sizeof(char), sizeof(buffer), 0);
				if (charsTrans < 0) error("CLIENT: ERROR writing to socket");
				charsTotal += charsTrans;
			}
			memset(state, '\0', sizeof(state));
			strcpy(state, "receive");
		// Receiving state
		} else {
			while (strstr(buffer, "&&") == NULL) {	
				charsTrans = recv(socketFD, message, sizeof(message) - 1, 0);
				if (charsTrans < 0) error("CLIENT: ERROR reading from socket");
				if (charsTrans == 0) {
					printf("Connection closed by serve.\n");
					close(socketFD);
					exit(1);
				}
				strcat(buffer, message);
				memset(message, '\0', sizeof(message));
			}	
			char *termChar = strstr(buffer, "&&");
			*termChar = '\0';	
			printf("%s\n", buffer);
			memset(state, '\0', sizeof(state));
			strcpy(state, "send");
		}
	}
}
	

	
