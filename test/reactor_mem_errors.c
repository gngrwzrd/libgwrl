
#include "gwrl/proactor.h"

int main(int argc, char ** argv) {
	#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
	gwrl * rl = NULL;
	gwpr * pr = NULL;
	gwrlevt * evt = NULL;
	gwrlsrc * src = NULL;

	asserts_var1 = gwrl_create_fail;
	rl = gwrl_create();
	assert(rl == NULL);

	asserts_var1 = gwrlbkd_init_fail;
	rl = gwrl_create();
	assert(rl == NULL);
	
	asserts_var1 = gwrl_create_gatherfncs_fail;
	rl = gwrl_create();
	assert(rl == NULL);
	
	asserts_var1 = gwrl_evt_create_fail;
	rl = gwrl_create();
	evt = gwrl_evt_create(rl,NULL,NULL,NULL,0,0);
	assert(evt == NULL);

	asserts_var1 = gwrl_src_time_create_fail;
	src = gwrl_src_time_create(100,false,0,false,NULL,NULL);
	assert(src == NULL);

	asserts_var1 = gwrl_src_file_create_fail;
	src = gwrl_src_file_create(0,GWRL_RD,NULL,NULL);
	assert(src == NULL);

	pr = gwpr_create(rl);
	gwrl_free(rl,NULL);

	#endif
	return 0;
}
