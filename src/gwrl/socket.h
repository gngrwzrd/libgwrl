
#define INCLUDE_ASSERT
#define INCLUDE_ERRNO
#define INCLUDE_STDIO
#define INCLUDE_STDLIB
#define INCLUDE_STDINT
#define INCLUDE_STDBOOL
#define INCLUDE_UNISTD
#define INCLUDE_STRING
#define INCLUDE_STRINGS
#define INCLUDE_ARPA_INET
#define INCLUDE_NETDB
#define INCLUDE_FCNTL
#define INCLUDE_WINDOWS
#define INCLUDE_WINSOCK2
#define INCLUDE_WS2TCPIP_H
#define INCLUDE_IPHLPAPI
#define TYPES_BOOL
#define TYPES_SOCKID_T
#define TYPES_SOCKLEN_T
#define TYPES_SA_FAMILY_T
#define TYPES_STRUCT_SOCKADDR_STORAGE
#include "shared_config.h"
#include "shared_include.h"
#include "shared_types.h"
#include "gwrl/string.h"

#ifndef GW_NET_H
#define GW_NET_H

#define SKCTL_GETADDRINFO (1<<0)
#define SKCTL_SOCKET      (1<<1)
#define SKCTL_CONNECT     (1<<2)
#define SKCTL_BIND        (1<<3)
#define SKCTL_LISTEN      (1<<4)
#define SKCTL_REUSE_ADDR  (1<<5)
#define SKCTL_PRINTADDR   (1<<6)
#define SKCTL_NOBLOCK     (1<<7)
#define SKCTL_TCP_CLIENT  (SKCTL_GETADDRINFO|SKCTL_SOCKET|SKCTL_CONNECT)
#define SKCTL_UDP_CONNECTED_CLIENT (SKCTL_GETADDRINFO|SKCTL_SOCKET|SKCTL_CONNECT)
#define SKCTL_UDP_CLIENT (SKCTL_GETADDRINFO|SKCTL_SOCKET)
#define SKCTL_TCP_SERVER (SKCTL_GETADDRINFO|SKCTL_SOCKET|SKCTL_BIND|SKCTL_LISTEN|SKCTL_REUSE_ADDR)
#define SKCTL_UDP_SERVER (SKCTL_GETADDRINFO|SKCTL_SOCKET|SKCTL_BIND|SKCTL_REUSE_ADDR)

#define _sockaddr(o) ((struct sockaddr *)o)
#define _sockaddrst(o) ((struct sockaddr_storage *)o)

typedef struct skctlinfo {
	/*
	//EXAMPLE:
	skctlinfo info;
	bzero(&info,sizeof(info));
	info.flags = SKCTL_TCP_CLIENT | SKCTL_PRINTADDR;
	info.node = "google.com" | "127.0.0.1" | "localhost";
	info.service = "echo" | "13";
	info.backlog = 100;
	info.hints.ai_family = AF_UNSPEC;
	info.hints.ai_protocol = IPPROTO_TCP;
	info.hints.ai_socktype = SOCK_STREAM;
	info.hints.ai_flags = AI_PASSIVE;
	if(skctl(&info) < 0) exit(-1);
	//use info.sockfd
	*/
	
	//these are for you to fill out before calling skctl.
	uint16_t flags;        //SKCTL_* flags
	struct addrinfo hints; //addrinfo hints
	char * node;           //ip or hostname
	char * service;        //port or service name
	int backlog;           //backlog for listen function
	
	//these are set and possibly available after calling gwsk_sockcntl.
	//depending on the return value of gwsk_sockcntl.
	struct addrinfo * list; //list used in addrinfo call. free with skctlinfo_free
	struct addrinfo * used; //the user addrinfo * when setting up a socket
	sockid_t sockfd;        //socket descriptor
	int serrno;             //system errno
	int serrnoflag;         //the ctlflag that caused an error
} skctlinfo;

//skctl
int skctl(skctlinfo * info);
int skctl_tcp_server(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock);
int skctl_udp_server(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock);
int skctl_tcp_client(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock);
int skctl_udp_client(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock);
int skctl_udp_connected_client(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock);
void skctlinfo_free(skctlinfo * info, bool freeinfo);

//miscelaneous functions
bool gwsk_startup();
bool gwsk_cleanup();
void gwsk_addrinfo_print(struct addrinfo * addr);
void gwsk_close(sockid_t sockfd);
int gwsk_connect(sockid_t sockfd, struct sockaddr_storage * addr);
sockid_t gwsk_accept(sockid_t sockfd, struct sockaddr_storage * addr, socklen_t * asize);
int gwsk_nonblock(sockid_t sockfd, int onoff);
int gwsk_reuseaddr(sockid_t sockfd, int onoff);
void gwsk_sockaddr_print(struct sockaddr_storage * addr);
int gwsk_sockaddr_port(struct sockaddr_storage * addr);
socklen_t gwsk_sockaddr_size(struct sockaddr_storage * addr);

#endif
