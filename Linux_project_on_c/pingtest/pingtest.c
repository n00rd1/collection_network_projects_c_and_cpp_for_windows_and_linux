#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <linux/icmp.h>
#include <netinet/ip.h>

#define REVISION "220121"

#define CLOSE_AND_EXIT(ret, text){					\
					if (sock)			\
						close(sock);		\
						printf(text);		\
						return ret;		\
				}

struct msg_icmp {
	uint8_t 	type;				// message type [8]
	uint8_t 	code;				// type sub-code [0]
	uint16_t 	checksum;			// hash-sum message
	uint16_t	id;				// PID
	uint16_t	sequence;
	unsigned char buffer[1472];	// (MTU_MAX - IP - ICMP_HDR) //1500 - (20 +8)
};

struct msg_icmp_get {
	struct iphdr ip;
	struct msg_icmp msg;
};

static int usage(const char *progname) {
	printf("Usage: %s <bind_ip> <remote_ip> <number_icmp> <number_block> <mtu> [<timeout>]\n", progname);
	printf("bind_ip			locale ip address\n");
	printf("remote_ip		destonation ip address\n");
	printf("number_icmp		one packet with message\n");
	printf("number_block		all packets\n");
	printf("mtu			data size of one message\n");
	printf("timeout			timeout, default 1 second\n");
	return 1;
}

static unsigned short checksum(void *buffer, int len) {
	unsigned short result = 0;
	unsigned short *buf = buffer;
	unsigned int sum = 0;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;

	if (len == 1)
		sum += *(unsigned char *)buf;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

unsigned timer_ms() {
	struct timeval t;
	gettimeofday(&t, NULL);
	unsigned ms = ((long)t.tv_sec * 1000) + ((long)t.tv_usec / 1000);
	return ms;
}

int main (int argc, char *argv[]) {
	int sock = 0;
	int recvlen = 0;
	const int val = 128;	// TTL value

	uint16_t save_d[65536] = { 0 };

	unsigned i = 0;
	unsigned timeout = 0;
	unsigned mtu_data = 0;
	unsigned block_cnt = 0;
	unsigned block_size = 0;

	unsigned index = 0;
	unsigned packets = 0;

	struct pollfd fds = { 0 };
	struct msg_icmp send_d = { 0 };
	struct msg_icmp_get get_d = { 0 };

	struct icmp_filter filt = { 0 };
	struct sockaddr_in si_me = { 0 };
	struct sockaddr_in si_other = { 0 };
	socklen_t c_len = sizeof(si_other);

	// statistick
	unsigned attemp = 0;
	unsigned time = 0;
	unsigned bits_speed = 0;
	unsigned send_all = 0;
	unsigned recv_all = 0;
	unsigned drop_all = 0;

	// <local_ip> <dest_ip> <pack> <number> <mtu> [<timeoout>]
	if (argv[1] == 0 || argv[2] == 0 || argv[3] == 0 || argv[4] == 0 || argv[5] == 0) {
		return usage(argv[0]);
	}

	// check local ip
	if (inet_aton((const char *) argv[1], &si_me.sin_addr) == 0) {
		CLOSE_AND_EXIT(1, "use bad locale ip addres\n");
	}
	else {
		si_me.sin_family = AF_INET;
		si_me.sin_port = 0;
	}

	// check dest ip
	if (inet_aton((const char *) argv[2], &si_other.sin_addr) == 0) {
		CLOSE_AND_EXIT(1, "use bad destanation ip addres\n");
	}
	else {
		si_other.sin_family = AF_INET;
		si_other.sin_port = 0;
	}

	// check packet - how many message will be send on 1 loop
	block_size = atoi(argv[3]);
	if ((block_size == 1) || (block_size > 101)) {
		CLOSE_AND_EXIT(1, "One loop too little or too small (1 > block > 100)\n");
	}

	// check number - how many message will be send
	block_cnt = atoi(argv[4]);
	if (1 > block_cnt || (block_cnt * block_size) > 65534) {
		CLOSE_AND_EXIT(1, "Value of messages too little (1 < (packets * block_size) < 65534)\n");
	}

	// check size message
	mtu_data = atoi(argv[5]);
	if (36 > mtu_data || mtu_data > 1500) {
		CLOSE_AND_EXIT(1, "Value of messages too little (36 < mtu < 1500)\n");
	}
	else {
		mtu_data = mtu_data - 20;
	}

	// check number - how many message will be send
	if (argc == 6 && argv[6] == 0) {
		timeout = 10;
	}
	else {
		timeout = atoi(argv[6]);
	}

//	printf("block_size = %d, block_cnt = %d, mtu_data = %d, timeout = %d\n", block_size, block_cnt, mtu_data, timeout);
	setpriority(PRIO_PROCESS, 0, -20);

	for (i = 0; i < sizeof(send_d.buffer); i++) {
		send_d.buffer[i] = (unsigned char)rand();
	}

	if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		CLOSE_AND_EXIT(2, "Create socket failed\n");
	}

	if (setsockopt(sock, SOL_IP, IP_TTL, &val, sizeof(val)) != 0) {
		CLOSE_AND_EXIT(2, "Create socket failed\n");
	}

	if (fcntl(sock, F_SETFL, SOCK_NONBLOCK) != 0) {
		CLOSE_AND_EXIT(2, "Failed to create a nonblockin mode\n");
	}

	if (bind(sock, (struct sockaddr *)&si_me, c_len) != 0) {
		CLOSE_AND_EXIT(3, "bind failed\n");
	}

	filt.data = ~((1<<ICMP_ECHOREPLY));
	if (setsockopt(sock, SOL_RAW, ICMP_FILTER, (char*)&filt, sizeof(filt)) == -1) {
		printf("WARNING: setsockopt(ICMP_FILTER)");
	}

	fds.fd = sock;
	fds.events = POLLIN;

	send_d.type = 8;
	send_d.code = 0;
	send_d.id = 0x0101;
	send_d.sequence = 0;
	send_d.checksum = 0;

	send_d.checksum = checksum(&send_d, mtu_data);
	if (sendto(sock, &send_d, mtu_data, 0, (struct sockaddr *) &si_other, c_len) < 0) {
		CLOSE_AND_EXIT(4, "Error send first ICMP");
	}

	for (i = 0; i < 1; ) {
		switch (poll (&fds, 1, 1500)) {
				case -1:	CLOSE_AND_EXIT(5, "Error when use poll");
				case 0:		CLOSE_AND_EXIT(6, "Error when send first ICMP");
				default:
						if (fds.revents & POLLIN) {
							if ((recvlen = recvfrom(sock, &get_d, sizeof(get_d), 0, (struct sockaddr *) &si_other, &c_len)) > 0) {
								if (get_d.msg.id != send_d.id) {
									continue;
								}
								i++;
								break;
							}
						}
			}
	}

	send_d.type = 8;
	send_d.code = 0;
	send_d.id = getpid();
	send_d.sequence = 0;
	send_d.checksum = 0;
	packets = block_cnt * block_size;

	for (i = 0; i < 65536; i++) {
		send_d.sequence = i;
		save_d[i] = checksum(&send_d, mtu_data);
	}

	send_d.sequence = 0;
	send_d.checksum = 0;
	index = 0;

	time = timer_ms();

	while(send_all < packets) {
		for (i = 0; i < block_size; i++) {
			send_d.checksum = save_d[send_d.sequence];
			if (sendto(sock, &send_d, mtu_data, 0, (struct sockaddr *) &si_other, c_len) > 0) {
				send_all++;
				send_d.sequence++;
			} else {
				CLOSE_AND_EXIT(7, "Error on send data");
			}
		}

		if (send_d.sequence == packets) {
			i = send_d.sequence;
			send_d.sequence = 65535;
			send_d.checksum = save_d[send_d.sequence];
			if (!(sendto(sock, &send_d, mtu_data, 0, (struct sockaddr *) &si_other, c_len) > 0)) {
				CLOSE_AND_EXIT(7, "Error on send data");
			}
			send_d.sequence = i;
			block_size = packets - recv_all + 1;
		}

		attemp = block_size;

		while (attemp != 0) {
			switch (poll (&fds, 1, timeout)) {
			case -1:	CLOSE_AND_EXIT(5, "Error when use poll");
			case 0:		attemp--;
					break;
			default:
					if (fds.revents & POLLIN) {
						if ((recvlen = recvfrom(sock, &get_d, sizeof(get_d), 0, (struct sockaddr *) &si_other, &c_len)) <= 0) {
							continue;
						}

						if (get_d.msg.id != send_d.id) {
							continue;
						}

						if (get_d.msg.sequence == 65535) {
							attemp = 0;
							break;
						}
						
						if (get_d.msg.sequence != index) {
							if (get_d.msg.sequence > index) {
								drop_all += get_d.msg.sequence - index;
							}
							else {
								drop_all--;
							}
						}
						index = get_d.msg.sequence + 1;
						recv_all++;
						attemp--;
					}
					break;
			}
		}
	}
	time = timer_ms() - time;

	if (time == 0)
		time++;

	if (recv_all != 0)
		bits_speed = (unsigned long long)recv_all * (mtu_data + 20 + 14) * 8 / time;
	else
		bits_speed = 0;

	if (bits_speed > 1000000)
		bits_speed = 1000000;

	printf("report:\n\tsend icmp: %d\n\treceived icmp: %d\n\tmissed icmp: %d\n\ttime: %u\n\tbitrate: %u\n", send_all, recv_all, drop_all, time, bits_speed);
	return 0;
}
