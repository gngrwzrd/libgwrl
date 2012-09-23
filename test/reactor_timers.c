
#include "gwrl/event.h"

gwrl * rl1 = NULL;
gwrlsrc * tsrc1 = NULL;

void functionCall(gwrl * rl, gwrlevt * evt) {
	printf("> function call!\n");
}

void delayedFunctionCall(gwrl * rl, gwrlevt * evt) {
	printf("> delayed function call!\n");
}

void futureTimeout(gwrl *rl, gwrlevt *evt) {
	printf("fiture timeout!\n");
	gwrl_stop(rl);
}

void timeout1(gwrl * rl, gwrlevt * evt) {
	printf("> timeout1\n");
	//gwrl_post_function(rl,&functionCall,NULL);
	//if(!gwrlsrc_enabled(tsrc1)) {
		//gwrlsrc_enable(rl,tsrc1);
	//}
}

void timeout2(gwrl * rl, gwrlevt * evt) {
	printf("> timeout2\n");
	gwrl_set_timeout(rl1,3000,false,&timeout2,NULL);
}

void timeout3(gwrl * rl, gwrlevt * evt) {
	printf("> timeout3\n");
}

int main(int argc, char ** argv) {
	struct timeval tv = {0};
	int64_t ms = 0;
	
	rl1 = gwrl_create();
	gwrl_post_function(rl1,&functionCall,NULL);
	gwrl_post_function(rl1,&functionCall,NULL);
	gwrl_post_function(rl1,&functionCall,NULL);
	gwrl_post_function(rl1,&functionCall,NULL);
	gwrl_set_timeout(rl1,2000,false,&delayedFunctionCall,NULL);
	gwrl_set_interval(rl1,1258,&timeout1,NULL);
	gwrl_set_timeout(rl1,3000,false,&timeout2,NULL);
	tsrc1 = gwrl_set_timeout(rl1,1000,true,&timeout3,NULL);
	
	gettimeofday(&tv,NULL);
	//printf("now: %ld.%ld\n",tv.tv_sec,tv.tv_usec);

	gwtm_add_ms_to_timeval(10000,&tv);
	//printf("future time: %ld.%ld\n",tv.tv_sec,tv.tv_usec);
	
	gwtm_timeval_to_ms(&tv,&ms);
	//printf("future ms timeout: %lld\n",ms);
	
	gwrl_set_date_timeout(rl1,ms,&futureTimeout,NULL);
	gwrl_run(rl1);
	return 0;
}
