
#include "gwrl/event.h"

int main(int argc, char ** argv) {
	int64_t ms1 = 1345223184123;
	struct timeval tv1;
	struct timeval tv2;
	struct timespec ts1;
	struct timespec ts2;
	struct timespec ts3;

	gwtm_ms_to_timeval(ms1,&tv1);
	assert(tv1.tv_sec == 1345223184);
	assert(tv1.tv_usec == 123000);
	
	gwtm_timeval_to_ms(&tv1,&ms1);
	assert(ms1 == 1345223184123);

	gwtm_gettimeofday_timespec(&ts1);
	memcpy(&ts2,&ts1,sizeof(ts2));

	gwtm_timespec_to_ms(&ts1,&ms1);
	assert(ms1 >= (ts1.tv_sec*1000));
	assert(ms1 < ((int64_t)(ts1.tv_sec+1)*1000));

	gwtm_ms_to_timespec(ms1,&ts1);
	assert(ts1.tv_sec == ts2.tv_sec);
	assert(ts2.tv_nsec >= ts1.tv_nsec);
	assert(ts2.tv_nsec < (ts1.tv_nsec+1000000));
	
	gettimeofday(&tv1,NULL);
	memcpy(&tv2,&tv1,sizeof(tv2));
	gwtm_add_ms_to_timeval(2000,&tv2);
	assert(tv2.tv_sec == (tv1.tv_sec+2));
	
	gwtm_gettimeofday_timespec(&ts1);
	memcpy(&ts2,&ts1,sizeof(ts2));
	gwtm_add_ms_to_timespec(2000,&ts2);
	assert(ts2.tv_sec == (ts1.tv_sec+2));

	gwtm_gettimeofday_timespec(&ts1);
	sleep(1);
	gwtm_gettimeofday_timespec(&ts2);
	assert((gwtm_timespec_cmp(&ts1,&ts2) == &ts1));

	gwtm_gettimeofday_timespec(&ts1);
	gwtm_timespec_to_timeval(&ts1,&tv1);
	assert(tv1.tv_sec == ts1.tv_sec);
	assert(tv1.tv_usec == ((long)ts1.tv_nsec)/1000);

	gettimeofday(&tv1,0);
	gwtm_timeval_to_timespec(&tv1,&ts1);
	assert(tv1.tv_sec == ts1.tv_sec);
	assert(ts2.tv_nsec == (tv1.tv_usec*1000));

	gwtm_gettimeofday_timespec(&ts1);
	sleep(1);
	gwtm_gettimeofday_timespec(&ts2);
	gwtm_timespec_copy_if_smaller(&ts1,&ts2);
	assert(ts1.tv_sec == ts2.tv_sec);
	assert(ts1.tv_nsec == ts2.tv_nsec);

	gwtm_gettimeofday_timespec(&ts1);
	sleep(1);
	gwtm_gettimeofday_timespec(&ts2);
	gwtm_timespec_sub_into(&ts1,&ts2,&ts3);
	assert(gwtm_timespec_is_expired(&ts3));

	return 0;
}
