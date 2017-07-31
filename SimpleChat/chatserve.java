import java.net.*;
import java.io.*;

public class chatserve {

	public static void main(String[] args) throws IOException {
		
		// Initialize and declare variables
		int portNumber = Integer.parseInt(args[0]);
		String handle = "Server>";
		String input, output;
		Console c = System.console();	
	
		// Create socket and bind port number
		try (ServerSocket serverSock = new ServerSocket(portNumber)) {
		
			// Enter infinite loop
			while (true) {
				// Create socket, listen for and accept connections, and create input
				// and output streams for the socket in a try-with-resources statement
				try (Socket clientSock = serverSock.accept();
					 PrintWriter sockOut = new PrintWriter(clientSock.getOutputStream(), true);
					 BufferedReader sockIn = new BufferedReader(
							 new InputStreamReader(clientSock.getInputStream()));
					) {
					
					clientSock.setTcpNoDelay(true);			// Disable Nagle's Algorithm
					// Loop while there are messages to read
					while ((input = sockIn.readLine()) != null) {
						System.out.println(input);
						System.out.print(handle);
						output = c.readLine();
						while (output.length() > 500) {
							System.out.println("Message should be less than 500 characters.");
							output = c.readLine();
						}
						if (output.equals("\\quit")) {
							break;
						}
						output = handle + output; 			// Append terminal characters
						sockOut.write(output + "&&");
						sockOut.flush();

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
