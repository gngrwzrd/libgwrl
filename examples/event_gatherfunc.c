
#include "gwrl/event.h"

void my_callback(gwrl * rl, gwrlevt * evt) {
	printf("callback!\n");
	gwrl_stop(rl);
}

void my_gather(gwrl * rl) {
	//this my_gather function continually gets called during the gather
	//phase of the run loop. allowing you to poll anything you like
	//looking for things that need events dispatched.
	
	gwrlevt * evt = gwrl_evt_create(rl,NULL,&my_callback,NULL,0,0);
	gwrl_post_evt(rl,evt);
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwrl_add_gather_fnc(rl,&my_gather);
	gwrl_allow_poll_sleep(rl,0);
	gwrl_run(rl);
	return 0;
}
