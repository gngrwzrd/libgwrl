
#include "gwrl/event.h"

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	
	gwrl_options opts = GWRL_DEFAULT_OPTIONS;
	opts.gwrl_gather_funcs_max = 10;
	gwrl_set_options(rl,&opts);
	assert(rl->gatherfncs != NULL);
	
	opts.gwrl_gather_funcs_max = 20;
	gwrl_set_options(rl,&opts);
	assert(rl->gatherfncs != NULL);

	gwrl_options opts2 = GWRL_DEFAULT_OPTIONS;
	free(rl->gatherfncs);
	rl->gatherfncs = NULL;
	rl->options.gwrl_gather_funcs_max = 0;
	opts2.gwrl_gather_funcs_max = 10;
	gwrl_set_options(rl,&opts2);
	assert(rl->gatherfncs != NULL);

	return 0;
}
