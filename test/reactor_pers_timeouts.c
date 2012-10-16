
#include "gwrl/event.h"

void timeout3(gwrl * rl, gwrlevt * evt) {
	gwrl_del_persistent_timeouts(rl);
	gwrl_stop(rl);
}

void timeout2(gwrl * rl, gwrlevt * evt) {
	gwrl_set_timeout(rl,10,true,&timeout3,NULL);
}

void timeout1(gwrl * rl, gwrlevt * evt) {
	gwrl_set_timeout(rl,10,false,&timeout2,NULL);
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwrl_set_timeout(rl,10,true,&timeout1,NULL);
	gwrl_run(rl);
	assert(rl->sources[GWRL_SRC_TYPE_TIME] == NULL);
	return 0;
}
