
#include "gwrl/event.h"

int count = 0;

void interval(gwrl * rl, gwrlevt * evt) {
	count++;
	if(count == 10) {
		gwrl_stop(rl);
	}
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwrl_set_interval(rl,10,&interval,NULL);
	gwrl_run(rl);
	return 0;
}
