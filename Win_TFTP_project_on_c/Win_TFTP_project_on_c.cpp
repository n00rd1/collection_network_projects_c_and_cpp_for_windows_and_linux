// TFTP (IP4) server with UDP packets
// Этот сервер предназначен для передачи файлов по протоколу TFTP (использовался внутри локальной сети)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>

#define BUFLEN			512						// max size of buffer
#define DIR_OF_FILE		"C:\\tftp\\1.txt"		// default folder for tftp files
#define CLIENT_DEF_IP4	"192.168.1.1"
#define SERVER_DEF_IP4	"192.168.1.2"
#define DEF_TIMEOUT		50

// error code
const char* error_str[] = {
	"OK",						// All ok
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

char* tftp_server(int time, char* dir, char* client_ip, char* server_ip) {

	FILE* fd = 0;
	WSADATA wsaData;

	return 0;
}


// работает с данными и имеет небольшое интерактивное меню.
int main(void) {
	int time = 0;

	char dir[256] = { 0 };

	char client_ip[16] = { 0 };
	char server_ip[16] = { 0 };

	char* result = 0;
	int x = -1;

	while (x != 0) {
		printf("\tWelcome to TFTP serverby N00rd1!\n\n");
		printf("Choose the operation from list:\n");
		printf("1. Start a TFTP machine\n");
		printf("2. Print a configuration\n");
		printf("3. Change a configuration\n");
		printf("4. Use a default configuration\n");
		printf("0. Exit from programm\n\n");

		scanf_s("%d", &x);
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
				result = tftp_server(time, dir, client_ip, server_ip);
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

		system("cls");
	}
}