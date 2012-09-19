
#define INCLUDE_STDBOOL
#define INCLUDE_STDINT
#define INCLUDE_STDIO
#define INCLUDE_STRING
#define INCLUDE_SYS_TIME
#define INCLUDE_WINDOWS
#define INCLUDE_WINSOCK2
#define TYPES_BOOL
#define TYPES_STRUCT_TIMEVAL
#define TYPES_STRUCT_TIMESPEC
#include "gwrl/shared_config.h"
#include "gwrl/shared_include.h"
#include "gwrl/shared_types.h"

#ifndef GWTM_H
#define GWTM_H

//units per wall clock second
#define _MILLISECONDS_WC 1000
#define _MICROSECONDS_WC 1000000
#define _NANOSECONDS_WC  1000000000

void gwtm_ms_to_timeval(int64_t ms, struct timeval * tv);
void gwtm_ms_to_timespec(int64_t ms, struct timespec * ts);
void gwtm_timeval_to_ms(struct timeval * tv, int64_t * ms);
void gwtm_timespec_to_ms(struct timespec * ts, int64_t * ms);
void gwtm_add_ms_to_timeval(int64_t ms, struct timeval * tv);
void gwtm_add_ms_to_timespec(int64_t ms, struct timespec * ts);
void gwtm_gettimeofday_timespec(struct timespec * ts);
void gwtm_timeval_to_timespec(struct timeval * tv, struct timespec * ts);
void gwtm_timespec_to_timeval(struct timespec * ts, struct timeval * tv);
void gwtm_timespec_sub_into(struct timespec * ts1, struct timespec * ts2, struct timespec * into);
bool gwtm_timespec_is_expired(struct timespec * ts1);
bool gwtm_timespec_copy_if_smaller(struct timespec * source, struct timespec * update);
struct timespec * gwtm_timespec_cmp(struct timespec * ts1, struct timespec * ts2);

#ifndef HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval *, struct timezone * tz);
#endif

#endif
