
#include "gwrl/proactor.h"
#include <pthread.h>

pthread_t th1;
pthread_key_t th1key;

typedef struct thdata {
	gwrl * rl;
	gwpr * pr;
} thdata;

void stdin_read(gwpr * pr, gwpr_io_info * info) {
	gwpr_buf_free(pr,info->buf);
	thdata * data = (thdata *)pthread_getspecific(th1key);
	gwrl * rl = data->rl;
	gwrl_stop(rl);
}

void testThreadSpecificData() {
	thdata * data = (thdata *)pthread_getspecific(th1key);
	gwpr * pr = data->pr;
	gwrlsrc * fsrc = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	gwpr_set_cb(pr,fsrc,gwpr_did_read_cb_id,&stdin_read);
	gwpr_read(pr,fsrc,128);
}

void setupReactor(gwrl * rl, gwrlevt * evt) {
	testThreadSpecificData();
}

void teardown_reactor(gwrl * rl) {
	gwrlsrc * src = NULL;
	gwrlsrc * hsrc = NULL;
	gwrl_free(rl,&hsrc);
	src = hsrc;
	while(src) {
		if(src->type == GWRL_SRC_TYPE_FILE) close(_gwrlsrcf(src)->fd);
		src = src->next;
	}
	gwrl_free(NULL,&hsrc);
}

void * threadMain(void * arg) {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	thdata * data = malloc(sizeof(thdata));
	data->rl = rl;
	data->pr = pr;
	pthread_key_create(&th1key,NULL);
	pthread_setspecific(th1key,data);
	gwrl_post_function(rl,&setupReactor,NULL);
	gwrl_run(rl);
	teardown_reactor(rl);
	return NULL;
}

int main(int argc, char ** argv) {
	pthread_create(&th1,NULL,&threadMain,NULL);
	pthread_join(th1,NULL);
	return 0;
}
