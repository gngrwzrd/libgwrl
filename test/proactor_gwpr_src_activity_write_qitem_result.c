
#include <gwrl/proactor.h>

bool
gwpr_src_activity_write_qitem_result(gwpr * pr, gwrlsrc_file * fsrc,
gwprwrq * q, gwpr_io_info * ioinfo, gwpr_error_info * errinfo,
size_t written, int errnm, bool * stopwrite);

bool continue_result = false;
bool stopwrite = false;
gwrl * rl = NULL;
gwpr * pr = NULL;
gwrlsrc_file * src = NULL;
gwprbuf * srcbuf = NULL;
gwprwrq  * srcq = NULL;
gwprdata * pdata = NULL;
gwpr_io_info * ioinfo = NULL;
gwpr_error_info * errinfo = NULL;

bool didwr1b = false;
bool closed1 = false;

bool didwr2b = false;
bool closed2 = false;

bool didwr3b = false;
bool closed3 = false;

bool didwr4b = false;
bool closed4 = false;
bool error4b = false;

bool didwr5b = false;
bool closed5 = false;
bool error5b = false;

void reset() {
	rl = gwrl_create();
	pr = gwpr_create(rl);
	src = gwpr_set_fd(pr,0,NULL);
	srcq = gwprwrq_get(pr,src);
	srcbuf = gwpr_buf_get_with_data(pr,12,"hello world",12);
	srcq->buf = srcbuf;
	pdata = src->pdata;
	ioinfo = calloc(1,sizeof(gwpr_io_info));
	errinfo = calloc(1,sizeof(gwpr_error_info));
}

void didwr1(gwpr * pr, gwpr_io_info * info) {
	didwr1b = true;
	assert(info->buf->len == 6);
}

void close1(gwpr * pr, gwpr_io_info * info) {
	closed1 = true;
}

void error1(gwpr * pr, gwpr_error_info * info) {
}

void didwr2(gwpr * pr, gwpr_io_info * info) {
	didwr2b = true;
	assert(info->buf);
}

void close2(gwpr * pr, gwpr_io_info * info) {
	closed2 = true;
	assert(info->buf);
}

void didwr3(gwpr * pr, gwpr_io_info * info) {
	didwr3b = true;
	assert(info->buf == NULL);
}

void close3(gwpr * pr, gwpr_io_info * info) {
	closed3 = true;
	assert(info->buf == NULL);
}

void didwr4(gwpr * pr, gwpr_io_info * info) {
	didwr4b = true;
	assert(info->buf);
}

void close4(gwpr * pr, gwpr_io_info * info) {
	closed4 = true;
	assert(info->buf);
}

void error4(gwpr * pr, gwpr_io_info * info) {
	error4b = true;
}

void didwr5(gwpr * pr, gwpr_io_info * info) {
	didwr5b = true;
	assert(info->buf);
}

void close5(gwpr * pr, gwpr_io_info * info) {
	closed5 = true;
	assert(info->buf);
}

void error5(gwpr * pr, gwpr_error_info * info) {
	error5b = true;
	assert(info->errnm == EBADF);
}

int main(int argc, char ** argv) {
	reset();
	gwpr_set_cb(pr,src,gwpr_did_write_cb_id,didwr1);
	gwpr_set_cb(pr,src,gwpr_error_cb_id,error1);
	continue_result = gwpr_src_activity_write_qitem_result(pr,src,srcq,ioinfo,errinfo,6,0,&stopwrite);
	assert(!continue_result);
	assert(didwr1b);
	assert(pdata->wrq);
	assert(pdata->wrq->buf->len == 6);

	reset();
	gwpr_set_cb(pr,src,gwpr_did_write_cb_id,didwr2);
	gwpr_set_cb(pr,src,gwpr_closed_cb_id,close2);
	continue_result = gwpr_src_activity_write_qitem_result(pr,src,srcq,ioinfo,errinfo,6,ECONNRESET,&stopwrite);
	assert(continue_result);
	assert(closed2);
	assert(didwr2b);

	reset();
	gwpr_set_cb(pr,src,gwpr_did_write_cb_id,didwr3);
	gwpr_set_cb(pr,src,gwpr_closed_cb_id,close3);
	continue_result = gwpr_src_activity_write_qitem_result(pr,src,srcq,ioinfo,errinfo,0,ECONNRESET,&stopwrite);
	assert(continue_result);
	assert(closed3);

	reset();
	continue_result = gwpr_src_activity_write_qitem_result(pr,src,srcq,ioinfo,errinfo,0,EWOULDBLOCK,&stopwrite);
	assert(!continue_result);
	assert(pdata->wrq);

	reset();
	gwpr_set_cb(pr,src,gwpr_did_write_cb_id,didwr4);
	gwpr_set_cb(pr,src,gwpr_closed_cb_id,close4);
	gwpr_set_cb(pr,src,gwpr_error_cb_id,error4);
	continue_result = gwpr_src_activity_write_qitem_result(pr,src,srcq,ioinfo,errinfo,6,EBADF,&stopwrite);
	assert(!continue_result);
	assert(didwr4b);
	assert(error4b);

	reset();
	gwpr_set_cb(pr,src,gwpr_did_write_cb_id,&didwr5);
	gwpr_set_cb(pr,src,gwpr_closed_cb_id,&close5);
	gwpr_set_cb(pr,src,gwpr_error_cb_id,&error5);
	continue_result = gwpr_src_activity_write_qitem_result(pr,src,srcq,ioinfo,errinfo,0,EBADF,&stopwrite);
	assert(!continue_result);
	assert(error5b);

	return 0;
}
