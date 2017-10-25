/*
*
* A Java remake of beej's talker.c, the "client" demo with additions for Lab 1.
* Group 13: Wulfy Boothe, Lane Little, David Harris
*/

import java.net.*; // for DatagramSocket, DatagramPacket, and InetAddress
import java.io.*; // for IOException
import java.text.DecimalFormat; //for Formating
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.nio.ByteOrder;

public class UDPClient { 

   private static final int TIMEOUT = 3000; // Resend timeout (milliseconds) 
   private static final int MAXTRIES = 7; // Maximum retransmissions 
   private static final int MAX_MESSAGE_LENGTH = 1024;

   public static void main(String[] args) throws IOException {
   
      UDPClient client = new UDPClient();
   
      if (args.length < 4) 
      {
      // Test for correct # of args     
         System.err.println("Parameter(s): <Servername> <Port#> <requestID> <hostname h1, h2, h3, etc>");
         return; 
      }
      
      ByteBuffer msg = ByteBuffer.allocate(MAX_MESSAGE_LENGTH);
      byte[] hostnames = new byte[MAX_MESSAGE_LENGTH - 9]; 
      
      
      int RID = Integer.parseInt(args[2]);
      if (RID > 127 || RID < 0) {
         System.err.println("Parameter(s): RID must be between 0 and 127");
         return;
      }      
   
      short TML = 9; //TML starts at 9 bytes - Magic#/TML/GID/Checksum/RequestID
      
      int length_index = 0;
      for (int i = 3; i < args.length; i++) {
         byte[] hostname = args[i].getBytes();
         if (hostname.length > 255) {
            System.err.println("hostname cannot contain more than 255 characters");
            return;  
         }
      
         TML += hostname.length + 1;         
         if (TML > MAX_MESSAGE_LENGTH) {
            System.err.println("Message length exceeds the maximum 1024 bytes");
            return;
         }
         
         hostnames[length_index] = (byte) hostname.length;
         for (int j = 0; j < hostname.length; j++) {
            hostnames[length_index + j + 1] = hostname[j]; 
         }
         length_index += hostname.length + 1;
      } 
      
   
   
      int magicNum = 0x4A6F7921;
      byte GID = 13;
      byte RID_byte = (byte) RID;
      byte checksum = 0;
   
      msg.putInt(magicNum);
      msg.putShort(TML);
      msg.put(GID);
      msg.put(checksum);
      msg.put(RID_byte);
      msg.put(hostnames, 0, TML - 9);
      
      //calculate checksum
      checksum = (byte) ~client.calculateChecksum(msg, TML); //return 1's complement of checksum
      byte[] bytesToSend = msg.array();
      bytesToSend[7] = checksum;
   
      try{ //try block for attempting to send and recieve the message
      
         String server = args[0]; // Server name 
      
      	 // Convert input String to bytes using the default character encoding 
         int portNum = Integer.parseInt(args[1]); //port number as an integer 
      
         DatagramChannel channel = DatagramChannel.open();
         SocketAddress address = new InetSocketAddress(server, portNum);
         DatagramSocket socket = channel.socket();
         socket.setSoTimeout(TIMEOUT); 
	 channel.connect(address); //Not the same as TCP connect. It affectively binds to one server
         
         
         //Make buffer for server response
         ByteBuffer response = ByteBuffer.allocate(MAX_MESSAGE_LENGTH); 
            
         int tries = 0; // Packets may be lost, so we have to keep trying 
         boolean receivedResponse = false; 
         do 
         { 
            msg.flip(); //Very important. Sets position to 0
            channel.send(msg, address);
            try   
            {
               channel.receive(response); //recieves response from server
               System.out.println("Got response");
               response.flip(); 
               int receivedMagicNum = response.getInt();
               if (receivedMagicNum != magicNum) {
                  System.out.println("Invalid Magic Num - Want: " + magicNum + " Received: " + receivedMagicNum);
                  tries++; //Invalid (or missing) Magic Number
                  response.clear();
                  continue;
               }
               
               TML = response.getShort();
               if (TML < (9 + 4 * (args.length - 3))) { //TML must be (9 + 4 x #hostnames) bytes long
                  tries++; //Message too short
                  response.clear();
                  System.out.println("Too Short");
                  continue;
               }
               
               //Lastly, check the checksum
	       byte[] responseBytes = response.array();
               int returnedChecksum = responseBytes[7] & 0xFF;
               System.out.println("Returned checksum: " + returnedChecksum);
	       responseBytes[7] = 0;
               int sum = (int) client.calculateChecksum(response, TML) & 0xFF;
	       System.out.println("Calculated sum: " + sum);
               int checksumResult = sum + returnedChecksum;
	       System.out.println("checksumResult: " + checksumResult);
               
	       if (checksumResult != 0xFF ) {
                  tries++; //invalid checksum
                  response.clear();
                  System.out.println("Bad Checksum");
                  continue;
               }

               receivedResponse = true;
            } 
            catch (SocketTimeoutException ste) 
            {
               tries ++; //increments tries
            }
         } while ((!receivedResponse) && (tries < MAXTRIES)); 
         if (receivedResponse)
         {
            GID = response.get();
            checksum = response.get();
            RID_byte = response.get();
            
            for (int i = 3; i < args.length; i++) {
               int ip = response.getInt();
               System.out.println("Hostname: " + args[i] + " IP address: " + client.getIPString(ip));
            }
         } 
         else 
            System.out.println("No valid response after 7 tries -- giving up."); 
         socket.close();
         channel.close(); 
      
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
   
   public byte calculateChecksum(ByteBuffer buf, int TML) {
      byte checksum = 0;
      short checksum_short = 0; 
      byte[] buffer = buf.array(); 
      for(int i = 0; i < TML; i++) {
         checksum_short += (short)(buffer[i] & 0xFF);
      }
     System.out.println("Checksum sum: " + checksum_short); 
      while ((checksum_short & 0xFF00) > 0) {
         int leftHalf = (checksum_short >> 8) & 0xFF;
         int rightHalf = checksum_short & 0xFF;
         checksum_short = (short) (leftHalf + rightHalf);
      }
            
      checksum = (byte) checksum_short;
      return checksum;
   }
   
   public String getIPString(int ip) { 
         String ipStr = 
         String.format("%d.%d.%d.%d",
         (ip >> 24 & 0xFF),   
         (ip >> 16 & 0xFF),             
         (ip >> 8 & 0xff),    
         (ip & 0xff));
        
      return  ipStr;
   } 
}

