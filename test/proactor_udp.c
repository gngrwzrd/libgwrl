
#include "gwrl/proactor.h"

gwrl * rl = NULL;
gwpr * pr = NULL;
gwrlsrc_file * rdsrc = NULL;
gwrlsrc_file * wrsrc = NULL;
int rdcount = 0;
int sockets[2];
struct sockaddr_storage peer;
socklen_t peerlen = 0;

void didrd(gwpr * pr, gwpr_io_info * info) {
	rdcount++;
	gwpr_buf_free(pr,info->buf);
	if(rdcount == 100) {
		gwrl_stop(rl);
	}
}

void didwr(gwpr * pr, gwpr_io_info * info) {
	gwpr_buf_free(pr,info->buf);
}

void error(gwpr * pr, gwpr_error_info * info) {
	gwprintsyserr("error:",info->errnm);
}

void write_data(gwrl * rl, gwrlevt * evt) {
	gwprbuf * buf = gwpr_buf_get_with_data(pr,12,"hello world",12);
	gwpr_sendto(pr,wrsrc,buf,&peer,peerlen);
}

int main(int argc, char ** argv) {
	bzero(&peer,sizeof(peer));
	
	struct skctlinfo server = {0};
	struct skctlinfo client = {0};
	skctl_udp_server(&server,0,"13009",AF_INET,true);
	skctl_udp_client(&client,0,"13009",AF_INET,true);
	
	peerlen = sizeof(struct sockaddr_storage);
	memcpy(&peer,server.used->ai_addr,server.used->ai_addrlen);

	gwsk_sockaddr_print(&peer);

	rl = gwrl_create();
	pr = gwpr_create(rl);
	rdsrc = gwpr_set_fd(pr,server.sockfd,NULL);
	wrsrc = gwpr_set_fd(pr,client.sockfd,NULL);

	gwrl_set_interval(rl,0,&write_data,NULL);
	
	gwpr_set_cb(pr,rdsrc,gwpr_did_read_cb_id,&didrd);
	gwpr_set_cb(pr,wrsrc,gwpr_did_write_cb_id,&didwr);
	gwpr_set_cb(pr,wrsrc,gwpr_error_cb_id,&error);
	gwpr_recvfrom(pr,rdsrc,gwpr_buf_get(pr,128));
	
	gwrl_run(rl);
	assert(rdcount == 100);

	return 0;
}