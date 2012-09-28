
#include "gwrl/proactor.h"

gwrl * rl;
gwpr * pr;
gwrlsrc_file * rdsrc = NULL;
gwrlsrc_file * wrsrc = NULL;
int rdcount = 0;
int sockets[2];
bool called_wrfilter1 = false;
bool called_wrfilter2 = false;
bool called_rdfilter1 = false;
bool called_rdfilter2 = false;

void didrd(gwpr * pr, gwpr_io_info * info) {
	assert(strcmp(info->buf->buf,"hello world") == 0);
	gwpr_buf_free(pr,info->buf);
	if(rdcount == 100) {
		gwrl_stop(rl);
	}
	rdcount++;
}

void didwr(gwpr * pr, gwpr_io_info * info) {	
	gwpr_buf_free(pr,info->buf);
}

void rdfilter1(gwpr * pr, gwpr_io_info * info) {
	assert(info->buf->buf[11] == '!');
	info->buf->buf[11] = '\0';
	called_rdfilter1 = true;
}

void rdfilter2(gwpr * pr, gwpr_io_info * info) {
	assert(info->buf->buf[12] == '!');
	info->buf->buf[12] = '\0';
	info->buf->len--;
	called_rdfilter2 = true;
}

void write_data(gwrl * rl, gwrlevt * evt) {
	gwprbuf * buf = gwpr_buf_get_with_data(pr,32,"hello world",12);
	buf->buf[11] = '\0';
	if(rdcount > 50) {
		pr->options.gwpr_synchronous_write_max_bytes = 0;
		gwpr_recv(pr,rdsrc,gwpr_buf_get(pr,128));
		gwpr_send(pr,wrsrc,buf);
	} else {
		gwpr_write(pr,wrsrc,buf);
	}
}

void wrfilter1(gwpr * pr, gwpr_io_info * info) {
	info->buf->buf[11] = '!';
	info->buf->buf[12] = '\0';
	called_wrfilter1 = true;
}

void wrfilter2(gwpr * pr, gwpr_io_info * info) {
	info->buf->buf[12] = '!';
	info->buf->buf[13] = '\0';
	info->buf->len++;
	called_wrfilter2 = true;
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
	
	gwpr_filter_add(pr,wrsrc,gwpr_wrfilter_id,&wrfilter1);
	gwpr_filter_add(pr,wrsrc,gwpr_wrfilter_id,&wrfilter2);
	gwpr_filter_add(pr,rdsrc,gwpr_rdfilter_id,&rdfilter1);
	gwpr_filter_add(pr,rdsrc,gwpr_rdfilter_id,&rdfilter2);
	
	gwpr_set_cb(pr,wrsrc,gwpr_did_write_cb_id,&didwr);
	gwpr_set_cb(pr,rdsrc,gwpr_did_read_cb_id,&didrd);
	gwpr_read(pr,rdsrc,gwpr_buf_get(pr,128));
	
	gwrl_set_interval(rl,0,&write_data,NULL);
	//gwrl_set_timeout(rl,10000,false,&timeout,NULL);
	gwrl_run(rl);
	
	timeout(rl,NULL);

	assert(called_rdfilter1);
	assert(called_rdfilter2);
	assert(called_wrfilter1);
	assert(called_wrfilter2);

	return 0;
}
