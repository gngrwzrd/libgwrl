
#include "gwrl/event.h"

int count = 0;

void timeout1(gwrl * rl, gwrlevt * evt) {
	count++;
	if(count == 10) {
		gwrl_stop(rl);
	} else {
		gwrl_src_enable(rl,evt->src);
	}
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwrl_set_timeout(rl,10,true,&timeout1,NULL);
	gwrl_run(rl);
	assert(count == 10);
	return 0;
}
