
#include "gwrl/event.h"
#include <pthread.h>

gwrl * rl;
pthread_t th1;
bool did_timeout;

void timeout(gwrl * rl, gwrlevt * evt) {
	did_timeout = true;
	gwrl_stop(rl);
}

void * threadMain(void * arg) {
	sleep(1);
	gwrl_post_function_safely(rl,&timeout,NULL);
	return NULL;
}

void start_thread(gwrl * rl, gwrlevt * evt) {
	pthread_create(&th1,NULL,&threadMain,NULL);
}

int main(int argc, char ** argv) {
	rl = gwrl_create();
	gwrl_post_function(rl,&start_thread,NULL);
	gwrl_run(rl);
	pthread_join(th1,NULL);
	assert(did_timeout);
	return 0;
}
