/*
*
* A Java remake of beej's talker.c, the "client" demo with additions for Lab 1.
* Group 13: Wulfy Boothe, Lane Little, David Harris
*/

import java.net.*; // for DatagramSocket, DatagramPacket, and InetAddress
import java.io.*; // for IOException
import java.text.DecimalFormat; //for Formating
import java.nio.ByteBuffer;

public class ClientUDP { 

   private static final int TIMEOUT = 3000; // Resend timeout (milliseconds) 
   private static final int MAXTRIES = 5; // Maximum retransmissions 
   private static final int MAX_MESSAGE_LENGTH = 1024;

   public static void main(String[] args) throws IOException {
   
      if (args.length < 5) 
      {
      // Test for correct # of args     
         System.err.println("Parameter(s): <Servername> <Port#> <requestID> <hostname h1, h2, h3, etc>");
         return; 
      }
      
      ByteBuffer msg = ByteBuffer.allocate(MAX_MESSAGE_LENGTH);
      byte[] hostnames = new byte[MAX_MESSAGE_LENGTH - 9]; 
      int RID = Integer.parseInt(args[2]);
      
      int TML = 9; //TML starts at 9 bytes - Magic#/TML/GID/Checksum/RequestID
      
      int length_index = 0;
      for (int i = 3; i < args.length; i++) {
         byte[] hostname = args[i].getBytes();

	 TML += hostname.length + 1;         
	 if (TML > MAX_MESSAGE_LENGTH) {
	    System.err.println("Message length exceeds the maximum 1024 bytes");
	    return;
	 }

	 hostnames[length_index] = hostname.length;
         for (int j = 0; j < hostname.length; j++) {
	    hostnames[length_index + j + 1] = hostname[j]; 
	 }
	length_index += hostname.length;
      } 

      try{ //try block for attempting to send and recieve the message
         InetAddress serverAddress = InetAddress.getByName(args[0]); // Server address 
      
      
      	 // Convert input String to bytes using the default character encoding 
         int servPort = Integer.parseInt(args[1]); //servPort as an integer 
      
         DatagramSocket socket = new DatagramSocket();
      
         socket.setSoTimeout(TIMEOUT); // Maximum receive blocking time (milliseconds) 
      
      
         //DatagramPacket sendPacket = new DatagramPacket(bytesToSend, // Sending packet 
         //   bytesToSend.length, serverAddress, servPort);
         ByteBuffer response = ByteBuffer.allocate(MAX_MESSAGE_LENGTH);   
         DatagramPacket receivePacket = // Receiving packet         
            new DatagramPacket(new byte[MAX_MESSAGE_LENGTH], MAX_MESSAGE_LENGTH);
            
         int tries = 0; // Packets may be lost, so we have to keep trying 
         boolean receivedResponse = false; 
         do 
         { 
            //socket.send(sendPacket);
            try   
            {
               socket.receive(receivePacket); //recieves response from server
               receivedResponse = true;
            } 
            catch (SocketTimeoutException ste) 
            {
               tries ++; //increments tries
            }
         } while ((!receivedResponse) && (tries < MAXTRIES)); 
         if (receivedResponse)
         {

         } 
         else 
            System.out.println("No response -- giving up."); 
         socket.close(); 
      
      }
      catch (UnknownHostException uhe)
      {
         System.err.println("Failed to find host.");
         return;
      }
      catch (SocketException se)
      {
         System.err.println("Failed to create socket.");
         return;
      }  
   }     
}
