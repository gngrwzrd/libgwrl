
#include "gwrl/proactor.h"

void didrd(gwpr * pr, gwpr_io_info * info) {
}

void stop(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	gwrlsrc_file * fsrc = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	gwrlsrc * sources = NULL;
	gwrlsrc * del = NULL;
	
	//setup proactor and run
	gwpr_set_cb(pr,fsrc,gwpr_did_read_cb_id,&didrd);
	gwpr_read(pr,fsrc,gwpr_buf_get(pr,128));
	gwrl_post_function(rl,&stop,NULL);
	gwrl_run(rl);
	
	//free everything
	gwpr_free(pr);
	gwrl_free(rl,&sources);
	del = sources;
	while(del) {
		if(del->type == GWRL_SRC_TYPE_FILE) {
			close(_gwrlsrcf(del)->fd);
		}
		del = del->next;
	}
	gwrl_free(NULL,&sources);
	didrd(NULL,NULL);
	
	rl = gwrl_create();
	pr = gwpr_create(rl);
	rl->sources[GWRL_SRC_TYPE_FILE] = NULL;
	fsrc = _gwrlsrcf(gwrl_src_file_create(STDIN_FILENO,0,NULL,NULL));
	gwpr_src_add_safely(pr,fsrc);
	fsrc = _gwrlsrcf(gwrl_src_file_create(STDIN_FILENO,0,NULL,NULL));
	gwpr_src_add_safely(pr,fsrc);
	gwpr_free(pr);
	gwrl_free(rl,NULL);

	rl = gwrl_create();
	pr = gwpr_create(rl);
	rl->sources[GWRL_SRC_TYPE_FILE] = NULL;
	fsrc = _gwrlsrcf(gwrl_src_file_create(STDIN_FILENO,0,NULL,NULL));
	gwpr_src_add_safely(pr,fsrc);
	fsrc = _gwrlsrcf(gwrl_src_file_create(STDIN_FILENO,0,NULL,NULL));
	gwpr_src_add_safely(pr,fsrc);
	gwpr_free(pr);
	gwrl_free(rl,&sources);
	gwrl_free(NULL,&sources);

	return 0;
}