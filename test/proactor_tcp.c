
#include "gwrl/proactor.h"

gwrl * rl;
gwpr * pr;
gwrlsrc_file * rdsrc = NULL;
gwrlsrc_file * wrsrc = NULL;
int rdcount = 0;
int sockets[2];

void didrd(gwpr * pr, gwpr_io_info * info) {
	gwpr_buf_free(pr,info->buf);
	rdcount++;
	if(rdcount == 100) {
		gwrl_stop(rl);
	}
}

void didwr(gwpr * pr, gwpr_io_info * info) {
	gwpr_buf_free(pr,info->buf);
}

void write_data(gwrl * rl, gwrlevt * evt) {
	gwprbuf * buf = gwpr_buf_get_with_data(pr,12,"hello world",12);
	gwpr_write(pr,wrsrc,buf);
}

void timeout(gwrl * rl, gwrlevt * evt) {
	assert(rdcount > 0);
	gwrl_stop(rl);
}

int main(int argc, char ** argv) {
	socketpair(AF_UNIX,SOCK_STREAM,0,sockets);

	rl = gwrl_create();
	pr = gwpr_create(rl);
	
	rdsrc = gwpr_set_fd(pr,sockets[0],NULL);
	wrsrc = gwpr_set_fd(pr,sockets[1],NULL);
	
	gwpr_set_cb(pr,wrsrc,gwpr_did_write_cb_id,&didwr);
	gwpr_set_cb(pr,rdsrc,gwpr_did_read_cb_id,&didrd);
	gwpr_read(pr,rdsrc,gwpr_buf_get(pr,128));

	gwrl_set_interval(rl,0,&write_data,NULL);
	gwrl_set_timeout(rl,10000,false,&timeout,NULL);
	gwrl_run(rl);
	
	timeout(rl,NULL);

	return 0;
}
