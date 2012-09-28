
#include "gwrl/event.h"

void timeout2(gwrl * rl, gwrlevt * evt) {
	gwrl_stop(rl);
}

void timeout(gwrl * rl, gwrlevt * evt) {
	asserts_var1 = gwrl_assert_wake_reinit;
	close(rl->fds[1]);
	gwrl_set_timeout(rl,2000,false,&timeout2,NULL);
}

int main(int argc, char ** argv) {
	#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
	gwrl * rl = gwrl_create();
	gwrl_set_timeout(rl,10,false,&timeout,NULL);
	gwrl_run(rl);
	assert(asserts_var2 == true);
	#endif
	return 0;
}
