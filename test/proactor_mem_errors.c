
#include "gwrl/proactor.h"

int main(int argc, char ** argv) {
	gwrl * rl = NULL;
	asserts_var1 = gwrl_create_fail;
	rl = gwrl_create();
	assert(rl == NULL);

	asserts_var1 = gwrlbkd_init_fail;
	rl = gwrl_create();
	assert(rl == NULL);
	return 0;
}
