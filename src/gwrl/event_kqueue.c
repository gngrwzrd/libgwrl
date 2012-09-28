
#include "gwrl/event.h"

//default amount of struct kevent to pull from kevent when polling.
#ifndef GWRL_KQUEUE_KEVENT_COUNT
#	define GWRL_KQUEUE_KEVENT_COUNT 64
#endif

//kqueue backend for gwrl->backend
typedef struct _gwrlbkd_kqueue {
	gwrlbkd _;                //base structure
	int kq;                   //kq fd
	size_t maxkevents;        //count of kevents
	struct kevent * kevents;  //container for kevents
} gwrlbkd_kqueue;

#ifdef __cplusplus
extern "C" {
#endif

gwrlbkd * gwrl_bkd_init(gwrl * rl) {
	gwrlbkd_kqueue * kbkd = _gwrlbkdk(gwrl_mem_calloc(1,sizeof(gwrlbkd_kqueue)));
	
	#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
		if(asserts_var1 == gwrlbkd_init_fail) {
			free(kbkd);
			kbkd = NULL;
		}
	#endif
	
	if(!kbkd) {
		#ifndef GWRL_HIDE_ERRORS
			gwerr("(3el0L) calloc error");
		#endif
		return NULL;
	}

	kbkd->kq = kqueue();
	if(kbkd->kq < 0) {
		gwerr("(1LKdoL) kqueue error");
		free(kbkd);
		return NULL;
	}
	kbkd->kevents = gwrl_mem_calloc(1,sizeof(struct kevent)*GWRL_KQUEUE_KEVENT_COUNT);
	kbkd->maxkevents = GWRL_KQUEUE_KEVENT_COUNT;
	rl->options.gwrl_kqueue_kevent_count = GWRL_KQUEUE_KEVENT_COUNT;
	return _gwrlbkd(kbkd);
}

void gwrl_bkd_set_options(gwrl * rl,gwrl_options * opts) {
	gwrlbkd_kqueue * kbkd = _gwrlbkdk(rl->backend);
	if(opts->gwrl_kqueue_kevent_count > kbkd->maxkevents) {
		kbkd->maxkevents = opts->gwrl_kqueue_kevent_count;
	}
	void * tmp = gwrl_mem_realloc(kbkd->kevents,sizeof(struct kevent) * kbkd->maxkevents);
	while(!tmp) {
		gwerr("(54oKD0) realloc error");
		tmp = gwrl_mem_realloc(kbkd->kevents,sizeof(struct kevent) * kbkd->maxkevents);
	}
	kbkd->kevents = (struct kevent *)tmp;
}

void gwrl_bkd_free(gwrl * rl) {
	gwrlbkd_kqueue * kbkd = _gwrlbkdk(rl->backend);
	close(kbkd->kq);
	free(kbkd->kevents);
	free(rl->backend);
	rl->backend = NULL;
}

void gwrl_bkd_kevent(gwrl * rl, gwrlsrc * src, int kflags, int kfilter) {
	int res = 0;
	struct kevent ke;
	struct timespec ts = {0};
	gwrlbkd_kqueue * kbkd = _gwrlbkdk(rl->backend);
	bzero(&ke,sizeof(ke));
	ke.ident = _gwrlsrcf(src)->fd;
	ke.udata = src;
	ke.flags = kflags;
	ke.filter = kfilter;
	res = kevent(kbkd->kq,&ke,1,NULL,0,&ts);
	if(res < 0 && errno != EBADF) {
		gwprintsyserr("(9dlkF) kevent error",errno);
	}
}

void gwrl_src_file_update_flags(gwrl * rl, gwrlsrc * src, gwrlsrc_flags_t flags) {
	
	//enable read
	if(flisset(flags,GWRL_RD)) {
		flset(flags,GWRL_ENABLED);
		gwrl_bkd_kevent(rl,src,EV_ADD|EV_ENABLE,EVFILT_READ);

	//disable read
	} else if(flisset(src->flags,GWRL_RD) && !flisset(flags,GWRL_RD)) {
		gwrl_bkd_kevent(rl,src,EV_ADD|EV_DISABLE,EVFILT_READ);
	}
	
	//enable write
	if(flisset(flags,GWRL_WR)) {
		flset(flags,GWRL_ENABLED);
		gwrl_bkd_kevent(rl,src,EV_ADD|EV_ENABLE,EVFILT_WRITE);
	}
	
	//disable write
	else if(flisset(src->flags,GWRL_WR) && !flisset(flags,GWRL_WR)) {
		gwrl_bkd_kevent(rl,src,EV_ADD|EV_DISABLE,EVFILT_WRITE);
	}
	
	src->flags = flags;
}

void gwrl_bkd_src_add(gwrl * rl, gwrlsrc * src) {
	int kflags = 0;

	if(flisset(src->flags,GWRL_RD)) {
		kflags = EV_ADD;
		if(!flisset(src->flags,GWRL_ENABLED)) kflags |= EV_DISABLE;
		gwrl_bkd_kevent(rl,src,kflags,EVFILT_READ);
	}
	
	if(flisset(src->flags,GWRL_WR)) {
		kflags = EV_ADD;
		if(!flisset(src->flags,GWRL_ENABLED)) kflags |= EV_DISABLE;
		gwrl_bkd_kevent(rl,src,kflags,EVFILT_WRITE);
	}
}

void gwrl_bkd_del_src(gwrl * rl, gwrlsrc * src) {
	if(flisset(src->flags,GWRL_RD)) {
		gwrl_bkd_kevent(rl,src,EV_DELETE,EVFILT_READ);
	}
	
	if(flisset(src->flags,GWRL_WR)) {
		gwrl_bkd_kevent(rl,src,EV_DELETE,EVFILT_WRITE);
	}
	
	flclr(src->flags,GWRL_ENABLED);
}

void gwrl_bkd_enable_src(gwrl * rl, gwrlsrc * src) {
	if(flisset(src->flags,GWRL_RD)) {
		gwrl_bkd_kevent(rl,src,EV_ENABLE,EVFILT_READ);
	}
	
	if(flisset(src->flags,GWRL_WR)) {
		gwrl_bkd_kevent(rl,src,EV_ENABLE,EVFILT_WRITE);
	}
	
	flset(src->flags,GWRL_ENABLED);
}

void gwrl_bkd_disable_src(gwrl * rl, gwrlsrc * src) {
	if(flisset(src->flags,GWRL_RD)) {
		gwrl_bkd_kevent(rl,src,EV_DISABLE,EVFILT_READ);
	}
	
	if(flisset(src->flags,GWRL_WR)) {
		gwrl_bkd_kevent(rl,src,EV_DISABLE,EVFILT_WRITE);
	}
	
	flclr(src->flags,GWRL_ENABLED);
}

void gwrl_bkd_gather(gwrl * rl) {
	int i = 0;
	int res = 0;
	bool postread = false;
	bool postwrite = false;
	struct kevent * event;
	struct timespec * timeout = &rl->backend->timeout;
	gwrlsrc * src = NULL;
	gwrlevt * evt = NULL;
	gwrlsrc_file * fsrc = NULL;
	gwrlbkd_kqueue * kbkd = _gwrlbkdk(rl->backend);
	
	if(timeout->tv_sec == sec_min && timeout->tv_nsec == nsec_min) {
		timeout = NULL;
	}
	
	if(!timeout && flisset(rl->flags,GWRL_NOSLEEP)) {
		struct timespec ts = {0};
		timeout = &ts;
		#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
			if(asserts_var1 == gwrlbkd_no_sleep_assert_true) {
				asserts_var2 = true;
			}
		#endif
	}
	
	flset(rl->flags,GWRL_SLEEPING);
	res = kevent(kbkd->kq,NULL,0,kbkd->kevents,rl->options.gwrl_kqueue_kevent_count,timeout);
	flclr(rl->flags,GWRL_SLEEPING);
	
	if(res == 0) return;
	if(res < 0 && (errno == EINVAL || errno == EINTR)) return;
	
	if(res > 0) {
		while(i < res) {
			postread = false;
			postwrite = false;
			event = &kbkd->kevents[i];
			src = event->udata;
			if(!flisset(src->flags,GWRL_ENABLED)) continue;
			
			if(src->type == GWRL_SRC_TYPE_FILE) {
				fsrc = _gwrlsrcf(src);
				
				if(event->flags & EV_ERROR) {
					gwrl_src_disable(rl,src);
					postread = true;
				}
				
				if((src->flags & GWRL_RD) && (event->flags & EV_EOF)) {
					postread = true;
				} else if((src->flags & GWRL_RD) && (event->filter == EVFILT_READ)) {
					postread = true;
				}
				
				if((src->flags & GWRL_WR) && (event->filter == EVFILT_WRITE)) {
					postwrite = true;
				}
					
				if(postread) {
					evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,GWRL_RD);
					while(!evt) evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,GWRL_RD);
					gwrl_post_evt(rl,evt);
				}
				
				else if(postwrite) {
					evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,GWRL_WR);
					while(!evt) evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,GWRL_WR);
					gwrl_post_evt(rl,evt);
				}
			}
			
			i++;
		}
	}
}

#ifdef __cplusplus
}
#endif
