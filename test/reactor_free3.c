
#include "gwrl/event.h"

void stdin_activity(gwrl * rl, gwrlevt * evt) {
}

void timeout(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwrlsrc * sources = NULL;
	gwrl_set_fd(rl,STDIN_FILENO,GWRL_RD,&stdin_activity,NULL);
	gwrl_set_timeout(rl,100,false,&timeout,NULL);
	gwrl_run(rl);
	gwrl_free(rl,&sources);
	while(sources) {
		if(sources->type == GWRL_SRC_TYPE_FILE) {
			close(_gwrlsrcf(sources)->fd);
		}
		sources = sources->next;
	}
	gwrl_free(NULL,&sources);
	stdin_activity(NULL,NULL);
	return 0;
}
