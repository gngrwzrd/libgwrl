
#include "gwrl/proactor.h"

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	gwprbuf * buf1 = gwpr_buf_get(pr,128);
	gwprbuf * buf2 = gwpr_buf_get(pr,256);
	gwprbuf * buf3 = gwpr_buf_get(pr,512);
	
	assert(buf1 && buf2 && buf3);
	assert(buf1->bufsize == 128);
	assert(buf2->bufsize == 256);
	assert(buf3->bufsize == 512);

	return 0;
}
