
#include "gwrl/proactor.h"

//variable socket pairs / pipes
//variable read sizes
//variable write sizes
//variable thread counts
//varant tcp / udp methods
//statistics collection interval

typedef struct gwstats {
	size_t wrcount;
	size_t rdcount;
} gsstats;

typedef struct thdata {
	gwrl * rl;
	gwpr * pr;
	struct gwstats stats;
} thdata;

int main(int argc, char ** argv) {
	
}
