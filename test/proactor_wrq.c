
#include "gwrl/proactor.h"

int main(int argc, char ** argv) {
	
	gwrl * rl = gwrl_create();
	gwpr * pr = gwpr_create(rl);
	gwrlsrc_file * fsrc = gwpr_set_fd(pr,STDIN_FILENO,NULL);
	
	gwprwrq * q1 = gwprwrq_get(pr,fsrc);
	gwprwrq * q2 = gwprwrq_get(pr,fsrc);
	gwprwrq * q3 = gwprwrq_get(pr,fsrc);
	gwprwrq * q4 = gwprwrq_get(pr,fsrc);
	gwprdata * pdata = fsrc->pdata;
	
	assert(pr->nwrqcache == 0);
	assert(q1 && q2 && q3);
	
	//testing adding some
	gwprwrq_add(pr,fsrc,q1);
	gwprwrq_add(pr,fsrc,q2);
	assert(pdata->wrq == q1);
	assert(pdata->wrq->next == q2);
	assert(pdata->wrqlast == q2);
	
	//assert one more add
	gwprwrq_add(pr,fsrc,q3);
	assert(pdata->wrqlast == q3);
	assert(pdata->wrq->next->next == q3);

	//assert posting a q back.
	pdata->wrq = NULL;
	gwprwrq_putback(pr,fsrc,q1);
	assert(pdata->wrq == q1);
	assert(pdata->wrq->next == q2);
	assert(pdata->wrqlast == q3);
	assert(pdata->wrq->next->next == q3);

	//assert putting a q back when one already existed
	pdata->wrq = q4;
	gwprwrq_putback(pr,fsrc,q1);
	assert(pdata->wrq == q1);
	assert(pdata->wrq->next == q2);
	assert(pdata->wrq->next->next == q3);
	assert(pdata->wrq->next->next->next == q4);
	assert(pdata->wrqlast == q4);
	
	//assert free/cache
	pdata->wrq = NULL;
	gwprwrq_free(pr,fsrc,q1);
	assert(pr->nwrqcache == 1);
	assert(pr->wrqcache == q1);
	assert(pr->wrqcache->next == NULL);
	assert(q1->next == NULL);
	
	gwprwrq_free(pr,fsrc,q2);
	assert(pr->nwrqcache == 2);
	assert(pr->wrqcache == q2);
	assert(pr->wrqcache->next == q1);
	assert(pr->wrqcache->next->next == NULL);
	
	gwprwrq_free(pr,fsrc,q3);
	assert(pr->nwrqcache == 3);
	assert(pr->wrqcache == q3);
	assert(pr->wrqcache->next == q2);
	assert(pr->wrqcache->next->next == q1);
	assert(pr->wrqcache->next->next->next == NULL);

	q4 = pr->wrqcache;
	pr->wrqcache = NULL;
	pr->nwrqcache = 0;
	gwprwrq_free_list_no_cache(pr,q4);
	assert(pr->wrqcache == NULL);

	return 0;
}
