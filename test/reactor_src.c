
#include "gwrl/event.h"

int main(int argc, char ** argv) {
	gwrl * rl = NULL;
	gwrlsrc * src = NULL;
	gwrlsrc * src2 = NULL;
	gwrlsrc * src3 = NULL;
	
	rl = gwrl_create();
	
	//clear sources
	rl->sources[GWRL_SRC_TYPE_FILE] = NULL;
	src = gwrl_src_file_create(0,0,NULL,NULL);
	gwrl_src_add(rl,src);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] != NULL);
	gwrl_src_remove(rl,src);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] == NULL);
	gwrl_src_add(rl,src);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] != NULL);
	gwrl_src_del(rl,src,NULL,true);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] == NULL);
	src = NULL;
	
	//clear sources
	rl->sources[GWRL_SRC_TYPE_FILE] = NULL;
	src = gwrl_src_file_create(0,0,NULL,NULL);
	src2 = gwrl_src_file_create(0,0,NULL,NULL);
	gwrl_src_add(rl,src);
	gwrl_src_add(rl,src2);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] != NULL);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] == src2);
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next == src);
	gwrl_src_remove(rl,src);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] == src2);
	gwrl_src_add(rl,src);
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next == src2);
	gwrl_src_remove(rl,src2);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] == src);
	
	rl->sources[GWRL_SRC_TYPE_FILE] = NULL;
	src = gwrl_src_file_create(0,0,NULL,NULL);
	src2 = gwrl_src_file_create(0,0,NULL,NULL);
	src3 = gwrl_src_file_create(0,0,NULL,NULL);
	gwrl_src_add(rl,src);
	gwrl_src_add(rl,src2);
	gwrl_src_add(rl,src3);
	gwrl_src_remove(rl,src2);
	assert(rl->sources[GWRL_SRC_TYPE_FILE] == src3);
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next == src);
	assert(rl->sources[GWRL_SRC_TYPE_FILE]->next->next == NULL);
	
	return 0;
}
