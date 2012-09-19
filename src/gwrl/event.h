
#define _DARWIN_UNLIMITED_SELECT
#define INCLUDE_ASSERT
#define INCLUDE_ERRNO
#define INCLUDE_EPOLL
#define INCLUDE_KQUEUE
#define INCLUDE_POLL
#define INCLUDE_SELECT
#define INCLUDE_STDIO
#define INCLUDE_STDLIB
#define INCLUDE_STDBOOL
#define INCLUDE_STDINT
#define INCLUDE_STRING
#define INCLUDE_STRINGS
#define INCLUDE_SYS_SOCKET
#define INCLUDE_UNISTD
#define INCLUDE_FCNTL
#define WIN_LEAN_MEAN
#define INCLUDE_WINDOWS
#define INCLUDE_WINSOCK2
#define INCLUDE_WS2TCPIP_H
#define INCLUDE_IPHLPAPI
#define TYPES_BOOL
#define TYPES_FILEID_T
#define TYPES_SSIZE_T
#define TYPES_SOCKLEN_T
#define TYPES_STDINT
#define TYPES_STRUCT_TIMEVAL
#define TYPES_STRUCT_SOCKADDR_STORAGE
#include "gwrl/shared_config.h"
#include "gwrl/shared_include.h"
#include "gwrl/shared_types.h"
#include "gwrl/time.h"
#include "gwrl/string.h"
#include "gwrl/socket.h"
#include "gwrl/lock.h"

#ifndef GWRL_EVENT_H
#define GWRL_EVENT_H

#if !defined(HAVE_BACKEND)
#	error No backend for gwrl event.
#endif

//default maximum amount of cached gwrlevt objects to keep in
//memory. this doesn't limit how many events can fire, just how
//many structures to cache for re-use. If there are more events
//than this in cache and events continue to dispatch they're freed
//and not added back to cache.
#ifndef GWRL_EVENT_CACHE_MAX
#	define GWRL_EVENT_CACHE_MAX 128
#endif

//default maximum number of gather functions that can be
//stored in gwrl->gatherfncs
#ifndef GWRL_GATHER_FUNCS_MAX
#	define GWRL_GATHER_FUNCS_MAX 0
#endif

//the maximum amount of times to re-dispatch events that
//were created by the user during the dispatch phase.
#ifndef GWRL_REDISPATCH_MAX
#	define GWRL_REDISPATCH_MAX 5
#endif

//types for gwrlsrc->type
#define GWRL_SRC_TYPE_FILE       0
#define GWRL_SRC_TYPE_TIME       1
#ifndef GWRL_SRC_TYPES_COUNT
#	define GWRL_SRC_TYPES_COUNT  2
#endif

//use these anytime a "whence" parameter is required.
#define GWRL_NOW 0 //relative to now
#define GWRL_ABS 1 //future date in milliseconds

//(source / interest flag)
//When adding a new file to monitor use these flags to
//indicate which events you want monitored.
#define GWRL_RD   (1 << 0) //read
#define GWRL_WR   (1 << 1) //write

//forwards
struct gwrl;
struct gwrlsrc;
struct gwrlevt;
struct gwrlbkd;

//user provided function callbacks
typedef void (gwrlevt_cb)(struct gwrl * rl, struct gwrlevt * evt);
typedef void (gwrl_gather_fnc)(struct gwrl * rl);

//runtime settable options for gwrl, gwrl_set_options()
typedef struct gwrl_options {
	int gwrl_event_cache_max;
	int gwrl_gather_funcs_max;
	int gwrl_kqueue_kevent_count;
	int gwrl_epoll_event_count;
	int gwrl_pollfd_count;
} gwrl_options;

//default runtime options
#define GWRL_DEFAULT_OPTIONS {\
GWRL_EVENT_CACHE_MAX,\
GWRL_GATHER_FUNCS_MAX,\
GWRL_KQUEUE_KEVENT_COUNT,\
GWRL_EPOLL_EVENT_COUNT,\
GWRL_POLLFD_COUNT}\

//these are bit flag types for various flag fields.
//in case they ever need to increase in size it can
//be done here.
typedef uint8_t gwrl_flags_t;
typedef uint8_t gwrlsrc_flags_t;
typedef uint8_t gwrlevt_flags_t;

//main run loop structure
typedef struct gwrl {
	gwrl_flags_t flags; //runloop flags
	uint16_t ncevents;  //number in cevents
	
	//self pipe trick for waking
	//windows uses IOCP PostQueuedCompletionStatus
	#if !defined(PLATFORM_WINDOWS)
	int fds[2];
	#endif
	
	//all registered input sources
	struct gwrlsrc * sources[GWRL_SRC_TYPES_COUNT];
	
	//fired events for next iteration
	struct gwrlevt * events;
	
	//cached re-usable events
	struct gwrlevt * cevents;
	
	//system event backend
	struct gwrlbkd * backend;
	
	//overridable runtime options
	struct gwrl_options options;
	
	//input sources queued for addition to the reactor from
	//the thread safe functions.
	struct gwrlsrc * _qsrc;
	lockid_t _qsrclk;

	//events queued for addition to the reactor from the
	//thread safe functions.
	struct gwrlevt * _qevt;
	lockid_t _qevtlk;

	//user defined gather functions to call
	gwrl_gather_fnc ** gatherfncs;
	
	#ifdef USING_PROACTOR
	void * pr; //gwpr, only if using the proactor.
	#endif
} gwrl;

//base input src, any input source needs to include
//this as it's first structure member.
typedef struct gwrlsrc {
	uint8_t type;          //type of the source
	uint8_t tag;           //tag for anyone to use
	gwrlsrc_flags_t flags; //source flags
	void * userdata;       //opaque user data
	gwrlevt_cb * callback; //callback
	struct gwrlsrc * next; //next src
} gwrlsrc;

//file input source
typedef struct _gwlsrc_file {
	gwrlsrc _;   //base input source
	fileid_t fd; //fd to monitor
	
	#ifdef USING_POLL_BACKEND
	struct pollfd * pfd; //struct pollfd * in gwrlbkd_poll->fds[].
	#endif
	
	#ifdef USING_PROACTOR
	void * pdata; //gwprdata, proactor specific file src data.
	#endif
} gwrlsrc_file;

//time input source
typedef struct gwrlsrc_time {
	gwrlsrc _;            //base input source
	int64_t ms;           //original milliseconds passed in
	struct timespec when; //when the event should fire
} gwrlsrc_time;

//base backend structure. all other backends include this as the
//first structure member.
typedef struct gwrlbkd {
	struct timespec timeout;
} gwrlbkd;

//event object for any event.
typedef struct gwrlevt {
	gwrlevt_flags_t flags; //interest flags (GWRL_RD, GWRL_WR)
	fileid_t fd;           //file descriptor
	void * userdata;       //opaque user data for users of the API
	gwrlevt_cb * callback; //callback
	struct gwrlsrc * src;  //gwrlsrc that generated the event, can be NULL.
	struct gwrlevt * next; //private, next fired event
} gwrlevt;

//create a runloop.
gwrl * gwrl_create();

//create and register a timeout input source.
gwrlsrc * gwrl_set_timeout(gwrl * rl, int64_t ms, bool persist, gwrlevt_cb * cb, void * userdata);

//create and register a repeating interval input source.
gwrlsrc * gwrl_set_interval(gwrl * rl, int64_t ms, gwrlevt_cb * callback, void * userdata);

//create and register a future date timeout.
gwrlsrc * gwrl_set_date_timeout(gwrl * rl, int64_t ms, gwrlevt_cb * cb, void * userdata);

//create and register a file input source.
gwrlsrc * gwrl_set_fd(gwrl * rl, fileid_t fd, gwrlsrc_flags_t flags, gwrlevt_cb * cb, void * userdata);

//create a file input source that you must register manually.
gwrlsrc * gwrl_src_file_create(fileid_t fd, gwrlsrc_flags_t flags, gwrlevt_cb * callback, void * userdata);

//create a time input source that you must register manually.
gwrlsrc * gwrl_src_time_create(int64_t ms, bool repeat, int whence, bool persist, gwrlevt_cb * callback, void * userdata);

//create an event postable to the reactor.
gwrlevt * gwrl_evt_create(gwrl * rl, gwrlsrc * src, gwrlevt_cb * callback, void * userdata, fileid_t fd, gwrlevt_flags_t flags);

//free a runloop.
void gwrl_free(gwrl * rl, gwrlsrc ** sources);

//run a runloop continually.
void gwrl_run(gwrl * rl);

//run a runloop only once.
void gwrl_run_once(gwrl * rl);

//set runtime options to override the default #defines.
void gwrl_set_options(gwrl * rl, gwrl_options * opts);

//stop the runloop.
void gwrl_stop(gwrl * rl);

//set whether or not kernel polling functions are allowed to put the current
//thread to sleep.
void gwrl_allow_poll_sleep(gwrl * rl, int onoff);

//add any input source.
void gwrl_src_add(gwrl * rl, gwrlsrc * src);

//thread safe version of gwrl_src_add.
void gwrl_src_add_safely(gwrl * rl, gwrlsrc * src);

//remove an input source but don't free it.
void gwrl_src_remove(gwrl * rl, gwrlsrc * src);

//remove and input source and optionally free it. the "prev" parameter
//is only used internally, unless you know how to use it pass NULL.
void gwrl_src_del(gwrl * rl, gwrlsrc * src, gwrlsrc * prev, bool freesrc);

//enable an input source.
void gwrl_src_enable(gwrl * rl, gwrlsrc * src);

//disable an input source.
void gwrl_src_disable(gwrl * rl, gwrlsrc * src);

//update the interest flags for an input source.
void gwrl_src_file_update_flags(gwrl * rl, gwrlsrc * src, gwrlsrc_flags_t flags);

//delete all expired persistent timeouts.
void gwrl_del_persistent_timeouts(gwrl * rl);

//add a custom gather function.
void gwrl_add_gather_fnc(gwrl * rl, gwrl_gather_fnc * fnc);

//post a function call.
void gwrl_post_function(gwrl * rl, gwrlevt_cb * cb, void * userdata);

//thread safe version of gwrl_post_function.
void gwrl_post_function_safely(gwrl * rl, gwrlevt_cb * cb, void * userdata);

//post an event.
void gwrl_post_evt(gwrl * rl, gwrlevt * evt);

//thread safe version of gwrl_post_evt.
void gwrl_post_evt_safely(gwrl * rl, gwrlevt * evt);

#include "event_private.h"
#endif
