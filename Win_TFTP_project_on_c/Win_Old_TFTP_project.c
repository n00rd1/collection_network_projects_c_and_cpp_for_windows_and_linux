// TFTP server with UDP packets
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma warning(disable: 4267)
#pragma warning(disable: 4244)

#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>

#define PORT_1 69
#define PORT_2 4259
#define BUFLEN 512		// max size of buffer

// structure for statistic
struct stat {
	int blocks;
	int errors;
} stat;

// error code
const char* error_str[] = {
	"OK",						// All okey
	"err1",                     // WSAS broken
	"err2",
	"err3",
	"err4",
	"err5",
	"err6",
	"err7",
	"err8",
	"err9",
	"err10",
	"err11"
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


		// ---  send a error message to client  --- //
size_t tftp_send_error(int s, int error_code, char* error_string, struct sockaddr_in* sock, int slen) {
	tftp_message m;
	size_t c;

	if (strlen(error_string) >= BUFLEN) {
		fprintf(stderr, "server: tftp_send_error(): error string too long\n");
		return -1;
	}

	m.opcode = htons(ERR0R);
	m.error.error_code = error_code;
	strcpy((char*)m.error.error_string, error_string);

	if ((c = sendto(s, (char*)&m, 4 + strlen(error_string) + 1, 0, (struct sockaddr*)sock, slen)) < 0)
		perror("server: sendto()");

	return c;
}

// check request for file name and mode; after check file availability
FILE* file_open(tftp_message* m, int cnt, struct sockaddr_in client, int sock_output, char* dir) {
	char* mode, * tmp;
	char filename[256];
	FILE* fd;
	//----------------------------------------------------------------------
	// Parse client request

	//filename = (char*)m->request.filename_and_mode;
	tmp = (char*)m->request.filename_and_mode;

	snprintf(filename, sizeof(filename), "%s%s", (char*)dir, (char*)tmp);

	//----------------------------------------------------------------------
	// Try to find a file name on request
	int	i = cnt - 2;
	while (i--) {
		if (*tmp == 0)
			break;
		tmp++;
	}

	if (*tmp)
		tftp_send_error(sock_output, 3, (char *)"broken file", &client, sizeof(&client));				// no end of line

	if (i == 0)
		tftp_send_error(sock_output, 4, (char*)"not found mod transfer!", &client, sizeof(&client));	// no mode

	mode = tmp + 1;
	i--;

	//----------------------------------------------------------------------
	// Try to find a mode transfer on request
	while (i--) {
		if (*tmp == 0)
			break;
		tmp++;
	}

	if (*tmp)
		tftp_send_error(sock_output, 3, (char *)"broken file", &client, sizeof(&client));				// no end of line

	if (strcmp(mode, "octet") != 0)
		tftp_send_error(sock_output, 4, (char *)"error mode transfer!", &client, sizeof(&client));		// check mode for octet

	if ((fd = fopen(filename, "rb")) == NULL) {
		printf("cant's open file %s\n\n ", filename);
		tftp_send_error(sock_output, 5, (char *)"can't open a file!", &client, sizeof(&client));
		return NULL;
	}
	else
		return fd;

}

// time of taking request (timeout)    --- //
int recvfromTimeOutUDP(SOCKET socket, long sec) {
	//--------------------------------------------------------------------------
	// Setup timeval variable
	struct fd_set fds;
	struct timeval timeout;
	timeout.tv_sec = sec;

	//--------------------------------------------------------------------------
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


const char* tftp_server(char* dir, int time, char* server_ip, char* client_ip, struct stat* stat) {

	FILE* fd = 0;							// file descriptor

	int code = 0;
	int timing = 0;		 					// timer, buffer, close
	int sock = 0;							// socket for get a request and socket for send a request
	int cnt = 0;							// temp to save a answert of requests

	size_t dlen = 0;
	size_t errors_number = 0;				// temp to check the number of errors
	size_t c_len = 0;

	uint8_t data[BUFLEN];                   // Buffer for data
	uint16_t block_number = 0;              // Block number

	WSADATA wsaData;

	//--------------------------------------------------------------------------
	// Structures
	struct sockaddr_in server = { 0 };				// Server
	struct sockaddr_in client = { 0 };              // Real client ip address

	//--------------------------------------------------------------------------
	// Really ip server address
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(server_ip);
	server.sin_port = htons(PORT_1);

	//--------------------------------------------------------------------------
	// Structures for message
	tftp_message get_m = { 0 };
	tftp_message send_m = { 0 };

	//--------------------------------------------------------------------------
	// Initialize Winsock

	if ((cnt = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		printf("WSAStartup failed: %d\n", cnt);
		code = 1;
		goto close_all;     //all close
	}

	//--------------------------------------------------------------------------
	// Create a SOCKET for listening for incoming connection requests

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("socket function failed with error: %u\n", WSAGetLastError());
		code = 2;
		goto close_all;     //all close
	}

	//--------------------------------------------------------------------------
	// Bind the socket.

	if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
		code = 3;
		goto close_all;     //all close
	}

	printf("\rtftp server: listening on %d\n", ntohs(server.sin_port));

	c_len = sizeof(client);

	//--------------------------------------------------------------------------
	// Get first true request

	while (1) {
		timing = recvfromTimeOutUDP(sock, time);

		switch (timing) {

			//----------------------------------------------------------------------
			// Timeout is coming
		case 0:
			code = 4;
			goto close_all;     //all close
			break;
			//----------------------------------------------------------------------
			// error
		case -1:
			code = 5;
			goto close_all;     //all close
			break;
			//----------------------------------------------------------------------
			// ALL OKEY LEST sart
		default:
		{

			if (errors_number > 12) {
				code = 12;
				goto close_all;     //all close
			}

			//----------------------------------------------------------------------
			// Try to receive some data, this is a blocking call

			if ((cnt = recvfrom(sock, (char*)&get_m, BUFLEN, 0, (struct sockaddr*)&client, (int *)c_len)) == SOCKET_ERROR) {
				code = 6;
				goto close_all;     //all close
			}

			//----------------------------------------------------------------------
			// Comparison of theoretical ip with real ip , if it's not a client ip, when wait new request

			if (client.sin_addr.s_addr != inet_addr(client_ip)) {
				errors_number++;
				continue;
			}

			//----------------------------------------------------------------------
			// Check for Error code and if it's error, then close app

			if (ntohs(get_m.opcode) == ERROR) {
				tftp_send_error(sock, 0, (char*)"get a request with ERROR code", &client, c_len);
				code = 7;
				goto close_all;     //all close
			}

			//----------------------------------------------------------------------
			// Check for first request

			if (ntohs(get_m.opcode) == RRQ && block_number == 0) {

				//----------------------------------------------------------------------
				// Check the size of getting packet

				if (cnt < 9) {
					continue;
				}

				//----------------------------------------------------------------------
				// Check file and if file is valiable send first packet

				if ((fd = file_open(&get_m, cnt, client, sock, dir)) == NULL) {
					code = 10;
					errors_number++;
					continue;
				}

				//---------------------------------------------------------------------
				// Rebind a socket to new port

				closesocket(sock);

				server.sin_port = htons(PORT_2);

				if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
					code = 2;
					goto close_all;     //all close
				}

				//----------------------------------------------------------------------
				// Bind the socket.

				if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
					code = 3;
					goto close_all;     //all close
				}
			}

			//--------------------------------------------------------------
			// Check for incorrect ACK

			if (ntohs(get_m.opcode) == ACK) {

				// Check for incorrect block number
				if (ntohs(get_m.ack.block_number) == block_number - 1) {

					if ((cnt = sendto(sock, (char*)&send_m, 4 + dlen, 0, (struct sockaddr*)&client, c_len)) < 0) {
						code = 8;
						goto close_all;
					}

					errors_number++;
					continue;
				}

				if (ntohs(get_m.ack.block_number) != block_number) { // the ack number is too high or low
					tftp_send_error(sock, 5, (char*)"invalid ack number", &client, c_len);
					code = 9;
					goto close_all;         // all close;
				}

				if (dlen < BUFLEN && block_number > 0) {
					if (fclose(fd))
						code = 14;
					else
						code = 0;
					goto close_all;
				}

			}

			//--------------------------------------------------------------
			//Read next block of file and sending
			dlen = fread(send_m.data.data, 1, sizeof(data), fd);

			if ((dlen != sizeof(data)) && (ferror(fd))) {
				tftp_send_error(sock, 6, (char*)"Error of opening file", &client, c_len);
				code = 10;
				goto close_all;
			}

			// Send data
			block_number++;

			send_m.opcode = htons(DATA);
			send_m.data.block_number = htons(block_number);

			if ((cnt = sendto(sock, (char*)&send_m, 4 + dlen, 0, (struct sockaddr*)&client, c_len)) < 0) {
				code = 8;
				goto close_all;
			}
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

	if (code == 0) {
		stat->blocks = block_number;
		stat->errors = errors_number;
		return 0;
	}

	else {
		stat->blocks = 0;
		stat->errors = 0;
		return error_str[code];
	}
}

/*int main(void) {
	const char *res;
	char *dir = (char *)"C:\\tftp\\";

	char *client_ip = (char *)"192.168.1.1";
	char *server_ip = (char *)"192.168.1.2";

	int time = 50;                      // 50 sec timeout

	res = tftp_server(dir, time, server_ip, client_ip, &stat);
	printf("%s\n", res);
}*/
