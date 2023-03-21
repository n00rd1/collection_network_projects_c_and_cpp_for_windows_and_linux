#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <linux/icmp.h>
#include <netinet/ip.h>
//#include <netinet/ip_icmp.h>

#include "libtype.h"

#define PRINTF false

#define ERROR_DEBUG(x) {				\
					if (PRINTF) {		\
						printf(x);		\
						putchar('\n');	\
					}					\
					return 1;			\
				}

/*
#ifdef CAPABILITIES
extern int modify_capability(cap_value_t, cap_flag_value_t);
static inline int enable_capability_raw(void)		{ return modify_capability(CAP_NET_RAW,   CAP_SET);   };
static inline int disable_capability_raw(void)		{ return modify_capability(CAP_NET_RAW,   CAP_CLEAR); };
static inline int enable_capability_admin(void)		{ return modify_capability(CAP_NET_ADMIN, CAP_SET);   };
static inline int disable_capability_admin(void)	{ return modify_capability(CAP_NET_ADMIN, CAP_CLEAR); };
#else
*/
extern int modify_capability(int);
static inline int enable_capability_raw(void)		{ return modify_capability(1); };
static inline int disable_capability_raw(void)		{ return modify_capability(0); };
static inline int enable_capability_admin(void)		{ return modify_capability(1); };
static inline int disable_capability_admin(void)	{ return modify_capability(0); };
//#endif

void error_message(char *error_text) {
	printf("%s\n", error_text);
}

int main(int argc, char **argv) {
	struct sockaddr_in si_me = { 0 };
	struct sockaddr_in si_other = { 0 };
	socklen_t c_len = sizeof(si_other);

	char *buffer = 0;
	
	int socket_errno = 0;
	int icmp_sock = 0;
	int sock = 0;

	// check comand searching (ping)
//	if (strstr(argv[0], "ping") != 0) {
//		ERROR_DEBUG("Bad comand name");
//	}

	if (argc < 2) {			// (argv[1] == 0) {
		printf("adress: ");
		scanf("%s", buffer);
	}

/*
 *	У MikroTik лог выглядит приблизительно следующим образом:
 * 	[admin@MikroTik] > ping
 * 	address: 192.168.1.1
 * 	  SEQ HOST                                     SIZE TTL TIME  STATUS
 *		0                                                         no route to host
 *		1                                                         no route to host
 *      sent=2 received=0 packet-loss=100%
 *------------------------------------------------------------------------------------
 * 	Либо второй вариант вызова
 *	[admin@MikroTik] > ping 127.0.0.1   
 *	  SEQ HOST                                     SIZE TTL TIME  STATUS                                                                                                                                                                                                         
 *		0 127.0.0.1                                  56  64 0ms  
 *		1 127.0.0.1                                  56  64 0ms  
 *    sent=2 received=2 packet-loss=0% min-rtt=0ms avg-rtt=0ms max-rtt=0ms 
 **/

	// Enable raw sockets
	enable_capability_raw();
	
	// Validate a destanation address
	while(1) {

		if (valid_type_ip4addr(argv[1], &si_other)) {
			icmp_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
			break;
		}

	/*	У MikroTik проверка идёт на IPv4/MAC/IPv6 (последнее у нас вообще не реализовано)	*/
//		if (valid_type_mac(argc[1]))
//			break;

	/*	Проверки на host у MikroTik нет	*/
//		if (valid_type_hostname(argv[1], &si_other))
//			break;

	/*	Проверка IPv6 у MikroTik есть, а у нас нет	*/
//		if (valid_type_ip6addr(argc[1], &si_other)) {
//			icmp_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
//			break;
//		}

		ERROR_DEBUG("Unknown addr");
	}

	socket_errno = errno;
	disable_capability_raw();	

	/*
	 *	Initialization of local addr
	 **/

	 

	return 0;
}
