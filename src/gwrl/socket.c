
#include "gwrl/socket.h"

#ifdef __cplusplus
extern "C" {
#endif

socklen_t gwsk_sockaddr_size(struct sockaddr_storage * addr) {
	sa_family_t fam = addr->ss_family;
	if(fam == AF_INET) return sizeof(struct sockaddr_in);
	else if(fam == AF_INET6) return sizeof(struct sockaddr_in6);
	return 0;
}

int gwsk_sockaddr_port(struct sockaddr_storage * addr) {
	sa_family_t fam = addr->ss_family;
	if(fam == AF_INET) return ntohs(((struct sockaddr_in *)addr)->sin_port);
	else if(fam == AF_INET6) return ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
	return 0;
}

void gwsk_sockaddr_print(struct sockaddr_storage * addr) {
	void * naddr = NULL;
	char buf[INET6_ADDRSTRLEN];
	sa_family_t fam = addr->ss_family;
	if(fam == AF_INET) naddr = &(((struct sockaddr_in *)addr)->sin_addr);
	else if(fam == AF_INET6) naddr = &(((struct sockaddr_in6 *)addr)->sin6_addr);
	inet_ntop(fam,naddr,buf,INET6_ADDRSTRLEN);
	fprintf(stdout,"gwsk_sockaddr_print: %s:%i\n",buf,gwsk_sockaddr_port(addr));
}

void gwsk_addrinfo_print(struct addrinfo * addr) {
	gwsk_sockaddr_print((struct sockaddr_storage *)addr->ai_addr);
}

bool gwsk_startup() {
	#if defined(PLATFORM_WINDOWS)
		WSADATA WsaData;
		return (WSAStartup(MAKEWORD(2,2),&WsaData) == 0);
	#endif
	return true;
}

bool gwsk_cleanup() {
	#if defined(PLATFORM_WINDOWS)
		return (WSACleanup() == 0);
	#endif
	return true;
}

int gwsk_connect(sockid_t sockfd, struct sockaddr_storage * addr) {
	int res = 0;
	
	#if defined(PLATFORM_WINDOWS)
		int errnm = 0;
	#endif
	
	while(1) {
		errno = 0;
		res = connect(sockfd,(struct sockaddr *)addr,gwsk_sockaddr_size(addr));
		if(res == 0) break;
		#if defined(PLATFORM_WINDOWS)
			if(res == SOCKET_ERROR) {
				res = -1;
				errnm = WSAGetLastError();
				if(errnm == WSAEWOULDBLOCK) break;
				if(errnm == WSAEINTR) continue;
			}
		#else
			if(errno == EINTR) continue;
			if(errno == EINPROGRESS) break;
		#endif
	};

	return res;
}

sockid_t gwsk_accept(sockid_t sockfd, struct sockaddr_storage * addr, socklen_t * asize) {
	sockid_t res = 0;
	
	#if defined(PLATFORM_WINDOWS)
	int errnm = 0;
	#endif
	
	while(1) {
		errno = 0;
		res = accept(sockfd,(struct sockaddr *)addr,asize);
		if(res == 0) break;
		
		#if defined(PLATFORM_WINDOWS)
			if(res == INVALID_SOCKET) {
				res = -1;
				errnm = WSAGetLastError();
				if(errnm == WSAEINTR) continue;
			}
		#else
			if(errno == EINTR) continue;
		#endif
	}

	return res;
}

void gwsk_close(sockid_t sockfd) {
	#if defined(PLATFORM_WINDOWS)
		closesocket(sockfd);
	#else
		close(sockfd);
	#endif
}

int gwsk_nonblock(sockid_t sockfd, int onoff) {
	#if defined(PLATFORM_WINDOWS)
		DWORD nonBlocking = 1;
		return ioctlsocket(sockfd,FIONBIO,&nonBlocking);
	#else
		return fcntl(sockfd,F_SETFL,O_NONBLOCK,&onoff);
	#endif
}

int gwsk_reuseaddr(sockid_t sockfd, int onoff) {
	#if defined(PLATFORM_WINDOWS)
		return setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const char*)&onoff,sizeof(onoff));
	#else
		return setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&onoff,sizeof(onoff));
	#endif
}

void skctlinfo_free(skctlinfo * info, bool freeinfo) {
	if(info->list) freeaddrinfo(info->list);
	info->list = NULL;
	if(freeinfo) free(info);
}

char * skctl_str_for_flag(int flag) {
	if(flag == SKCTL_GETADDRINFO) {
		return "SKCTL_GETADDRINFO";
	} else if(flag == SKCTL_SOCKET) {
		return "SKCTL_SOCKET";
	} else if(flag == SKCTL_CONNECT) {
		return "SKCTL_CONNECT";
	} else if(flag == SKCTL_BIND) {
		return "SKCTL_BIND";
	} else if(flag == SKCTL_LISTEN) {
		return "SOCKCNTL_LIST";
	} else if(flag == SKCTL_REUSE_ADDR) {
		return "SKCTL_REUSE_ADDR";
	} else if(flag == SKCTL_PRINTADDR) {
		return "SKCTL_PRINTADDR";
	} else if(flag == SKCTL_NOBLOCK) {
		return "SKCTL_NOBLOCK";
	}
	return "Unknown flag";
}

int skctl(skctlinfo * arg) {
	bool addr_existed = false;
	int res = 0;
	struct addrinfo * ap = NULL;
	struct addrinfo * info = NULL;

	if(arg->flags == 0) {
		arg->serrno = 0;
		fprintf(stderr,"skctl error: arg->flags is 0.\n");
		return -1;
	}

	if(arg->backlog <= 0) arg->backlog = 25;

	if(arg->used) {
		ap = arg->used;
		addr_existed = true;
	}

	if(!addr_existed && (arg->flags & SKCTL_GETADDRINFO)) {
		while(1) {
			arg->serrnoflag = 0;
			arg->serrno = 0;
			res = getaddrinfo(arg->node,arg->service,&(arg->hints),&info);
			if(res == 0) break;

			#if defined(PLATFORM_WINDOWS)
				if(res == WSATRY_AGAIN) continue;
			#else
				if(res == EAI_AGAIN) continue;
				if(res == EAI_SYSTEM && errno == EINTR) continue;
			#endif

			fprintf(stderr,"skctl: getaddrinfo error: %s\n",gai_strerror(res));
			arg->serrno = res;
			arg->serrnoflag = SKCTL_GETADDRINFO;
			return -1;
		}
		arg->list = info;
	}

	if(!arg->list) {
		fprintf(stderr,"skctl error: no addrinfo list.");
		return -1;
	}

	if(!ap) ap = info;

	#if !defined(PLATFORM_WINDOWS)
		#define LOOP_ENTRY_CLOSE(flag)\
			if(res < 0) {\
				gwsk_close(arg->sockfd);\
				arg->sockfd = -1;\
				arg->serrno = errno;\
				arg->serrnoflag = flag;\
				goto nextap;\
			}\

	#endif

	#if defined(PLATFORM_WINDOWS)
		#define LOOP_ENTRY_CLOSE_WINDOWS(flag)\
			if(res == SOCKET_ERROR) {\
				gwsk_close(arg->sockfd);\
				arg->sockfd = 0;\
				arg->serrno = WSAGetLastError();\
				arg->serrnoflag = flag;\
				res = -1;\
				goto nextap;\
			}\

	#endif

	while(ap != NULL) {
		//res = 0;
		arg->serrnoflag = 0;
		arg->serrno = 0;
		arg->used = NULL;
		
		if(arg->flags & SKCTL_PRINTADDR) {
			gwsk_addrinfo_print(ap);
		}
		
		if(arg->flags & SKCTL_SOCKET) {
			#if defined(PLATFORM_WINDOWS)
				if(arg->flags & SKCTL_NOBLOCK) {
					//socket() by default on windows creates non-blocking sockets.
					arg->sockfd = socket(ap->ai_family,ap->ai_socktype,ap->ai_protocol);
				} else {
					//have to use WSASocket() to get a blocking socket.
					arg->sockfd = WSASocket(ap->ai_family,ap->ai_socktype,ap->ai_protocol,NULL,0,0);
				}
			#else
				arg->sockfd = socket(ap->ai_family,ap->ai_socktype,ap->ai_protocol);
			#endif
			
			#if defined(PLATFORM_WINDOWS)
				if(arg->sockfd == INVALID_SOCKET) {
					res = -1;
					arg->serrno = WSAGetLastError();
					arg->serrnoflag = SKCTL_SOCKET;
					goto nextap;
				}
			#else
				if(arg->sockfd < 0) {
					res = -1;
					arg->serrno = errno;
					arg->serrnoflag = SKCTL_SOCKET;
					goto nextap;
				}
			#endif
		}
		
		if(arg->flags & SKCTL_NOBLOCK) {
			#if !defined(PLATFORM_WINDOWS)
				res = gwsk_nonblock(arg->sockfd,1);
				#if !defined(PLATFORM_WINDOWS)
					LOOP_ENTRY_CLOSE(SKCTL_NOBLOCK);
				#endif
			#endif
		}
		
		if(arg->flags & SKCTL_REUSE_ADDR) {
			res = gwsk_reuseaddr(arg->sockfd,1);
			#if defined(PLATFORM_WINDOWS)
				LOOP_ENTRY_CLOSE_WINDOWS(SKCTL_REUSE_ADDR)
			#else
				LOOP_ENTRY_CLOSE(SKCTL_REUSE_ADDR);
			#endif
		}

		if(arg->flags & SKCTL_BIND) {
			res = bind(arg->sockfd,ap->ai_addr,gwsk_sockaddr_size((struct sockaddr_storage *)ap->ai_addr));
			#if defined(PLATFORM_WINDOWS)
				LOOP_ENTRY_CLOSE_WINDOWS(SKCTL_BIND);
			#else
				LOOP_ENTRY_CLOSE(SKCTL_BIND);
			#endif
		}

		if(arg->flags & SKCTL_LISTEN) {
			res = listen(arg->sockfd,arg->backlog);
			#if defined(PLATFORM_WINDOWS)
				LOOP_ENTRY_CLOSE_WINDOWS(SKCTL_LISTEN);
			#else
				LOOP_ENTRY_CLOSE(SKCTL_LISTEN);
			#endif
		}

		if(arg->flags & SKCTL_CONNECT) {
			res = gwsk_connect(arg->sockfd,(struct sockaddr_storage *)ap->ai_addr);
			#if defined(PLATFORM_WINDOWS)
				if(res == SOCKET_ERROR) {
					int errnm = WSAGetLastError();
					if(errnm != WSAEWOULDBLOCK) {
						LOOP_ENTRY_CLOSE_WINDOWS(SKCTL_CONNECT);
					}
				}
			#else
				if(res < 0 && errno != EINPROGRESS) {
					LOOP_ENTRY_CLOSE(SKCTL_CONNECT);
				}
			#endif
		}

		res = 0;
		arg->used = ap;
		break;

	nextap:
		if(addr_existed) {
			arg->used = ap;
			break;
		}
		ap = ap->ai_next;
	}
	
	if(res < 0) {
		#if defined(PLATFORM_WINDOWS)
			gwprintsyserr("skctl error",arg->serrno);
		#else
			gwprintsyserr("skctl error",arg->serrno);
		#endif
		fprintf(stderr,"skctlinfo flag that caused the error: %s\n",skctl_str_for_flag(arg->serrnoflag));
	}

	return res;
}

int skctl_tcp_server(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock) {
	info->node = node;
	info->service = service;
	info->hints.ai_family = ipfamily;
	info->hints.ai_protocol = IPPROTO_TCP;
	info->hints.ai_flags = AI_PASSIVE;
	info->hints.ai_socktype = SOCK_STREAM;
	info->flags = SKCTL_TCP_SERVER;
	if(noblock) info->flags |= SKCTL_NOBLOCK;
	return skctl(info);
}

int skctl_udp_server(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock) {
	info->node = node;
	info->service = service;
	info->hints.ai_family = ipfamily;
	info->hints.ai_protocol = IPPROTO_UDP;
	info->hints.ai_flags = AI_PASSIVE;
	info->hints.ai_socktype = SOCK_DGRAM;
	info->flags = SKCTL_UDP_SERVER;
	if(noblock) info->flags |= SKCTL_NOBLOCK;
	return skctl(info);
}

int skctl_tcp_client(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock) {
	info->node = node;
	info->service = service;
	info->hints.ai_family = ipfamily;
	info->hints.ai_protocol = IPPROTO_TCP;
	info->hints.ai_socktype = SOCK_STREAM;
	info->flags = SKCTL_TCP_CLIENT;
	if(noblock) info->flags |= SKCTL_NOBLOCK;
	return skctl(info);
}

int skctl_udp_client(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock) {
	info->node = node;
	info->service = service;
	info->hints.ai_family = ipfamily;
	info->hints.ai_protocol = IPPROTO_UDP;
	info->hints.ai_socktype = SOCK_DGRAM;
	info->flags = SKCTL_UDP_CLIENT;
	if(noblock) info->flags |= SKCTL_NOBLOCK;
	return skctl(info);
}

int skctl_udp_connected_client(skctlinfo * info, char * node, char * service, int ipfamily, bool noblock) {
	info->node = node;
	info->service = service;
	info->hints.ai_family = ipfamily;
	info->hints.ai_protocol = IPPROTO_UDP;
	info->hints.ai_socktype = SOCK_DGRAM;
	info->flags = SKCTL_UDP_CONNECTED_CLIENT;
	if(noblock) info->flags |= SKCTL_NOBLOCK;
	return skctl(info);
}

#ifdef __cplusplus
}
#endif
