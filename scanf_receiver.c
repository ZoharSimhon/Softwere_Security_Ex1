// The libraries required throughout the process are:
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>

#define SERVER_PORT 5061
#define BUFFER_SIZE 1000

int main()
{
	// create a TCP Connection
	struct sockaddr_in serverAddress;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// If the socket is not established the method sock() return the value -1 (INVALID_SOCKET)
	if (sockfd == -1)
	{
		perror("socket");
		return -1;
	}

	// the "memset" function copies the character "\0"
	memset(&serverAddress, 0, sizeof(serverAddress));

	// AF_INET is an Address Family that is Internet Protocol IPv4 addresses
	serverAddress.sin_family = AF_INET;
	// any IP at this port (any address to accept incoming messages)
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	// the "htons" convert the port to network endian (big endian)
	serverAddress.sin_port = htons(SERVER_PORT);

	// Often the activation of the method bind() falls with the message "Address already in use".
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
	{
		perror("setsockopt");
		return -1;
	}

	// Link an address and port with a socket is carried out by the method bind().
	int bindResult = bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	// On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
	if (bindResult == -1)
	{
		printf("Bind() failed with error code : %d\n", errno);
		return -1;
	}

	int listenResult = listen(sockfd, 1);
	if (listenResult == -1)
	{
		printf("listen() failed with error code : %d\n", errno);
		return -1;
	}
	printf("The receiver is listening...\n");

	// infinity loop for the incoming requests
	while (1)
	{
		// accept and incoming connection:
		struct sockaddr_in clientAddress;
		socklen_t clientAddressLen = sizeof(clientAddress);

		memset(&clientAddress, 0, sizeof(clientAddress));
		clientAddressLen = sizeof(clientAddress);

		int clientSocket = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
		if (clientSocket == -1)
		{
			perror("accept() failed");
			return -1;
		}
		printf("The server is conected\n\n");

		// accept the file
		char buffer[BUFFER_SIZE];
		int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
		// If the method returns -1 then there is an error.
		if (bytesReceived == -1)
		{
			printf("recv failed with error code : %d", errno);
			return -1;
		}
		// If the function returns 0 it reflects that the connection is closed.
		if (bytesReceived == 0)
		{
			printf("Sender disconnect, exit.\n");
			return -1;
		}
		buffer[bytesReceived] = '\0';

		printf("The password we have got is: %s \n", buffer);

		// close the connection:
		close(clientSocket);
	}

}