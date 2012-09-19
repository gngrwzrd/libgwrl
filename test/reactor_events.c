
#include "gwrl/event.h"

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwrlevt * evt = NULL;
	gwrlevt * evt_tmp = NULL;
	int i = 0;
	
	evt = gwrl_evt_create(rl,NULL,NULL,NULL,0,0);
	gwrl_post_evt(rl,evt);
	assert(rl->events != NULL);
	gwrl_post_evt(rl,evt);
	assert(rl->events->next != NULL);
	
	rl->events = NULL;
	gwrl_evt_free(rl,evt);
	assert(rl->cevents != NULL);
	assert(rl->ncevents == 1);
	
	evt = gwrl_evt_create(rl,NULL,NULL,NULL,0,0);
	gwrl_post_evt(rl,evt);
	assert(rl->events != NULL);
	assert(rl->cevents == NULL);
	assert(rl->ncevents == 0);
	
	while(i < GWRL_EVENT_CACHE_MAX + 100) {
		i++;
		evt = gwrl_evt_create(rl,NULL,NULL,NULL,0,0);
		if(evt_tmp) evt->next = evt_tmp;
		evt_tmp = evt;
	}
	
	gwrl_evt_free_list(rl,evt);
	assert(rl->ncevents == GWRL_EVENT_CACHE_MAX);
	
	return 0;
}
