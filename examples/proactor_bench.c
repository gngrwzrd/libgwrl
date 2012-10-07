
#include "gwrl/proactor.h"
#include <getopt.h>

int timeout = 0;
int nsources = 0;
int * pipes;
bool use_pipes;
bool use_sockets;
bool stopped = false;
gwrl * rl = NULL;
gwpr * pr = NULL;
gwrlsrc_file * fsrc1;
gwrlsrc_file * fsrc2;
gwrlsrc_file ** sources;

size_t wrcount = 0;
size_t rdcount = 0;

void didrd(gwpr * pr, gwpr_io_info * info) {
	fprintf(stderr,"didrd\n");
	rdcount += info->buf->len;
	gwpr_buf_free(pr,info->buf);
}

void didwr(gwpr * pr, gwpr_io_info * info) {
	fprintf(stderr,"didwr\n");
	wrcount += info->buf->len;
	gwpr_buf_free(pr,info->buf);
	stopped = true;
}

void wr() {
	printf("wr\n");
	int i = 0;
	gwprbuf * buf = NULL;
	for(; i<nsources; i+=2) {
		fsrc2 = sources[i+1];
		buf = gwpr_buf_get_with_data(pr,1,"a",1);
		gwpr_write(pr,fsrc2,buf);
	}
}

void didtimeout(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
	stopped = true;
}

void setup() {
	int i = 0;
	rl = gwrl_create();
	pr = gwpr_create(rl);
	sources = calloc(1,sizeof(void*)*nsources);
	pipes = calloc(1,sizeof(int)*nsources);
	for(; i<nsources; i+= 2) {
		if(use_pipes) pipe(&pipes[i]);
		else if(use_sockets) socketpair(AF_UNIX,SOCK_STREAM,0,&pipes[i]);
		fsrc1 = gwpr_set_fd(pr,pipes[i],NULL);
		fsrc2 = gwpr_set_fd(pr,pipes[i+1],NULL);
		sources[i] = fsrc1;
		sources[i+1] = fsrc2;
		gwpr_set_cb(pr,fsrc1,gwpr_did_read_cb_id,&didrd);
		gwpr_set_cb(pr,fsrc2,gwpr_did_write_cb_id,&didwr);
		gwpr_read(pr,fsrc1,gwpr_buf_get(pr,1));
	}
	gwrl_set_timeout(rl,timeout,false,&didtimeout,NULL);
	gwrl_set_interval(rl,0,&wr,NULL);
}

void teardown() {
	gwrlsrc * _sources = NULL;
	gwrlsrc * _sources_dup = NULL;
	gwpr_free(pr);
	gwrl_free(rl,&_sources);
	_sources_dup = _sources;
	while(_sources) {
		if(_sources->type == GWRL_SRC_TYPE_FILE) {
			close(_gwrlsrcf(_sources)->fd);
		}
		_sources = _sources->next;
	}
	gwrl_free(NULL,&_sources_dup);
}

int main(int argc, char ** argv) {
	int ch = 0;
	int64_t ms = 0;
	struct timespec ts1,ts2,ts3;
	
	struct option opts[] = {
		{"pipes",optional_argument,NULL,'p'},
		{"sockets",optional_argument,NULL,'s'},
		{"timeout",optional_argument,NULL,'t'},
		{"sources",optional_argument,NULL,'i'},
		{NULL,0,NULL,0}
	};
	
	while((ch = getopt_long_only(argc,argv,"p:s:t:i:",opts,NULL)) != -1) {
		switch(ch) {
		case 'p':
			use_pipes = true;
			break;
		case 's':
			use_sockets = true;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		case 'i':
			nsources = atoi(optarg) * 2;
			break;
		}
	}

	if(nsources == 0) nsources = 2;
	if(timeout < 1) timeout = 10000;
	if(!use_pipes && !use_sockets) use_pipes = true;

	setup();
	gwrl_allow_poll_sleep(rl,0);
	while(!stopped) {
		gwtm_gettimeofday_timespec(&ts1);
		printf("before\n");
		gwrl_run_once(rl);
		printf("after\n");
		gwtm_gettimeofday_timespec(&ts2);
		gwtm_timespec_sub_into(&ts2,&ts1,&ts3);
		fprintf(stderr,"ts3.tv_sec: %lu ts3.tv_nsec: %lu\n", (long)ts3.tv_sec, (long)ts3.tv_nsec);
		gwtm_timespec_to_ms(&ts3,&ms);
		fprintf(stderr,"ms: %8ld, writes: %8ld, reads: %8ld\n",(long)ms,wrcount,rdcount);
	}
	teardown();
	
	return 0;
}
