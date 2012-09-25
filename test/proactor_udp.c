
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

void write_data(gwrl * rl, gwrlevt * evt) {
	gwprbuf * buf = gwpr_buf_get_with_data(pr,12,"hello world",12);
	gwpr_sendto(pr,wrsrc,buf,&peer,peerlen);
}

void timeout(gwrl * rl, gwrlevt * evt) {
	assert(rdcount > 0);
	gwrl_stop(rl);
}

int main(int argc, char ** argv) {
	socketpair(AF_UNIX,SOCK_DGRAM,0,sockets);
	peerlen = sizeof(peer);
	getpeername(sockets[1],(struct sockaddr *)&peer,&peerlen);

	rl = gwrl_create();
	pr = gwpr_create(rl);
	rdsrc = gwpr_set_fd(pr,sockets[0],NULL);
	wrsrc = gwpr_set_fd(pr,sockets[1],NULL);

	gwrl_set_interval(rl,0,&write_data,NULL);
	gwrl_set_timeout(rl,10000,false,&timeout,NULL);

	gwpr_set_cb(pr,rdsrc,gwpr_did_read_cb_id,&didrd);
	gwpr_set_cb(pr,rdsrc,gwpr_did_write_cb_id,&didwr);
	gwpr_recvfrom(pr,rdsrc,gwpr_buf_get(pr,128));
	
	gwrl_run(rl);
	
	assert(rdcount == 100);

	return 0;
}