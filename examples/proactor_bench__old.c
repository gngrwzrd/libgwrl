
#include "gwrl/proactor.h"
#include <getopt.h>
#include <pthread.h>

//timeout to stop everything
//variable socket pairs / pipes
//variable read sizes
//variable write sizes
//variable thread counts
//varant tcp / udp methods
//statistics collection interval

void start_thread(void * main);
gwrl * create_reactor();
gwpr * create_proactor();

int thread_count = 1;
int sources_count = 12;
bool use_pipes = true;
bool use_sockets = false;
bool tcp_only = false;
bool udp_only = false;
bool mix_tcp_udp = false;
size_t wrsize = 0;
uint64_t timeout = 0;

#define ACTIVE_THREAD_MAX 128
int active_thread_used = 0;
int runloops_used = 0;
pthread_t * active_thread;
pthread_t active_threads[ACTIVE_THREAD_MAX];
gwrl * runloops[ACTIVE_THREAD_MAX];

void all_timeout(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
}

void * timeout_main(void * arg) {
	gwrl * rl = create_reactor();
	gwrl_set_timeout(rl,timeout,false,&all_timeout,NULL);
	gwrl_run(rl);
	gwrl_free(rl,NULL);
	return NULL;
}

void start_thread(void * main) {
	pthread_create(active_thread,NULL,main,NULL);
	active_thread_used++;
	active_thread++;
}

gwrl * create_reactor() {
	gwrl * rl = gwrl_create();
	runloops[runloops_used] = rl;
	runloops_used++;
	return rl;
}

gwpr * create_proactor() {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	runloops[runloops_used] = rl;
	runloops_used++;
	return pr;
}

int main(int argc, char ** argv) {
	int i = 0;
	int ch = 0;

	struct option opts[] = {
		{"threads",optional_argument,NULL,'t'},
		{"pipes",no_argument,NULL,'p'},
		{"sockets",no_argument,NULL,'s'},
		{"tcp_only",no_argument,NULL,'c'},
		{"udp_only",no_argument,NULL,'u'},
		{"mix_tcp_udp",no_argument,NULL,'m'},
		{"sources",optional_argument,NULL,'i'},
		{"size",optional_argument,NULL,'z'},
		{"timeout",optional_argument,NULL,'x'},
		{NULL,0,NULL,0}
	};

	while((ch=getopt_long_only(argc,argv,"t:p:s:c:u:m:i:z:x:",opts,NULL)) != -1) {
		switch(ch) {
		case 't':
			thread_count = atoi(optarg);
			if(thread_count < 1) thread_count = 1;
			break;
		case 'p':
			use_pipes = true;
			tcp_only = true;
			break;
		case 's':
			use_sockets = true;
			break;
		case 'c':
			tcp_only = true;
			break;
		case 'u':
			udp_only = true;
			break;
		case 'm':
			mix_tcp_udp = true;
			break;
		case 'i':
			sources_count = atoi(optarg);
			break;
		case 'z':
			wrsize = (size_t)atoi(optarg);
			break;
		case 'x':
			timeout = atoi(optarg);
			break;
		}
	}

	if(use_sockets && use_pipes) {
		fprintf(stderr,"--pipes and --sockets are mutually exclusive, use one or the other.\n");
		exit(1);
	}

	if(use_pipes && (udp_only || mix_tcp_udp)) {
		fprintf(stderr,"pipes can't use udp, ignoring udp request.\n");
		tcp_only = true;
		udp_only = false;
		mix_tcp_udp = false;
	}

	if(udp_only && tcp_only) {
		fprintf(stderr,"--tcp, --udp, and --mix are mutually exclusive, use one or the other.\n");
		exit(1);
	}

	if((mix_tcp_udp && udp_only) || (mix_tcp_udp && tcp_only)) {
		fprintf(stderr,"--tcp, --udp, and --mix are mutually exclusive, use one or the other.\n");
		exit(1);
	}
	
	if(mix_tcp_udp && (udp_only || tcp_only)) {
		fprintf(stderr,"already using tcp or udp exclusively, not mixing.\n");
		mix_tcp_udp = false;
	}
	
	active_thread = &active_threads[0];
	if(timeout > 0) start_thread(&timeout_main);
	for(; i<active_thread_used; i++) {
		pthread_join(active_threads[i],NULL);
	}

	return 0;
}
