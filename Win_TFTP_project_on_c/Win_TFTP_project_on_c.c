// TFTP (IP4) server with UDP packets
// This server is designed to transfer files over the TFTP protocol (used inside a local network)
#define _CRT_SECURE_NO_WARNINGS		// Error with use a scanf
#pragma warning(disable: 4013)		// On 227 line is a error of inet_pton

#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>

#define DIR_OF_FILE		"C:\\tftp\\1.txt"		// default folder for tftp files
#define CLIENT_DEF_IP4	"192.168.1.1"			// default ip client
#define SERVER_DEF_IP4	"192.168.1.2"
#define DEF_TIMEOUT		50

#define BUFLEN			512						// max size of buffer
#define PORT_1			69
#define PORT_2			4259

// error code
char* error_str[] = {
	"OK",										// All ok
	"Conversion error",							// Error of conver from str ip4 to binary ip4
	"WSAStartup failed",
	"Socket function failed",
	"Bind failed",
	"Timeout of packet",
	"Error of waiting a packet",
	"Error of geting a request packet",
	"Error get a error packet",
	"",
	"Error of open a file descriptor",
	"",
	"",
	"Max of Errors packets"
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
int recvest_from_TimeOut_UDP(SOCKET socket, long sec) {
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
	WSADATA wsaData;				

	int cnt = 0;								// Variable to save an answer of requests
	int code = 0;
	int sock = 0;								// Socket for get a request and socket for send a request
	int timing = 0;		 						// Timer, buffer, close

	int dlen = 0;
	int errors_number = 0;						// Variable to check the number of errors
	int c_len = 0;

	uint8_t data[BUFLEN];						// Buffer for data
	uint16_t block_number = 0;					// Block number

	// Structures
	struct sockaddr_in server = { 0 };			// Server ip address
	struct sockaddr_in client = { 0 };          // Real client ip address
	struct sockaddr_in client_tmp = { 0 };		// Temp to save a binary ip address of client IP4

	// Set server IP address
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT_1);
	if (inet_pton(AF_INET, server_ip, &server.sin_addr) == 0) {
		code = 1;
		goto close_all;							//all close
	}

	if (inet_pton(AF_INET, client_ip, &client_tmp.sin_addr) == 0) {
		code = 1;
		goto close_all;							//all close
	}

	// Structures for message
	tftp_message get_m = { 0 };					// Structures for incoming message
	tftp_message send_m = { 0 };				// Structures for outgoing message

	// Initialize Winsock
	if ((cnt = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		code = 2;
		goto close_all;							// Close everything
	}

	// Create a SOCKET for listening for incoming connection requests
	if ((sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		//printf("socket function failed with error: %u\n", WSAGetLastError());
		code = 3;
		goto close_all;							// Close everything
	}

	// Bind the socket
	if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		//printf("Bind failed with error code : %d", WSAGetLastError());
		code = 4;
		goto close_all;							// Close everything
	}

	printf("\n\rtftp server: listening on %d\n", ntohs(server.sin_port));

	// Get first true request
	c_len = sizeof(client);

	// Get first true request
	while (1) {
		timing = recvest_from_TimeOut_UDP(sock, time);

		switch (timing) {

			//----------------------------------------------------------------------
			// A timeout is coming
		case 0:
			code = 5;
			goto close_all;						// Close everything
			break;

			//----------------------------------------------------------------------
			// An error has occurred
		case -1:
			code = 6;
			goto close_all;						// Close everything
			break;
			
			//----------------------------------------------------------------------
			// Everything is ok, let's start
		default:
			if (errors_number > 13) {
				code = 13;
				goto close_all;					// Close everything
			}

			// Try to receive some data, this is a blocking call
			if ((cnt = recvfrom(sock, (char*)&get_m, BUFLEN, 0, (struct sockaddr*)&client, &c_len)) == SOCKET_ERROR) {
				code = 7;
				goto close_all;					// Close everything
			}

			// Comparison of theoretical ip with real ip, if it's not the client's IP, then wait for a new request
			if (client.sin_addr.s_addr != client_tmp.sin_addr.s_addr) {
				errors_number++;
				continue;
			}

			// Check for Error code and if it's an error, then close the app
			if (ntohs(get_m.opcode) == ERROR) {
				tftp_send_error_msg(sock, 0, (char*)"get a request with ERROR code", &client, c_len);
				code = 8;
				goto close_all;					// Close everything
			}

			// Check for first request
			if (ntohs(get_m.opcode) == RRQ && block_number == 0) {
				// Check the size of the received packet
				if (cnt < 9) {
					continue;
				}

				// Check the file and if the file is valid, send the first packet
				if ((fd = file_open_by_request(&get_m, cnt, client, sock, dir)) == NULL) {
					code = 11;
					errors_number++;
					continue;
				}

				// Rebind a socket to new port
				closesocket(sock);
				server.sin_port = htons(PORT_2);

				if ((sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
					code = 3;
					goto close_all;				// Close everything
				}

				// Bind the socket.
				if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
					code = 4;
					goto close_all;				// Close everything
				}
			}

			//--------------------------------------------------------------
			// Check for incorrect ACK
			if (ntohs(get_m.opcode) == ACK) {

				// Check if block number in ACK is correct
				if (ntohs(get_m.ack.block_number) == block_number - 1) {

					// Resend last packet and increment error count
					if ((cnt = sendto(sock, (char*)&send_m, 4 + dlen, 0, (struct sockaddr*)&client, c_len)) < 0) {
						code = 9;
						goto close_all;
					}

					errors_number++;
					continue;
				}

				// Check if ACK block number is incorrect
				if (ntohs(get_m.ack.block_number) != block_number) {
					// Send error message and close connection
					tftp_send_error_msg(sock, 5, (char*)"invalid ack number", &client, c_len);
					code = 10;
					goto close_all;				// Close everything
				}

				// Check if all data has been sent and close connection
				if (dlen < BUFLEN && block_number > 0) {
					if (fclose(fd))
						code = 13;
					else
						code = 0;
					goto close_all;
				}
			}

			//--------------------------------------------------------------
			// Read next block of data from file
			dlen = (int)(fread(send_m.data.data, 1, sizeof(data), fd));

			// Check for errors while reading from file
			if ((dlen != sizeof(data)) && (ferror(fd))) {
				tftp_send_error_msg(sock, 6, (char*)"Error of opening file", &client, c_len);
				code = 11;
				goto close_all;
			}

			// Send data packet with next block number
			block_number++;

			send_m.opcode = htons(DATA);
			send_m.data.block_number = htons(block_number);

			// Send data packet to client
			if ((cnt = sendto(sock, (char*)&send_m, 4 + dlen, 0, (struct sockaddr*)&client, c_len)) < 0) {
				code = 9;
				goto close_all;
			}
		}
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