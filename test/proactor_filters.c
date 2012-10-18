
#include "gwrl/proactor.h"

void filter(gwpr * pr, gwpr_io_info * info){}
void filter2(gwpr * pr, gwpr_io_info * info){}
void filter3(gwpr * pr, gwpr_io_info * info){}

int main(int argc, char ** argv) {
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	gwrlsrc * src1 = gwrl_src_file_create(0,0,NULL,NULL);
	gwrlsrc_file * fsrc = _gwrlsrcf(src1);
	
	if(GWPR_FILTERS_MAX < 2) {
		printf("error: GWPR_FILTERS_MAX must be at least 2 for this test.\n");
		return -1;
	}
	
	gwpr_src_add(pr,src1);
	gwpr_filter_add(pr,src1,gwpr_rdfilter_id,&filter);
	assert(((gwprdata *)fsrc->pdata)->rdfilters != NULL);
	assert(((gwprdata *)fsrc->pdata)->rdfilters[0] == &filter);
	
	gwpr_filter_add(pr,src1,gwpr_rdfilter_id,&filter2);
	assert(_gwprdata(fsrc->pdata)->rdfilters != NULL);
	assert(_gwprdata(fsrc->pdata)->rdfilters[1] == &filter2);
	
	gwpr_filter_reset(pr,src1,gwpr_rdfilter_id);
	gwpr_filter_reset(pr,src1,gwpr_wrfilter_id);
	assert(((gwprdata *)fsrc->pdata)->rdfilters[0] == NULL);
	assert(((gwprdata *)fsrc->pdata)->wrfilters[0] == NULL);
	
	filter(NULL,NULL);
	filter2(NULL,NULL);
	filter3(NULL,NULL);
	
	return 0;
}
