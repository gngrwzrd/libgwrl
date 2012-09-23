
#include "gwrl/event.h"

int main(int argc, char ** argv) {

	gwrl * rl = gwrl_create();
	//assert(rl->gatherfncs == NULL);
	
	gwrl_options opts = GWRL_DEFAULT_OPTIONS;
	opts.gwrl_gather_funcs_max = 10;
	
	gwrl_set_options(rl,&opts);
	assert(rl->gatherfncs != NULL);
	
	//assert(test_var1 == 1);
	//assert(test_var2 == 1);

	opts.gwrl_gather_funcs_max = 20;
	gwrl_set_options(rl,&opts);

	//assert(test_var3 == 1);
	//assert(test_var1 == 2);
	
	assert(rl->gatherfncs != NULL);	

	return 0;
}
