
#include <gwrl/proactor.h>

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	gwrlsrc * src1 = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	gwrlsrc_file * fsrc1 = _gwrlsrcf(src1);
	gwprwrq * q1 = gwprwrq_get(pr,fsrc1);
	gwprwrq * q2 = gwprwrq_get(pr,fsrc1);
	gwprwrq_free(pr,fsrc1,q1);
	gwprwrq_free(pr,fsrc1,q2);
	q1 = gwprwrq_get(pr,fsrc1);
	assert(q1);
	q2 = gwprwrq_get(pr,fsrc1);
	assert(q2);
	assert(pr->wrqcache == NULL);
	return 0;
}
