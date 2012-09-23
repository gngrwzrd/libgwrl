
#include "gwrl/event.h"

#define MY_SOURCE_TYPE 2

typedef struct my_source {
	gwrlsrc _;
	int something;
} my_source;

void my_gather(gwrl * rl) {
	gwrlsrc * src = rl->sources[MY_SOURCE_TYPE];
	my_source * msrc = NULL;
	while(src) {
		msrc = (my_source *)src;
		msrc->something++;
		if(msrc->something == 10) {
			msrc->something = 0;
			gwrlevt * evt = gwrl_evt_create(rl,src,src->callback,src->userdata,0,0);
			gwrl_post_evt(rl,evt);
		}
		src = src->next;
	}
}

void my_callback(gwrl * rl, gwrlevt * evt) {
	//printf("custom gather fired an event for a custom input source.\n");
	gwrl_stop(rl);
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	my_source * msrc = gwrl_mem_calloc(1,sizeof(my_source));
	gwrlsrc * src = _gwrlsrc(msrc);
	
	#if GWRL_SRC_TYPES_COUNT < 3
		printf("error: GWRL_SRC_TYPES_COUNT must be at least 3 for this example.\n");
		exit(-1);
	#endif
	
	#if GWRL_GATHER_FUNCS_MAX < 1
		printf("error: GWRL_GATHER_FUNCS_MAX must be at least 1 for this example\n");
		exit(-1);
	#endif
	
	msrc->something = 0;
	src->type = MY_SOURCE_TYPE;
	src->callback = &my_callback;
	gwrl_add_gather_fnc(rl,&my_gather);
	gwrl_allow_poll_sleep(rl,0);
	gwrl_src_add(rl,src);
	gwrl_run(rl);
	return 0;
}
