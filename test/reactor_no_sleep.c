
#include "gwrl/event.h"

int main(int argc, char ** argv) {
	
	#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
	gwrl * rl = gwrl_create();
	asserts_var2 = false;
	asserts_var1 = gwrlbkd_no_sleep_assert_true;
	gwrl_allow_poll_sleep(rl,0);
	gwrl_run_once(rl);
	assert(asserts_var2);
	#endif
	
	return 0;
}
