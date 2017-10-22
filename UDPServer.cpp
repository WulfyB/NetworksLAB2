/*
** listener.c -- a datagram sockets "server" demo
* This UDPServer is based on the beej demo listed above
* This version is a modification of the UDPServer created for LAB1
* While the following changes have been listed here, this list is not
* to be assumed to be exhaustive.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

//#define MYPORT "10023"	// the port users will be connecting to

#define MAXBUFLEN 8192

void performOperation(char op, char message[], int messageSize, char response[], int *responseSize);
void uppercase(char message[], int messageSize);
int isVowel(char letter);


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	short numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	int hasError = 0;
	int lengthError = 0;
	int magicError = 0;
	int checkSumError = 0;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	while (1)
	{
		hasError = 0;
		checkSumError = 0;
		lengthError = 0;
		magicError = 0;
		if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and bind to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
				perror("listener: socket");
				continue;
			}

			if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				perror("listener: bind");
				continue;
			}

			break;
		}

		if (p == NULL) {
			fprintf(stderr, "listener: failed to bind socket\n");
			return 2;
		}

		freeaddrinfo(servinfo);

		printf("listener: waiting to recvfrom...\n");

		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		printf("listener: got packet from %s\n",
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s));


		unsigned long recMagicNumber = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
		recMagicNumber = ntohl(recMagicNumber);
		unsigned short recTML = buf[3] << 8 | buf[4];
		recTML = noths(recTML);
		if (numbytes != recTML)
		{
			lengthError = 1;
		}
		unsigned short byteSum;
		unsigned int count = 0;
		for (short i = 0; i < numbytes; i++)
		{
			if (i = 7)
			{
				continue;
			}
			byteSum += buf[i];
		}
		unsigned char checksum = getCheckSum(byteSum); //Obviously losing data at the moment, need updated to double

		if (checksum != buf[7])
		{
			hasError = 1;
			checkSumError = 1;
		}
		unsigned char recGID = buf[6];
		unsigned char recCSum = buf[7];
		unsigned char requestID = buf[8];
		unsigned int numOfHosts = 0;
		unsigned long magicNumber = 0x4A6F7921;

		if (magicNumber == recMagicNumber)
		{
			hasError = 1;
			magicError = 1;
		}
		if (hasError)
		{
			char errBuf[9];
			magicNumber = htonl(magicNumber);
			errBuf[0] = magicNumber << 24;
			errBuf[1] = magicNumber << 16;
			errBuf[2] = magicNumber << 8;
			errBuf[3] = magicNumber;
			short TML = 9;
			TML = htons(TML);
			errBuf[4] = TML << 8;
			errBuf[5] = TML;
			errBuf[6] = 0;
			errBuf[7] = 13; //hardcoded, group ID is 13
			if (lengthError)
			{
				errBuf[8] = errBuf[8] | 0x01; //error code, b0 should be 1 indicating length mismatch.
			}
			if (checkSumError)
			{
				errBuf[8] = errBuf[8] | 0x02; //error code, b1 should be 1 indicating checksum is incorrect.
			}
			if (magicError)
			{
				errBuf[8] = errBuf[8] | 0x04; //error code, b0 should be 1 indicating no or wrong magic number.
			}
			//errBuf[8] should now be all errors from received message
			for (short i = 0; i < 9; i++)
			{
				byteSum += errBuf[i];
			}
			errBuf[6] = getCheckSum(byteSum);
			if (sendto(sockfd, errBuf, TML, 0, (struct sockaddr *)&their_addr, sizeof(their_addr)) !=
				TML)
			{
				perror("Failed to send error for invalid message; resuming listening");
			}
			continue; //Moves the program back to start of while loop to wait for next message.
		}
		std::string hostIP[256];
		for (int i = 0; i < recTML; i++)
		{
			unsigned short hostLength = buf[i];
			std::string hostName = "";
			for (int j = 0; j < hostLength; j++)
			{
				hostName += buf[i + j];

			}
			numOfHosts++;
			//hostname should be constructed
			//Need to fetch the ip address and add to return message
			hostIP[i] = fetchHostIP(hostName);

		}
		//Host IP Addresses should now be stored in the hostIP array. Now need to put into buffer


		char message[MAXBUFLEN];
		numOfHosts *= 4; //each host takes 4 bytes
		unsigned short TML = 9 + numOfHosts;
		magicNumber = htonl(magicNumber);
		buf[0] = magicNumber << 24;
		buf[1] = magicNumber << 16;
		buf[2] = magicNumber << 8;
		buf[3] = magicNumber;
		numOfHosts /= 4;
		buf[4] = TML << 8;
		buf[5] = TML;
		buf[6] = 13; //Hardcoded
		buf[7] = 0;
		buf[8] = requestID;
		int hostTracker = 0;
		for (int i = 9; 0 < numOfHosts; i++)
		{
			std::string host = hostIP[hostTracker];
			unsigned long hostnum = std::stoi(host);
			hostnum = htonl(host);
			buf[i] = host<< 24;
			i++;
			buf[i] = host<< 16;
			i++;
			buf[i] = host << 8;
			i++;
			buf[i] = host;
			numOfHosts--;
		}

		//calculate checksum
		for (short i = 0; i < TML; i++)
		{

			byteSum += buf[i];
		}
		buf[7] = getCheckSum(byteSum);
		if (sendto(sockfd, buf, TML, 0, (struct sockaddr *)&their_addr, sizeof(their_addr)) !=
			TML)
		{
			perror("Failed to echo");
		}

		close(sockfd);
	}//END OF BIG WHILE
	return 0;
}
std::string fetchHostIP(std::string hostName)
{

	return ""; //maybe find int?
}

unsigned char getCheckSum(short byteSum)
{
	unsigned char checksum = 0;
	while ((byteSum & 0xFF00) > 0) {
		unsigned char leftHalf = (char)(byteSum >> 8);
		unsigned char rightHalf = (char)byteSum;
		byteSum = (short)(leftHalf + rightHalf);

	}
	checksum = byteSum;
	return checksum;
}
