Description: A simple client server chat program.

The server side is implemented in Java and utilizes the Socket and ServerSocket 
classes provided in the java.net package. The server lives forever until
terminated by a SIGINT signal. It accepts a connection from a client on the port
specified when first starting the program. The main thread continually receives messages
from the socket input stream, while a separate thread is used to send messages
to the client. 

The client side is implemented in C and connects to the server at the specified
hostname and port. A select() call is used to check on the both the socket and standard
input file descriptors to allow the client to receive messages without blocking. In
order to simulate a chat application, the curses library was used to split the input
and output windows. The user sends messages at the bottom of the screen, while messages
between the client and server appear above. 


Instructions: Run the makefile by entering 'make' in directory with all the files

			  Start the server by entering 'java chatserve [portnumber]'

			  Start the client by entering 'chatclient [hostname] [portnumber]'

			  Enter a handle for the client.

			  Messages can now be exchanged from either side simultaneously.

			  *Note that while the client is typing, messages sent from the server into
			  the socket will be delayed until the client sends his/her message. A 
			  multi-threaded client side has not yet been implemented which would remove this
			  issue.

			  Either side may type '\quit' in order to end the connection. The server side
			  will continue to listen for connections until terminated by a SIGINT.

Sources:

http://docs.oracle.com/javase/tutorial/networking/sockets/index.html
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
http://invisible-island.net/ncurses/ncurses-intro.html
https://stackoverflow.com/questions/32088603/how-to-keep-stdin-at-bottom-the-terminal-in-c
https://www.gnu.org/software/libc/manual/html_node/Waiting-for-I_002fO.html

*Code from a CS344 Operating Systems courses was utilized to set up the socket in C. A portion
of the code was supplied by the instructor. 



