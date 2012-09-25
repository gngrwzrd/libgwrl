
#include "gwrl/event.h"

bool fired = false;

void event(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
	fired = true;
}

int main(int argc, char ** argv) {
	int count = 0;
	gwrl * rl = gwrl_create();
	gwrlsrc * src = gwrl_src_file_create(STDIN_FILENO,GWRL_RD,NULL,NULL);
	gwrlevt * evt = gwrl_evt_create(rl,src,&event,NULL,STDIN_FILENO,GWRL_RD);
	gwrlsrc * src2 = NULL;
	gwrl_src_add_safely(rl,src);
	gwrl_post_evt_safely(rl,evt);
	
	//this clears the internal wake mechanism, just for testing only
	rl->sources[GWRL_SRC_TYPE_FILE] = NULL;
	
	//run and assert stuff fired and was added.
	gwrl_run(rl);
	assert(fired == true);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] != NULL);
	
	//add more queued data so it uses the internal queue freeing logic
	src = gwrl_src_file_create(STDIN_FILENO,GWRL_RD,NULL,NULL);
	evt = gwrl_evt_create(rl,src,&event,NULL,STDIN_FILENO,GWRL_RD);
	gwrl_src_add_safely(rl,src);
	gwrl_post_evt_safely(rl,evt);
	
	//now free
	gwrl_free(rl,&src);
	src2 = src;
	while(src2) {
		count++;
		if(src2->type == GWRL_SRC_TYPE_FILE) {
			close(_gwrlsrcf(src)->fd);
		}
		src2 = src2->next;
	}
	assert(count == 2);
	gwrl_free(NULL,&src);

	return 0;
}
