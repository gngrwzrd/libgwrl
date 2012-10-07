
#include "gwrl/proactor.h"

void cb1(gwpr * pr, gwpr_io_info * info){}
void cb2(gwpr * pr, gwpr_io_info * info){}
void cb3(gwpr * pr, gwpr_io_info * info){}
void cb4(gwpr * pr, gwpr_io_info * info){}
void cb5(gwpr * pr, gwpr_io_info * info){}
void cb6(gwpr * pr, gwpr_io_info * info){}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	gwrlsrc_file * fsrc = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	gwpr_set_cb(pr,fsrc,gwpr_error_cb_id,&cb1);
	gwpr_set_cb(pr,fsrc,gwpr_accept_cb_id,&cb2);
	gwpr_set_cb(pr,fsrc,gwpr_did_read_cb_id,&cb3);
	gwpr_set_cb(pr,fsrc,gwpr_did_write_cb_id,&cb4);
	gwpr_set_cb(pr,fsrc,gwpr_closed_cb_id,&cb5);
	gwpr_set_cb(pr,fsrc,gwpr_connect_cb_id,&cb6);
	cb1(NULL,NULL);
	cb2(NULL,NULL);
	cb3(NULL,NULL);
	cb4(NULL,NULL);
	cb5(NULL,NULL);
	cb6(NULL,NULL);
	return 0;
}