
#include <gwrl/proactor.h>

bool
gwpr_src_activity_write_qitem_result(gwpr * pr, gwrlsrc_file * fsrc,
gwprwrq * q, gwpr_io_info * ioinfo, gwpr_error_info * errinfo,
size_t written, int errnm, bool * stopwrite);

void didwr(gwpr * pr, gwpr_io_info * info) {
	assert(info->buf->len == 6);
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	gwprdata * pdata = NULL;
	gwpr_io_info ioinfo = {0};
	gwpr_error_info errinfo = {0};
	
	bool stopwrite = false;
	bool continue_result = true;

	//setup sources
	gwrlsrc_file * wrsrc = gwpr_set_fd(pr,0,NULL);
	gwpr_set_cb(pr,wrsrc,gwpr_did_write_cb_id,&didwr);
	pdata = wrsrc->pdata;
	gwprwrq * wrsrcq = gwprwrq_get(pr,wrsrc);
	gwprbuf * wrsrcbuf = gwpr_buf_get_with_data(pr,12,"hello world",12);
	wrsrcq->buf = wrsrcbuf;

	continue_result = gwpr_src_activity_write_qitem_result(pr,wrsrc,wrsrcq,&ioinfo,&errinfo,6,0,&stopwrite);
	assert(!continue_result);
	assert(pdata->wrq);
	assert(pdata->wrq->buf->len == 6);

	return 0;
}