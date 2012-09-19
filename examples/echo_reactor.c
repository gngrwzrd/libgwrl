
#include "gwrl/event.h"

#define MAXLINE 1024

char * mode;
char * node;
char * service;
char * protocol;
bool istcp;
bool isserver;

gwrl * rl = NULL;
skctlinfo sockcntl;

int c = 0;
gwrlsrc * intsrc = NULL;

void interval_test(gwrl * rl, gwrlevt* evt) {
	//printf("interval_test\n");
	if(c == 10) gwrl_src_del(rl,intsrc,NULL,true);
	c++;
}

void client_socket_activity(gwrl * rl, gwrlevt * evt) {
	printf("client_socket_activity\n");
	if(evt->flags & GWRL_RD) {
		printf("GWRL_RD\n");
		char buf[MAXLINE];
		int amnt = read(evt->fd,buf,MAXLINE-1);
		if(amnt <= 0) {
			printf("disconnected\n");
			if(errno == ECONNRESET) {
				printf("ECONNRESET\n");
			}
			if(errno == ENOTCONN) {
				printf("ENOTCONN\n");
			}
			if(errno == ETIMEDOUT) {
				printf("ETIMEDOUT\n");
			}
			//shutdown(evt->fd,SHUT_RDWR);
			printf("closing fd: %i\n",evt->fd);
			close(evt->fd);
			gwrl_src_del(rl,evt->src,NULL,true);
			return;
		}
		write(evt->fd,buf,amnt);
	}
	else if(evt->flags & GWRL_WR) {
		printf("GWRL_WR!\n");
	} else {
		printf("!!!!!!!!!!!!!!!\n");
	}
}

void server_socket_activity(gwrl * rl, gwrlevt * evt) {
	printf("server_socket_activity\n");
	if(evt->flags & GWRL_RD) {
		printf("GWRL_RD\n");
		if(istcp) {
			struct sockaddr_storage sockaddr;
			socklen_t clsize = sizeof(sockaddr);
			int clientfd = gwsk_accept(evt->fd,&sockaddr,&clsize);
			gwrl_set_fd(rl,clientfd,GWRL_RD,&client_socket_activity,NULL);
		} else {
			char buf[MAXLINE];
			struct sockaddr_storage sockaddr;
			socklen_t socklen = sizeof(sockaddr);
			int res = recvfrom(evt->fd,buf,MAXLINE-1,0,(struct sockaddr *)&sockaddr,&socklen);
			if(res > 0) {
				gwsk_sockaddr_print(&sockaddr);
				int res2 = sendto(evt->fd,buf,res,0,(struct sockaddr *)&sockaddr,socklen);
				if(res2 < 0) {
					gwprintsyserr("sendto error",errno)
				}
			} else {
				gwprintsyserr("recvfrom error",errno)
			}
		}
	} else {
		printf("GWRL_BADF!!!!!!!!!!!!!!!!!!!!!!!\n");
	}
}

void setup_server() {
	sockcntl.node = NULL;
	sockcntl.service = service;
	sockcntl.flags |= SKCTL_NOBLOCK;
	sockcntl.hints.ai_family = AF_UNSPEC;
	sockcntl.hints.ai_flags = AI_PASSIVE;
	
	if(istcp) {
		sockcntl.hints.ai_protocol = IPPROTO_TCP;
		sockcntl.hints.ai_socktype = SOCK_STREAM;
		sockcntl.flags |= SKCTL_TCP_SERVER|SKCTL_PRINTADDR;
	} else {
		sockcntl.hints.ai_protocol = IPPROTO_UDP;
		sockcntl.hints.ai_socktype = SOCK_DGRAM;
		sockcntl.flags |= SKCTL_UDP_SERVER|SKCTL_PRINTADDR;
	}
	
	if(skctl(&sockcntl) < 0) exit(0);
	gwrl_set_fd(rl,sockcntl.sockfd,GWRL_RD,&server_socket_activity,NULL);
}

void setup_socket() {
	if(strcmp(mode,"s") == 0) setup_server();
	intsrc = gwrl_set_interval(rl,1000,&interval_test,NULL);
}

int main(int argc, char ** argv) {
	if(argc < 5) {
		printf("usage: echo c|s tcp|udp host|ip port|service\n");
		exit(0);
	}
	
	mode = argv[1];
	protocol = argv[2];
	node = argv[3];
	service = argv[4];
	istcp = (strcmp(protocol,"tcp") == 0);
	isserver = (strcmp(mode,"s") == 0);
	printf("%s %s %s %s %s\n",argv[0],argv[1],argv[2],argv[3],argv[4]);
	
	rl = gwrl_create();
	gwrl_post_function(rl,&setup_socket,NULL);
	gwrl_run(rl);
	return 0;
}
