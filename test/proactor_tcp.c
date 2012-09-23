
#include "gwrl/proactor.h"

gwrl * mainrl;
gwpr * mainpr;

int main(int argc, char ** argv) {
	mainrl = gwrl_create();
	mainpr = gwpr_create(rl);
	
	return 0;
}
