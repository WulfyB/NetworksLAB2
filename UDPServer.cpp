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

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	while (1) {
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
		
		//CALCULATE CHECKSUM

		//Process the incoming bytes

		unsigned long recMagicNumber = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
		recMagicNumber = ntohl(recMagicNumber);
		unsigned short recTML = buf[3] << 8 | buf[4];
		recTML = noths(recTML);
		if (numbytes != recTML)
		{
			//construct fail message
		}
		unsigned char checksum = 0;
		unsigned short bytesum = 0;
		unsigned int count = 0;
		for (short i = 0; i < numbytes; i++)
		{
			if (i = 5)
			{
				continue;
			}
			bytesum += buf[i];
		}
		checksum = bytesum; //Obviously losing data at the moment, need updated to double
		//check method of finding checksum
		//while ((bytesum & 0xff00) > 0) 
		//{
		//		bytesum += 0x1;
		//	}
		if (checksum != buf[5])
		{

		}
		unsigned char recGID = buf[6];
		unsigned char recCSum = buf[7];
		unsigned char requestID = buf[8];
		unsigned int numOfHosts = 0;
		unsigned long magicNumber = 0x4A6F7921;
		
		if (magicNumber == recMagicNumber) {
			for (int i = 0; i < recTML; i++)
			{
				unsigned short hostLength = buf[i];
				std::string hostName = "";
				unsigned long hostIP;
				for (int j = 0; j < hostLength; j++)
				{
					hostName += buf[i + j];

				}
				numOfHosts++;
				//hostname should be constructed
				//Need to fetch the ip address and add to return message
				hostIP = fetchHostIP(hostName);
			}

		}
		else //magicNumber error!
		{

		}
		numOfHosts * 4; //each host takes 4 bytes
		unsigned short TML = 9 + numOfHosts;

		buf[0] = TML;
		buf[1] = requestID;

		//Add response back to buffer

		if (sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, sizeof(their_addr)) !=
			strlen(buf))
		{
			perror("Failed to echo");
		}

		close(sockfd);
	}//END OF BIG WHILE
	return 0;
}
unsigned long fetchHostIP(std::string hostName)
{
	
	return 0;
}
