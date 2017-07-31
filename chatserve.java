import java.net.*;
import java.io.*;

public class chatserve {

	public static void main(String[] args) throws IOException {
		
		// Initialize and declare variables
		int portNumber = Integer.parseInt(args[0]);
		String handle = "Server>";		// Hard coded handle
		String input;	// Stores message from client
		Console c = System.console();	
	
		// Create socket and bind port number
		try (ServerSocket serverSock = new ServerSocket(portNumber)) {
		
			// Enter infinite loop
			while (true) {
				// Create socket, listen for and accept connections, and create input
				// streams for the socket in a try-with-resources statement
				try (Socket clientSock = serverSock.accept();
					 BufferedReader sockIn = new BufferedReader(
							 new InputStreamReader(clientSock.getInputStream()));
					) {
					
					clientSock.setTcpNoDelay(true);			// Disable Nagle's Algorithm

					// Create another thread to send messages with
					ServerSender ss = new ServerSender(clientSock, handle);
					Thread sendThread = new Thread(ss);
					sendThread.start();

					// Loop while there are messages to read
					while ((input = sockIn.readLine()) != null) {
						System.out.println(input);
					}
					System.out.println("Connection closed.");
				} catch (IOException e) {
					System.out.println("Exception caught when opening socket or when "
							+ "listening/accepting a connection");
					System.out.println(e.getMessage());
					e.printStackTrace();
				}
			}
		}
	}
	
}
