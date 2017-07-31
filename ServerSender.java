import java.net.*;
import java.io.*;

class ServerSender implements Runnable {
	Socket sock;
	PrintWriter sockOut;
	BufferedReader consoleIn;	
	String output, handle;

	// Constructor
	ServerSender(Socket sock, String handle) {
		this.sock = sock;

		// Initialize output stream and console input stream
		try {
			sockOut = new PrintWriter(this.sock.getOutputStream(), true);
			consoleIn = new BufferedReader(
				new InputStreamReader(System.in)); 
			this.handle = handle;
		} catch (IOException e) {
			System.out.println("Exception caught in ServeSender constructor");
			System.out.println(e.getMessage());
		}
	}

	
	public void run() {
		try {	
			while(true) {
				System.out.print(handle);		// Print handle

				// Read from stdin
				output = consoleIn.readLine();

				// Check if message is greater  than 500 characters
				while (output.length() > 500) {
					System.out.println("Message should be less than 500 characters.");
					output = consoleIn.readLine();
				}
				// Server side enters \quit
				if (output.equals("\\quit")) { 
					sock.shutdownInput();		// Shut down socket input stream
					break;
				}
				output = handle + output;		// Append handle
				sockOut.write(output + "&&");	// Write to socket, appending terminal characters
				sockOut.flush();
			}
		} catch (IOException e) {
			System.out.println("Exception caught in ServerWriter.");
			System.out.println(e.getMessage());
		} 
	}		
}
