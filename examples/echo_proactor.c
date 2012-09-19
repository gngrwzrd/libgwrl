
#include "gwrl/proactor.h"

gwpr * pr;
gwrl * rl;
char * mode;
char * node;
char * service;
char * protocol;
bool istcp;
bool isserver;
skctlinfo skinfo;
gwrlsrc_file * client = NULL;
fileid_t stdinid = 0;
fileid_t stdoutid = 0;

void _error(gwpr * pr, gwpr_error_info * info) {
	gwprintsyserr("error",info->errnm);
}

void did_read(gwpr * pr, gwpr_io_info * info) {
	printf("did read: %lu\n",(unsigned long)info->buf->len);
	if(isserver && istcp) {
		gwpr_write(pr,info->src,info->buf);
	} else if(isserver && !istcp) {
		printf("peerlen: %lu\n",(unsigned long)info->peerlen);
		gwpr_sendto(pr,info->src,info->buf,&info->peer,info->peerlen);
	} else if(!isserver) {
		fprintf(stdout,"%s",info->buf->buf);
	}
}

void did_read_stdin(gwpr * pr, gwpr_io_info * info) {
	gwpr_write(pr,client,info->buf);
	printf("did read stdin!\n");
}

void did_write(gwpr * pr, gwpr_io_info * info) {
	size_t len = info->buf->len;
	gwpr_buf_free(pr,info->buf);
	printf("did write: %li\n",len);
}

void did_disconnect(gwpr * pr, gwpr_io_info * info) {
	gwsk_close((sockid_t)info->src->fd);
	gwpr_src_del(pr,info->src);
	printf("disconnected!\n");
}

void did_connect(gwpr * pr, gwpr_io_info * info) {
	gwrlsrc_file * stdinfd = gwpr_set_fd(pr,stdinid,NULL);
	gwpr_set_cb(pr,stdinfd,gwpr_did_read_cb_id,&did_read_stdin);
	gwpr_read(pr,stdinfd,gwpr_buf_get(pr,128));
	printf("connected!\n");
}

void did_accept(gwpr * pr, gwpr_io_info * info) {
	gwpr_src_add(pr,info->peersrc);
	gwpr_set_cb(pr,info->peersrc,gwpr_error_cb_id,&_error);
	gwpr_set_cb(pr,info->peersrc,gwpr_closed_cb_id,&did_disconnect);
	gwpr_set_cb(pr,info->peersrc,gwpr_did_read_cb_id,&did_read);
	gwpr_set_cb(pr,info->peersrc,gwpr_did_write_cb_id,&did_write);
	gwpr_read(pr,info->peersrc,gwpr_buf_get(pr,128));
	printf("did accept!\n");
}

void setup_server() {
	gwrlsrc_file * fsrc = NULL;
	
	if(istcp) {
		if((skctl_tcp_server(&skinfo,NULL,service,AF_INET,true)) < 0) exit(-1);
		fsrc = gwpr_set_fd(pr,(fileid_t)skinfo.sockfd,NULL);
		gwpr_set_cb(pr,fsrc,gwpr_accept_cb_id,&did_accept);
		gwpr_accept(pr,fsrc);
	} else {
		if((skctl_udp_server(&skinfo,NULL,service,AF_INET,true)) < 0) exit(-1);
		fsrc = gwpr_set_fd(pr,(fileid_t)skinfo.sockfd,NULL);
		gwpr_set_cb(pr,fsrc,gwpr_error_cb_id,&_error);
		gwpr_set_cb(pr,fsrc,gwpr_did_read_cb_id,&did_read);
		gwpr_set_cb(pr,fsrc,gwpr_did_write_cb_id,&did_write);
		gwpr_recvfrom(pr,fsrc,gwpr_buf_get(pr,512));
	}
	
	skctlinfo_free(&skinfo,false);
}

void setup_client() {
	gwrlsrc_file * fsrc = NULL;
	gwrlsrc_file * stdinfd = NULL;
	
	if(istcp) {
		if((skctl_tcp_client(&skinfo,node,service,AF_INET,true)) < 0) exit(-1);
		
		stdinfd = gwpr_set_fd(pr,stdinid,NULL);
		gwpr_set_cb(pr,stdinfd,gwpr_did_read_cb_id,&did_read_stdin);
		gwpr_read(pr,stdinfd,gwpr_buf_get(pr,128));
		
		fsrc = gwpr_set_fd(pr,(fileid_t)skinfo.sockfd,NULL);
		client = fsrc;
		gwpr_set_cb(pr,fsrc,gwpr_connect_cb_id,&did_connect);
		gwpr_set_cb(pr,fsrc,gwpr_closed_cb_id,&did_disconnect);
		gwpr_set_cb(pr,fsrc,gwpr_did_read_cb_id,&did_read);
		gwpr_set_cb(pr,fsrc,gwpr_did_write_cb_id,&did_write);
		gwpr_connect(pr,fsrc,(struct sockaddr_storage *)skinfo.used->ai_addr);
	} else {
		if((skctl_udp_connected_client(&skinfo,node,service,AF_INET,true)) < 0) exit(-1);
		
		stdinfd = gwpr_set_fd(pr,stdinid,NULL);
		gwpr_set_cb(pr,stdinfd,gwpr_did_read_cb_id,&did_read_stdin);
		gwpr_read(pr,stdinfd,gwpr_buf_get(pr,128));
		
		fsrc = gwpr_set_fd(pr,(fileid_t)skinfo.sockfd,NULL);
		client = fsrc;
		gwpr_set_cb(pr,fsrc,gwpr_error_cb_id,&_error);
		gwpr_set_cb(pr,fsrc,gwpr_did_read_cb_id,&did_read);
		gwpr_set_cb(pr,fsrc,gwpr_did_write_cb_id,&did_write);
		gwpr_recvfrom(pr,fsrc,gwpr_buf_get(pr,128));
	}
	
	skctlinfo_free(&skinfo,false);
}

void setup_socket(gwrl * rl, gwrlevt * evt) {
	if(isserver) setup_server();
	else setup_client();
}

int main(int argc, char ** argv) {
	if(argc < 5) {
		printf("usage: echo c|s tcp|udp host|ip port|service\n");
		exit(0);
	}
	
	gwsk_startup();
	
	#if defined(PLATFORM_WINDOWS)
	stdinid = GetStdHandle(STD_INPUT_HANDLE);
	stdoutid = GetStdHandle(STD_OUTPUT_HANDLE);
	#else
	stdinid = STDIN_FILENO;
	stdoutid = STDOUT_FILENO;
	#endif
	
	mode = argv[1];
	protocol = argv[2];
	node = argv[3];
	service = argv[4];
	
	istcp = (strcmp(protocol,"tcp") == 0);
	isserver = (strcmp(mode,"s") == 0);
	printf("%s %s %s %s %s\n","echo_proactor",mode,protocol,node,service);
	
	rl = gwrl_create();
	pr = gwpr_create(rl);
	gwrl_post_function(rl,&setup_socket,NULL);
	gwrl_run(rl);
	
	return 0;
}
