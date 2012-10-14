
#include "gwrl/event.h"

//the number of epoll_event structs to accept from epoll
#ifndef GWRL_EPOLL_EVENT_COUNT
#	define GWRL_EPOLL_EVENT_COUNT 64
#endif

//epoll backend for gwrl->backend
typedef struct _gwrlbkd_epoll {
	gwrlbkd _;                    //base structure
	int efd;                      //epoll fd
	uint16_t maxnpevents;         //max number that will fit in pevents
	struct epoll_event * pevents; //container for epoll_events
} gwrlbkd_epoll;

#ifdef __cplusplus
extern "C" {
#endif

gwrlbkd * gwrl_bkd_init(gwrl * rl) {
	gwrlbkd_epoll * ebkd = _gwrlbkde(gwrl_mem_calloc(1,sizeof(gwrlbkd_epoll)));
	if(!ebkd) {
		gwerr("(el5KJz) calloc error");
		return NULL;
	}
	ebkd->efd = epoll_create(GWRL_EPOLL_EVENT_COUNT);
	if(ebkd->efd < 0) {
		gwprintsyserr("(po7kDs) epoll_create() error",errno);
		free(ebkd);
		return NULL;
	}
	ebkd->maxnpevents = GWRL_EPOLL_EVENT_COUNT;
	ebkd->pevents = (struct epoll_event *)gwrl_mem_calloc(1,sizeof(struct epoll_event) * ebkd->maxnpevents);
	rl->options.gwrl_epoll_event_count = ebkd->maxnpevents;
	return _gwrlbkd(ebkd);
}

void gwrl_bkd_set_options(gwrl * rl, gwrl_options * opts) {
	gwrlbkd_epoll  * ebkd = _gwrlbkde(rl->backend);
	if(opts->gwrl_epoll_event_count > ebkd->maxnpevents) {
		ebkd->maxnpevents = opts->gwrl_epoll_event_count;
	}
	void * tmp = gwrl_mem_realloc(ebkd->pevents,sizeof(struct epoll_event) * ebkd->maxnpevents);
	while(!tmp) {
		gwprintsyserr("(3RPcV) realloc error",errno);
		tmp = gwrl_mem_realloc(ebkd->pevents,sizeof(struct epoll_event) * ebkd->maxnpevents);
	}
	ebkd->pevents = (struct epoll_event *)tmp;
}

void gwrl_bkd_free(gwrl * rl) {
	gwrlbkd_epoll * ebkd = _gwrlbkde(rl->backend);
	close(ebkd->efd);
	free(ebkd->pevents);
	free(rl->backend);
	rl->backend = NULL;
}

void
gwrl_bkd_epoll_ctl(gwrl * rl, gwrlsrc_file * fsrc, int eflag) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwrlbkd_epoll * ebkd = _gwrlbkde(rl->backend);
	struct epoll_event ev;
	bzero(&ev,sizeof(ev));
	ev.data.ptr = fsrc;
	if(flisset(src->flags,GWRL_RD)) {
		ev.events |= EPOLLIN;
		ev.events |= EPOLLRDHUP;
	}
	if(flisset(src->flags,GWRL_WR)) {
		ev.events |= EPOLLOUT;
	}
	if((epoll_ctl(ebkd->efd,eflag,fsrc->fd,&ev)) < 0
		&& errno != EEXIST && errno != ENOENT) {
		gwprintsyserr("(dlB8d) epoll_ctl error",errno);
	}
}

void gwrl_src_file_update_flags(gwrl * rl, gwrlsrc * src, gwrlsrc_flags_t flags) {
	src->flags = flags;
	gwrl_bkd_epoll_ctl(rl,_gwrlsrcf(src),EPOLL_CTL_MOD);
}

void gwrl_bkd_src_add(gwrl * rl, gwrlsrc * src) {
	gwrl_bkd_epoll_ctl(rl,_gwrlsrcf(src),EPOLL_CTL_ADD);
}

void gwrl_bkd_del_src(gwrl * rl, gwrlsrc * src) {
	gwrl_bkd_epoll_ctl(rl,_gwrlsrcf(src),EPOLL_CTL_DEL);
}

void gwrl_bkd_enable_src(gwrl * rl, gwrlsrc * src) {
	gwrl_bkd_epoll_ctl(rl,_gwrlsrcf(src),EPOLL_CTL_ADD);
}

void gwrl_bkd_disable_src(gwrl * rl, gwrlsrc * src) {
	gwrl_bkd_epoll_ctl(rl,_gwrlsrcf(src),EPOLL_CTL_DEL);
}

void gwrl_bkd_gather(gwrl * rl) {
	int i = 0;
	int res = 0;
	gwrlevt_flags_t newflags = 0;
	struct epoll_event * event;
	long ms = 0;
	gwrlsrc * src = NULL;
	gwrlevt * evt = NULL;
	gwrlbkd_epoll * ebkd = _gwrlbkde(rl->backend);
	
	if(rl->backend->timeout.tv_sec == sec_min &&
		rl->backend->timeout.tv_nsec == nsec_min) {
		ms = -1;
	} else {
		gwtm_timespec_to_ms(&rl->backend->timeout,&ms);
	}
	
	if(ms < 0 && flisset(rl->flags,GWRL_NOSLEEP)) ms = 0;
	
	flset(rl->flags,GWRL_SLEEPING);
	res = epoll_wait(ebkd->efd,ebkd->pevents,ebkd->maxnpevents,ms);
	flclr(rl->flags,GWRL_SLEEPING);
	
	if(res == 0) return;
	
	if(res < 0 && (errno == EFAULT || errno == EBADF || EINVAL)) {
		gwprintsyserr("(3Xf9G) epoll_wait error",errno);
		return;
	}
	
	if(res > 0) {
		while(i < res) {
			newflags = 0;
			event = &(ebkd->pevents[i]);
			src = event->data.ptr;
			if(src->type == GWRL_SRC_TYPE_FILE) {
				if((src->flags & GWRL_RD) && (event->events & EPOLLIN)) newflags |= GWRL_RD;
				if((src->flags & GWRL_WR) && (event->events & EPOLLOUT)) newflags |= GWRL_WR;
			}
			if(newflags > 0) {
				evt = gwrl_evt_createp(rl,src,src->callback,src->userdata,_gwrlsrcf(src)->fd,newflags);
				gwrl_post_evt(rl,evt);
			}
			i++;
		}
		bzero(ebkd->pevents,sizeof(*ebkd->pevents)*i);
	}
}

#ifdef __cplusplus
}
#endif
