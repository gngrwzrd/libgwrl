
#include "gwrl/proactor.h"

gwrl * rl;
gwpr * pr;
gwrlsrc_file * stdinsrc;
gwrlsrc_file * stdoutsrc;

void did_write(gwpr * pr, gwpr_io_info * info) {
	printf("did write!\n");
	gwpr_buf_free(pr,info->buf);
}

void did_read(gwpr * pr, gwpr_io_info * info) {
	printf("did read!\n");
	gwpr_write(pr,stdoutsrc,info->buf);
}

void setup_stdin_polling(gwrl * rl, gwrlevt * evt) {
	
}

int main(int argc, char ** argv) {
	fileid_t stdinid = 0;
	
	#if defined(PLATFORM_WINDOWS)
	stdinid = GetStdHandle(STD_INPUT_HANDLE);
	#else
	stdin = STDIN_FILENO;
	#endif

	rl = gwrl_create();
	pr = gwpr_create(rl);
	
	stdinsrc = gwpr_set_fd(pr,stdinid,NULL);
	stdoutsrc = gwpr_set_fd(pr,stdinid,NULL);
	gwpr_set_cb(pr,stdinsrc,gwpr_did_read_cb_id,&did_read);
	//gwpr_set_cb(pr,stdoutsrc,gwpr_write_cb_id,&did_write);
	gwpr_read(pr,stdinsrc,gwpr_buf_get(pr,128));
	
	gwrl_run(rl);
	return 0;
}
