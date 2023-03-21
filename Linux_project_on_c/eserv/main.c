#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

#define UDP_PORT 	9000			// UDP_PORT: the port number to be used for the UDP socket
#define MTU 		1500			// MTU: the maximum transmission unit size for the packet
#define HEADER_IP 	20				// HEADER_IP: the size of the IP header for the packet
#define HEADER_UDP 	8				// HEADER_UDP: the size of the UDP header for the packet
#define HEADER_ETH 	14				// HEADER_ETH: the size of the Ethernet header for the packet
#define MAX_CLIENTS 4				// MAX_CLIENTS: the maximum number of clients that can be tested

//#define DEBUG 1

// T_SpeedTest: a struct containing the necessary parameters for the speedtest
struct T_SpeedTest {
	char* server_ip;
	int packet_cnt;
	int mtu_data;
	int clients_cnt;
	char* client_ip[MAX_CLIENTS];
	struct timeval timeout;

	unsigned	send[MAX_CLIENTS];
	unsigned	drop[MAX_CLIENTS];
	unsigned	recv[MAX_CLIENTS];
	unsigned	half_bitrate[MAX_CLIENTS];
	unsigned	time[MAX_CLIENTS];
};

// T_packet: a struct representing a single packet to be sent and received
struct T_packet {
	uint32_t cnt;
	uint32_t block;
	unsigned char buffer[MTU - (HEADER_UDP + HEADER_IP + 4 + 4)];
};

// Function to perform UDP speed test
int udp_speedtest(struct T_SpeedTest* st, int number) {
	int code = 0;
	int block = 0;
	int recvlen = 0;
	int index = 0;
	int block_size = 4;
	int mtu_send = st->mtu_data - HEADER_IP - HEADER_UDP;
	unsigned i = 0;
	unsigned recv = 0;
	unsigned drop = 0;
	//	unsigned count = 0;
	int sock = -1;

	struct T_packet send_d = { 0 };
	struct T_packet get_d = { 0 };
	struct pollfd fds;
	struct sockaddr_in si_me = { 0 };
	struct sockaddr_in si_other = { 0 };
	socklen_t c_len = sizeof(si_other);

	// If the MTU (maximum transmission unit) is less than or equal to zero, return an error code of 1
	if (mtu_send <= 0)
		return 1;

	// Create a socket for communication using the UDP protocol
	// If the socket creation fails, return an error code of 3
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("socket creation failed\n");
		code = 3;
		goto close_all;
	}

	// Set the socket to be non-blocking
	// If changing the flag of the socket fails, return an error code of 4
	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
		printf("change flag of socket failed\n");
		code = 4;
		goto close_all;
	}

	// Set the values of the si_me and si_other structs to represent the sender and receiver addresses
	// If using a bad IP address, return an error code of 1
	memset((char*)&si_me, 0, c_len);

	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(UDP_PORT);
	if (inet_aton("192.168.1.1", &si_other.sin_addr) == 0) {
		printf("use bad ip addres\n");
		code = 1;
		goto close_all;
	}


	// structing a some information
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(UDP_PORT);
	if (inet_aton("192.168.1.2", &si_me.sin_addr) == 0) {
		printf("use bad ip addres\n");
		code = 1;
		goto close_all;
	}

	// Bind the socket to the si_me address
	// If binding the socket fails, return an error code of 5
	if (bind(sock, (struct sockaddr*)&si_me, c_len) != 0) {
		printf("bind failed\n");
		code = 5;
		goto close_all;
	}

	// Fill the send_d buffer with random unsigned characters
	for (i = 0; i < sizeof(send_d.buffer); i++) {
		send_d.buffer[i] = (unsigned char)rand();
	}

	// While the number of sent packets is less than the total number of packets to be sent
	while (st->send[number] < st->packet_cnt) {
		send_d.cnt = 0;
		index = 0;

		// Set the block size to the remaining packets if it is less than the specified block size
		if ((st->packet_cnt - st->send[number]) < block_size) {
			block_size = st->packet_cnt - st->send[number];
		}

		// For each packet in the block
		for (i = 0; i < block_size; i++) {
			// Send the packet using the sendto function
			// If sending the packet fails, close the socket and return an error code of 4
			send_d.block = block;
			if (sendto(sock, (char*)&send_d, mtu_send, 0, (const struct sockaddr*)&si_other, c_len) > 0) {
				st->send[0]++;
				send_d.cnt++;
			}
			else {
				if (sock)
					close(sock);
				return 4;
			}
		}

		drop = 0;
		recv = 0;

		while ((drop + recv) < block_size) {
			// Use the poll function to check for incoming data
			switch (poll(&fds, 1, 1500)) {
			case -1:
				printf("poll error \n");
				code = 6;
				goto close_all;
				break;
			case 0:
				printf("timeout: block %d: index=%d\n", block, index);
				drop += (block_size - (recv + drop));
				break;

			default:	// echo server function
				// If data is available to be read, receive it using the recvfrom function
				if (fds.revents & POLLIN) {
					memset(&get_d, 0, sizeof(struct T_packet));

					if (0 < (recvlen = recvfrom(sock, (char*)&get_d, sizeof(struct T_packet), 0, (struct sockaddr*)&si_other, &c_len))) {
						// Check if the received data corresponds to the expected packet
						if (get_d.cnt == index) {
							recv++;
						}
						else
							if (get_d.cnt > index) {
								printf("problems block %d: %u > %u, recvlen = %d !!!\n", block, get_d.cnt, index, recvlen);
								//return 0;
								drop += (get_d.cnt - index);
								recv++;
							}
							else {
								printf("Problems! \n");
								drop++;
								get_d.cnt = index;
							}
						index = get_d.cnt + 1;
					}
					else {}
				}
				fds.revents = 0;
			}
			continue;
			break;
		}
		// Update statistics
		st->drop[number] += drop;
		st->recv[number] += recv;
		block++;
	}

close_all:
	// check socket for open and close if need
	if (sock != 0)
		close(sock);
	printf("code %d		", code);
	return code;
}

int main(int argc, char* argv[]) {
	struct T_SpeedTest stest = { 0 };
	int code = 0;

	// Repeat the test 100 times
	for (int j = 0; j < 100; j++) {
		memset(&stest, 0, sizeof(stest));
		stest.server_ip = "192.168.1.1";			// Set the IP address of the server to test
		stest.packet_cnt = 1500;					// Set the number of packets to send
		stest.mtu_data = 1500;						// Set the maximum transmission unit size
		stest.clients_cnt = 1;						// Set the number of clients to use
		stest.client_ip[0] = "192.168.1.2";			// Set the IP address of the client to use		

		// Set the timeout for each test
		stest.timeout.tv_sec = 1;
		stest.timeout.tv_usec = 0;

		// Run the speed test and get the error code
		if ((code = udp_speedtest(&stest, 0)) == 0) {
			printf("packets %u || drops %u\n", stest.recv[0], stest.drop[0]);
		}
		else {
			printf("problem err code %u\n", code);
			printf("packets %u || drops %u\n", stest.recv[0], stest.drop[0]);
		}
	}
	return 0;
}
