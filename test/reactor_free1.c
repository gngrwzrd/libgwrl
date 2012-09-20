
#include "gwrl/event.h"

void timeout(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwrl_set_timeout(rl,100,false,&timeout,NULL);
	gwrl_run(rl);
	gwrl_free(rl,NULL);
	return 0;
}
