
#include "gwrl/proactor.h"

gwrl * rl;
gwpr * pr;
gwrlsrc * src1;

void timeout(gwrl * rl, gwrlevt * evt) {
	gwpr_src_remove(pr,src1);
	gwrl_stop(rl);
}

void timeout2(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
}

int main(int argc, char ** argv) {
	rl = gwrl_create();
	pr = gwpr_create(rl);
	
	src1 = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	gwpr_src_remove(pr,src1);
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next == NULL);
	gwpr_src_add(pr,src1);
	gwpr_src_del(pr,src1);
	src1 = NULL;
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next == NULL);
	
	src1 = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	gwrl_set_timeout(rl,10,false,&timeout,NULL);
	gwrl_run(rl);
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next == NULL);

	src1 = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	gwpr_src_add_safely(pr,src1);
	gwrl_set_timeout(rl,10,false,&timeout2,NULL);
	gwrl_run(rl);
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next != NULL);
	
	return 0;
}