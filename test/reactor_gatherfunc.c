
#include "gwrl/event.h"

int gather_count = 0;
bool did_gather = false;

void custom_gather(gwrl * rl) {
	did_gather = true;
	gather_count++;
}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	
	#if GWRL_GATHER_FUNCS_MAX < 2
	printf("error: GWRL_GATHER_FUNCS_MAX must be at least 2 for this test.\n");
	exit(-1);
	#endif
	
	gwrl_add_gather_fnc(rl,&custom_gather);
	gwrl_add_gather_fnc(rl,&custom_gather);
	assert(rl->gatherfncs[0] != NULL);
	assert(rl->gatherfncs[1] != NULL);
	gwrl_allow_poll_sleep(rl,0);
	gwrl_run_once(rl);
	assert(did_gather == true);
	assert(gather_count == 2);
	gwrl_reset_gather_fncs(rl);
	assert(rl->gatherfncs[0] == NULL);
	return 0;
}
