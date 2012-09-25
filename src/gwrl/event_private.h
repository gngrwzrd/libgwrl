
#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
extern int asserts_var1;
extern bool asserts_var2;
typedef enum assert_codes {
	gwrl_create_fail = 10,
	gwrl_create_gatherfncs_fail = 11,
	gwrl_evt_create_fail = 12,
	gwrl_src_time_create_fail = 13,
	gwrl_src_file_create_fail = 14,
	gwrlbkd_init_fail = 20,
	gwrlbkd_no_sleep_assert_true = 21,
} assert_codes;
#endif

//(runloop flag) indicates a break in the runloop
#define GWRL_STOP (1 << 0)

//(runloop flag) indicates if the loop is currently
//sleeping in a system call
#define GWRL_SLEEPING (1 << 1)

//(runloop flag) indiciates that the loop isn't allowed to
//sleep in polling system calls.
#define GWRL_NOSLEEP (1 << 2)

//(source flag) whether or not the gwrlsrc is enabled.
#define GWRL_ENABLED (1 << 3)

//(source flag) gwrlsrc_time only, indicates that the
//src should generate repeated time events.
#define GWRL_REPEAT (1 << 4)

//(source flag) gwrlsrc_time only, indicates whether a timeout
//is relative to now, or a future absolute date in milliseconds.
#define GWRL_WHENCE_ABS (1 << 5)

//(source flag), gwrlsrc_time only, indicates whether a timer src
//should be left installed but disabled after it's fired. This is useful
//if you want to re-enable it later without re-installing a timer.
#define GWRL_PERSIST (1 << 6)

//(source / event flag)
//if this flag is set in a reactor callback evt->flags, then
//the reactor backend is event_iocp. The only way you'd ever
//see this is if you're using the reactor by itself with
//Windows - which isn't officially supported. You have to use
//the proactor with Windows.
#define GWRL_IOCP (1 << 2) //evt->src is a gwrl_ovlp

//(event flags only)
//this is used with the proactor and synchronous writes. when a
//synchronous write succeeds, an event is posted with this flag
//so the proactor can catch it specifically and call the correct
//callback.
#define GWRL_SYNC_WRITE (1 << 3)
#define GWRL_SYNC_CLOSE (1 << 4)
#define GWRL_SYNC_ERROR (1 << 5)

//calloc
#ifndef gwrl_mem_calloc
#	define gwrl_mem_calloc(n,s) (calloc(n,s))
#endif

//realloc
#ifndef gwrl_mem_realloc
#	define gwrl_mem_realloc(p,ns) (realloc(p,ns))
#endif

//free
#ifndef gwrl_mem_free
#	define gwrl_mem_free(mm) (free(mm))
#endif

//bit setting and clearing
#define flset(i,flag)(i|=flag)
#define flclr(i,flag)(i&=~flag)
#define flisset(i,flag)(i&flag)

//flags to indicate invalid timeouts
#define sec_min  -1
#define nsec_min -1

//some casting helpers
#define _gwrl(o) ((gwrl *)o)
#define _gwpr(o) ((gwpr *)o)
#define _gwrlevt(o) ((gwrlevt *)o)
#define _gwrlbkd(o) ((gwrlbkd *)o)
#define _gwrlbkdi(o) ((gwrlbkd_iocp *)o)
#define _gwrlbkdk(o) ((gwrlbkd_kqueue *)o)
#define _gwrlbkde(o) ((gwrlbkd_epoll *)o)
#define _gwrlbkds(o) ((gwrlbkd_select *)o)
#define _gwrlbkdp(o) ((gwrlbkd_poll *)o)
#define _gwrlsrc(o) ((gwrlsrc *)o)
#define _gwrlsrcf(o) ((gwrlsrc_file *)o)
#define _gwrlsrct(o) ((gwrlsrc_time *)o)
#define _gwprdata(o) ((gwprdata *)o)
#define _gwprovlp(o) ((gwpr_ovlp *)o)

gwrlbkd * gwrl_bkd_init(gwrl * rl);
void gwrl_dispatch(gwrl * rl);
void gwrl_bkd_enable_src(gwrl * rl, gwrlsrc * src);
void gwrl_bkd_disable_src(gwrl * rl, gwrlsrc * src);
void gwrl_bkd_del_src(gwrl * rl, gwrlsrc * src);
void gwrl_bkd_src_add(gwrl * rl, gwrlsrc * src);
void gwrl_bkd_free(gwrl * rl);
void gwrl_bkd_gather(gwrl * rl);
void gwrl_bkd_recalc_srcs(gwrl * rl);
void gwrl_bkd_set_options(gwrl * rl, gwrl_options * opts);
void gwrl_evt_free(gwrl * rl, gwrlevt * evt);
void gwrl_evt_free_list(gwrl * rl, gwrlevt * evt);
void gwrl_wake(gwrl * rl);
void gwrl_wake_init(gwrl * rl);
void gwrl_wake_activity(gwrl * rl, gwrlevt * evt);
void gwrl_wake_free(gwrl * rl);
