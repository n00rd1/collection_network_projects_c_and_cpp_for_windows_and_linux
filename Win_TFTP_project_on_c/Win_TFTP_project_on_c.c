// TFTP (IP4) server with UDP packets
// This server is designed to transfer files over the TFTP protocol (used inside a local network)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>

#define DIR_OF_FILE		"C:\\tftp\\1.txt"		// default folder for tftp files
#define CLIENT_DEF_IP4	"192.168.1.1"
#define SERVER_DEF_IP4	"192.168.1.2"
#define DEF_TIMEOUT		50

#define BUFLEN			512						// max size of buffer
#define PORT_1			69
#define PORT_2			4259

// error code
const char* error_str[] = {
	"OK",										// All ok
	"err1"
};

// tftp transfer mode
enum mode {
	NETASCII = 1,
	OCTET
};

// tftp opcode mnemonic
enum opcode {
	RRQ = 1,
	WRQ,
	DATA,
	ACK,
	ERR0R
};

// structure for statistic
struct stat {
	int blocks;
	int errors;
} stat;

// tftp message structure
typedef union {
	uint16_t opcode;
	struct {
		uint16_t opcode; // RRQ
		uint8_t filename_and_mode[BUFLEN + 2];
	} request;
	struct {
		uint16_t opcode; // DATA
		uint16_t block_number;
		uint8_t data[BUFLEN];
	} data;
	struct {
		uint16_t opcode; // ACK
		uint16_t block_number;
	} ack;
	struct {
		uint16_t opcode; // ERR0R
		uint16_t error_code;
		uint8_t error_string[BUFLEN];
	} error;
} tftp_message;

	// ---  Send a error message to client  --- //
// This function sends an error message to the TFTP client.
// It takes as input parameters a socket descriptor (s),
// the error code and message to be sent, and the client's socket address and length.
int tftp_send_error_msg(int s, int error_code, char* error_string, struct sockaddr_in* sock, int slen) {
	tftp_message m = { 0 };
	unsigned c = 0;

	// Check if the error message is too long.
	if (strlen(error_string) >= BUFLEN) {
		fprintf(stderr, "server: tftp_send_error(): error string too long\n");
		return -1;
	}

	// Set the opcode of the TFTP message to ERR0R (5).
	m.opcode = htons(ERR0R);

	// Set the error code of the TFTP message.
	m.error.error_code = error_code;

	// Copy the error string to the TFTP message.
	strcpy((char*)m.error.error_string, error_string);

	// Send the TFTP message to the client.
	if ((c = sendto(s, (char*)&m, 4 + (int)(strlen(error_string)) + 1, 0, (struct sockaddr*)sock, slen)) < 0)
		perror("server: sendto()");

	// Return the number of bytes sent.
	return c;
}

	// ---  Check the client's request for file name and mode, and verify file availability	--- //
// This function checks the TFTP client request for the file name and mode of transfer.
// It also checks if the requested file is available for transfer.
// If the file is available, it returns a file pointer to the opened file.
// If there is an error, it sends an error message to the client and returns NULL.
// The function takes as input parameters a pointer to the TFTP message (m),
// the length of the TFTP message (cnt), the client's socket address (client),
// the output socket descriptor (sock_output), and the directory in which to search for the file (dir).
FILE* file_open_by_request(tftp_message* m, int cnt, struct sockaddr_in client, int sock_output, char* dir) {
	char* mode, * tmp;
	char filename[256];
	FILE* fd;

	// Parse client request
	tmp = (char*)m->request.filename_and_mode;
	snprintf(filename, sizeof(filename), "%s%s", (char*)dir, (char*)tmp);

	// Try to find a file name on request
	int	i = cnt - 2;
	while (i--) {
		if (*tmp == 0)
			break;
		tmp++;
	}


	// Check if there is no end of line in the file name.
	if (*tmp)
		tftp_send_error_msg(sock_output, 3, (char*)"broken file", &client, sizeof(&client));

	// Check if there is no mode of transfer.
	if (i == 0)
		tftp_send_error_msg(sock_output, 4, (char*)"not found mod transfer!", &client, sizeof(&client));

	mode = tmp + 1;
	i--;

	// Try to find a mode transfer on request
	while (i--) {
		if (*tmp == 0)
			break;
		tmp++;
	}

	// Check if there is no end of line in the mode of transfer.
	if (*tmp)
		tftp_send_error_msg(sock_output, 3, (char*)"broken file", &client, sizeof(&client));				// no end of line

	// Check if the mode of transfer is octet.
	if (strcmp(mode, "octet") != 0)
		tftp_send_error_msg(sock_output, 4, (char*)"error mode transfer!", &client, sizeof(&client));		// check mode for octet

	// Try to open the requested file.
	if ((fd = fopen(filename, "rb")) == NULL) {
		printf("cant's open file %s\n\n ", filename);
		tftp_send_error_msg(sock_output, 5, (char*)"can't open a file!", &client, sizeof(&client));
		return NULL;
	}
	
	return fd;
}

	// ---  Time of taking request (timeout)    --- //
// socket: socket 
// time: timeout for receive request
int recvfromTimeOutUDP(SOCKET socket, long sec) {
	// Setup timeval variable
	struct fd_set fds;
	struct timeval timeout;
	timeout.tv_sec = sec;

	// Setup fd_set structure
	FD_ZERO(&fds);
	FD_SET(socket, &fds);

	//--------------------------------------------------------------------------
	// Returns value:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, &fds, 0, 0, &timeout);
}

	// ---  Function to start a TFTP server and handle incoming requests	--- //
// time: timeout for receive request
// dir: directory path for server to serve files from
// client_ip: IP address of the client
// server_ip: IP address of the server
// stat: struct to store statistics of the server's performance
char* tftp_server(int time, char* dir, char* client_ip, char* server_ip, struct stat* stat) {
	FILE* fd = 0;
	int sock = 0;						// socket for get a request and socket for send a request
	WSADATA wsaData;

	int code = 0;
	int timing = 0;		 					// timer, buffer, close
	int cnt = 0;							// temp to save a answert of requests

	int dlen = 0;
	int errors_number = 0;				// temp to check the number of errors
	int c_len = 0;

	uint8_t data[BUFLEN];                   // Buffer for data
	uint16_t block_number = 0;              // Block number

	// Structures
	struct sockaddr_in server = { 0 };				// Server
	struct sockaddr_in client = { 0 };              // Real client ip address

	// Set server IP address
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(server_ip);
	server.sin_port = htons(PORT_1);

	// Structures for message
	tftp_message get_m = { 0 };             // Structures for incoming message
	tftp_message send_m = { 0 };            // Structures for outgoing message

	// Initialize Winsock
	if ((cnt = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		printf("WSAStartup failed: %d\n", cnt);
		code = 1;
		goto close_all;     //all close
	}


	// Create a SOCKET for listening for incoming connection requests
	if ((sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("socket function failed with error: %u\n", WSAGetLastError());
		code = 2;
		goto close_all;     //all close
	}

	// Bind the socket
	if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
		code = 3;
		goto close_all;     //all close
	}

	printf("\rtftp server: listening on %d\n", ntohs(server.sin_port));

	// Get first true request
	c_len = sizeof(client);

	// Get first true request
	while (1) {

	}
close_all:

	Sleep(10);

	if (sock) {
		closesocket(sock);
		sock = 0;
	}
	WSACleanup();

	// Return statistics
	if (code == 0) {
		stat->blocks = block_number;
		stat->errors = errors_number;
		return 0;
	}

	stat->blocks = 0;
	stat->errors = 0;
	return error_str[code];
}

	// ---  This is the main function that works with data and has a small interactive menu.
int main(void) {
	int time = 0;

	char dir[256] = { 0 };

	char client_ip[16] = { 0 };
	char server_ip[16] = { 0 };

	char* result = 0;
	int x = -1;

	// ---  While loop for the interactive menu
	while (x != 0) {
		printf("\tWelcome to TFTP serverby N00rd1!\n\n");
		printf("Choose the operation from list:\n");
		printf("1. Start a TFTP machine\n");
		printf("2. Print a configuration\n");
		printf("3. Change a configuration\n");
		printf("4. Use a default configuration\n");
		printf("0. Exit from programm\n\n");

		scanf_s("%d", &x);

		// Switch statement to execute the selected operation
		switch (x) {
		case 0:
			printf("\nThank you for using a N00rd1 software! Have a good day!");
			Sleep(1500);
			exit(0);

		case 1:
			if (strlen(dir) < 4 || strlen(client_ip) < 7 || strlen(server_ip) < 7 || time < 0) {
				printf("\tValues is empty!!");
			}
			else {
				result = tftp_server(time, dir, client_ip, server_ip, &stat);
				printf("\n%s", result);
			}

			system("pause");
			break;

		case 2:
			printf("\nSettings is a:");
			printf("\n\tFile path: %s", dir);
			printf("\n\tTimeout: %d", time);
			printf("\n\tClient ip4: %s", client_ip);
			printf("\n\tServer ip4: %s\n", server_ip);
			system("pause");
			break;

		case 3:
			printf("\nFile path: ");
			scanf("%s", dir);
			printf("\nTimeout: ");
			scanf("%d", &time);
			printf("\nClient ip4: ");
			scanf("%s", client_ip);
			printf("\nServer ip4: ");
			scanf("%s", server_ip);
			break;

		case 4:
			strcpy(dir, DIR_OF_FILE);
			time = DEF_TIMEOUT;
			strcpy(client_ip, CLIENT_DEF_IP4);
			strcpy(server_ip, SERVER_DEF_IP4);
			break;

		default:
			printf("\n\n\tSomething wrong!");
			system("pause");
			break;
		}

		system("cls");			// clear the console window
	}
}