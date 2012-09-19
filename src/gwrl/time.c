
#include "gwrl/time.h"

#ifdef __cplusplus
extern "C" {
#endif

void gwtm_gettimeofday_timespec(struct timespec * ts) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	gwtm_timeval_to_timespec(&tv,ts);
}

void gwtm_ms_to_timeval(int64_t ms, struct timeval * tv) {
	tv->tv_sec = (long)(ms/1000);
	tv->tv_usec = 1000*(ms%1000);
	if(tv->tv_usec >= _MICROSECONDS_WC) {
		tv->tv_sec += 1;
		tv->tv_usec -= _MICROSECONDS_WC;
	}
}

void gwtm_ms_to_timespec(int64_t ms, struct timespec * ts) {
	ts->tv_sec = (long)(ms/1000);
	ts->tv_nsec = 1000000 * (ms%1000);
	if(ts->tv_nsec >= _NANOSECONDS_WC) {
		ts->tv_sec += 1;
		ts->tv_nsec -= _NANOSECONDS_WC;
	}
}

void gwtm_timeval_to_ms(struct timeval * tv, int64_t * ms) {
	if(tv->tv_sec == 0 && tv->tv_usec == 0) {
		*ms = 0;
		return;
	}
	*ms = ((int64_t)tv->tv_sec)*1000;
	*ms += tv->tv_usec/1000;
}

void gwtm_timespec_to_ms(struct timespec * ts, int64_t * ms) {
	if(ts->tv_sec == 0 && ts->tv_nsec == 0) {
		*ms = 0;
		return;
	}
	*ms = ((int64_t)ts->tv_sec)*1000;
	*ms += ts->tv_nsec/1000000;
}

void gwtm_add_ms_to_timeval(int64_t ms, struct timeval * tv) {
	if(ms == 0) return;
	tv->tv_sec += (long)(ms/1000);
	tv->tv_usec += 1000*(ms%1000);
	while(tv->tv_usec >= _MICROSECONDS_WC) {
		tv->tv_sec += 1;
		tv->tv_usec -= _MICROSECONDS_WC;
	}
}

void gwtm_add_ms_to_timespec(int64_t ms, struct timespec * ts) {
	if(ms == 0) return;
	ts->tv_sec += (long)(ms/1000);
	ts->tv_nsec += 1000000*(ms%1000);
	while(ts->tv_nsec >= _NANOSECONDS_WC) {
		ts->tv_sec += 1;
		ts->tv_nsec -= _NANOSECONDS_WC;
	}
}

struct timespec * gwtm_timespec_cmp(struct timespec * ts1, struct timespec * ts2) {
	if(ts1->tv_sec < ts2->tv_sec) return ts1;
	if(ts1->tv_sec == ts2->tv_sec) {
		if(ts1->tv_nsec < ts2->tv_nsec) return ts1;
		else if(ts1->tv_nsec == ts2->tv_nsec) return NULL;
	}
	return ts2;
}

bool gwtm_timespec_copy_if_smaller(struct timespec * source, struct timespec * update) {
	struct timespec * smaller = gwtm_timespec_cmp(source,update);
	if(smaller == source) {
		memcpy(update,smaller,sizeof(struct timespec));
		return true;
	}
	return false;
}

void gwtm_timespec_sub_into(struct timespec * ts1, struct timespec * ts2, struct timespec * into) {
	into->tv_sec = ts1->tv_sec - ts2->tv_sec;
	into->tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
	while(into->tv_nsec < 0) {
		into->tv_sec -= 1;
		into->tv_nsec += _NANOSECONDS_WC;
	}
}

bool gwtm_timespec_is_expired(struct timespec * ts1) {
	if(ts1->tv_sec < 0) return true;
	if(ts1->tv_sec <= 0 && ts1->tv_nsec <= 0) return true;
	return false;
}

void gwtm_timeval_to_timespec(struct timeval * tv, struct timespec * ts) {
	ts->tv_sec = tv->tv_sec;
	ts->tv_nsec = ((int64_t)tv->tv_usec)*1000;
}

void gwtm_timespec_to_timeval(struct timespec * ts, struct timeval * tv) {
	tv->tv_sec = ts->tv_sec;
	tv->tv_usec = ((long)ts->tv_nsec)/1000;
}

#ifdef __cplusplus
}
#endif
