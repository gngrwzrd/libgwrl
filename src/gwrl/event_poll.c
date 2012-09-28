
#include "gwrl/event.h"

//the initial struct pollfd * size and grow by size to accept
//events from poll backend
#ifndef GWRL_POLLFD_COUNT
#	define GWRL_POLLFD_COUNT 64
#endif

//poll backend for gwrl->backend
typedef struct _gwrlbkd_poll {
	gwrlbkd _;           //base structure
	uint16_t nfds;       //num of fds
	uint16_t maxnfds;    //size of .fds below
	int lkp_maxfd;       //max fd in the lkp_src
	gwrlsrc ** lkp_src;  //lookup by fd => src
	struct pollfd * fds; //for poll
} gwrlbkd_poll;

#ifdef __cplusplus
extern "C" {
#endif

gwrlbkd * gwrl_bkd_init(gwrl * rl) {
	gwrlbkd_poll * pbkd = _gwrlbkdp(gwrl_mem_calloc(1,sizeof(gwrlbkd_poll)));
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!pbkd) {
		gwprintsyserr("(pe1Ij) calloc error",errno);
		return NULL;
	}
	#endif
	pbkd->nfds = 0;
	pbkd->maxnfds = GWRL_POLLFD_COUNT;
	pbkd->fds = (struct pollfd *)gwrl_mem_calloc(pbkd->maxnfds,sizeof(struct pollfd));
	if(!pbkd->fds) {
		gwprintsyserr("(poEi4) calloc error",errno);
		free(pbkd);
		return NULL;
	}
	pbkd->lkp_maxfd = -1;
	pbkd->lkp_src = NULL;
	rl->options.gwrl_pollfd_count = pbkd->maxnfds;
	return _gwrlbkd(pbkd);
}

void gwrl_bkd_set_options(gwrl * rl, gwrl_options * opts) {
	if(opts->gwrl_pollfd_count > rl->options.gwrl_pollfd_count) {
		gwrlbkd_poll * pbkd = _gwrlbkdp(rl->backend);
		int diff = rl->options.gwrl_pollfd_count - opts->gwrl_pollfd_count;
		void * tmp = gwrl_mem_realloc(pbkd->fds,sizeof(struct pollfd) * (pbkd->maxnfds + diff));
		while(!tmp) {
			gwprintsyserr("(6FB3D) calloc error",errno);
			tmp = gwrl_mem_realloc(pbkd->fds,sizeof(struct pollfd) * pbkd->maxnfds+diff);
		}
		pbkd->fds = tmp;
		rl->options.gwrl_pollfd_count = opts->gwrl_pollfd_count;
	}
}

void gwrl_bkd_free(gwrl * rl) {
	gwrlbkd_poll * pbkd = _gwrlbkdp(rl->backend);
	free(pbkd->fds);
	free(pbkd->lkp_src);
	free(rl->backend);
	rl->backend = NULL;
}

void gwrl_src_file_update_flags(gwrl * rl, gwrlsrc * src, gwrlsrc_flags_t flags) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	fsrc->pfd->events = 0;
	if(flisset(flags,GWRL_RD)) fsrc->pfd->events |= POLLIN;
	if(flisset(flags,GWRL_WR)) fsrc->pfd->events |= POLLOUT;
	src->flags = flags;
}

void gwrl_bkd_recalc_srcs(gwrl * rl) {
	gwrlsrc * src = NULL;
	gwrlbkd_poll * pbkd = _gwrlbkdp(rl->backend);
	gwrlsrc_file * fsrc = _gwrlsrcf(rl->sources[GWRL_SRC_TYPE_FILE]);
	struct pollfd * pfd = &(pbkd->fds[0]);
	bzero(pbkd->fds,sizeof(struct pollfd) * pbkd->nfds);
	pbkd->nfds = 0;
	while(fsrc) {
		src = _gwrlsrc(fsrc);
		if(!flisset(src->flags,GWRL_ENABLED)) {
			fsrc = (gwrlsrc_file *)src->next;
			continue;
		}
		pbkd->nfds++;
		pfd->fd = fsrc->fd;
		if(src->flags & GWRL_RD) pfd->events |= POLLIN;
		if(src->flags & GWRL_WR) pfd->events |= POLLOUT;
		fsrc->pfd = pfd;
		pfd++;
		fsrc = _gwrlsrcf(src->next);
	}
}

void gwrl_bkd_del_src(gwrl * rl, gwrlsrc * src) {
	gwrlsrc_file * fsrc = (gwrlsrc_file *)src;
	//poll can't easily be uninstalled. the fd=>src lookup
	//can be removed but otherwise it has to be recalc'd.
	gwrlbkd_poll * pbkd = _gwrlbkdp(rl->backend);
	fsrc->pfd = NULL;
	pbkd->lkp_src[fsrc->fd] = NULL;
	gwrl_bkd_recalc_srcs(rl);
}

void gwrl_bkd_enable_src(gwrl * rl, gwrlsrc * src) {
	flset(src->flags,GWRL_ENABLED);
	//poll can't easily be re-enabled, has to be recalc'd
	gwrl_bkd_recalc_srcs(rl);
}

void gwrl_bkd_disable_src(gwrl * rl, gwrlsrc * src) {
	flclr(src->flags,GWRL_ENABLED);
	//poll can't easily be disabled, have to recalc.
	gwrl_bkd_recalc_srcs(rl);
}

void gwrl_bkd_src_add(gwrl * rl, gwrlsrc * src) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	gwrlbkd_poll * pbkd = _gwrlbkdp(rl->backend);
	
	if(pbkd->nfds == pbkd->maxnfds) {
		pbkd->maxnfds += rl->options.gwrl_pollfd_count;
		void * tmp = gwrl_mem_realloc(pbkd->fds, sizeof(struct pollfd)*pbkd->maxnfds);
		while(!tmp) {
			gwprintsyserr("(3l8FG) realloc error",errno);
			tmp = gwrl_mem_realloc(pbkd->fds, sizeof(struct pollfd)*pbkd->maxnfds);
		}
		pbkd->fds = tmp;
	}
	
	//update the pollfd for this src
	struct pollfd * pfd = &(pbkd->fds[pbkd->nfds]);
	fsrc->pfd = pfd;
	pbkd->nfds++;
	pfd->fd = fsrc->fd;
	pfd->revents = 0;
	if(src->flags & GWRL_RD) pfd->events |= POLLIN;
	if(src->flags & GWRL_WR) pfd->events |= POLLOUT;
	
	//update fd => src lookup
	//the +1 in a few places below is to account for 0 based indexes.
	if(pfd->fd > pbkd->lkp_maxfd) {
		pbkd->lkp_maxfd = pfd->fd;
		if(pbkd->lkp_src) {
			void * tmp = gwrl_mem_realloc(pbkd->lkp_src, sizeof(struct pollfd) * pbkd->lkp_maxfd+1);
			while(!tmp) {
				gwprintsyserr("(3lUv4) realloc error",errno);
				tmp = gwrl_mem_realloc(pbkd->lkp_src, sizeof(struct pollfd) * pbkd->lkp_maxfd+1);
			}
			pbkd->lkp_src = tmp;
		} else {
			pbkd->lkp_src = gwrl_mem_calloc(pbkd->lkp_maxfd+1,sizeof(struct pollfd));
			while(!pbkd->lkp_src) {
				gwprintsyserr("(4kKeR) calloc error",errno);
				pbkd->lkp_src = gwrl_mem_calloc(pbkd->lkp_maxfd+1,sizeof(struct pollfd));
			}
		}
	}
	pbkd->lkp_src[pfd->fd] = _gwrlsrc(fsrc);
}

void gwrl_bkd_gather(gwrl * rl) {
	int res = 0;
	int64_t ms = 0;
	uint16_t i = 0;
	struct pollfd * pfd;
	gwrlevt_flags_t newflags = 0;
	gwrlsrc * src = NULL;
	gwrlevt * evt = NULL;
	gwrlbkd_poll * pbkd = _gwrlbkdp(rl->backend);
	
	//check timeout for indefinite waiting
	if(rl->backend->timeout.tv_sec == sec_min &&
		rl->backend->timeout.tv_nsec == nsec_min) {
		ms = -1;
	} else {
		gwtm_timespec_to_ms(&rl->backend->timeout,&ms);
	}
	
	//check and see if we're allowed to sleep in sys calls when no timeout is set.
	if(ms < 0 && flisset(rl->flags,GWRL_NOSLEEP)) {
		ms = 0;
		#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
			if(asserts_var1 == gwrlbkd_no_sleep_assert_true) {
				asserts_var2 = true;
			}
		#endif
	}
		
	//poll, wrap in sleep flags so other threads can wake us.
	flset(rl->flags,GWRL_SLEEPING);
	res = poll(pbkd->fds,pbkd->nfds,ms);
	flclr(rl->flags,GWRL_SLEEPING);
	
	if(res == 0) return; //timeout
	
	//break and let the event loop continue or start over.
	//if a signal did happen, the event loop may have an
	//event that needs processing.
	if(res < 0 && (errno == EINTR || errno == EAGAIN)) return;
	
	//invalid timeout or ndfs too large
	if(res < 0 && errno == EINVAL) {
		if(ms < -1) return; //invalid timeout, let process events fix it
		
		//ndfs is greater than system allowed
		gwprintsyserr("(EJ4KD) poll file descriptor limit reached. exiting.",errno);
		exit(-1);
	}
	
	//valid events ready
	if(res > 0) {
		while(i < pbkd->nfds) {
			newflags = 0;
			pfd = &(pbkd->fds[i]);
			src = pbkd->lkp_src[pfd->fd];
			
			if(!flisset(src->flags,GWRL_ENABLED)) {
				i++;
				continue;
			}
			
			if(pfd->revents & POLLNVAL) {
				flset(newflags,GWRL_RD);
				gwrl_src_disable(rl,src);
			}
			
			if(pfd->revents & POLLHUP) {
				flset(newflags,GWRL_RD);
			}
			
			if(pfd->revents & POLLERR) {
				flset(newflags,GWRL_RD);
			}
			
			if(pfd->revents & POLLIN) {
				flset(newflags,GWRL_RD);
			}
			
			if(pfd->revents & POLLOUT) {
				flset(newflags,GWRL_WR);
			}
			
			if(newflags > 0) {
				evt = gwrl_evt_create(rl,src,src->callback,src->userdata,_gwrlsrcf(src)->fd,newflags);
				while(!evt) evt = gwrl_evt_create(rl,src,src->callback,src->userdata,_gwrlsrcf(src)->fd,newflags);
				gwrl_post_evt(rl,evt);
			}
			
			i++;
		}
	}
}

#ifdef __cplusplus
}
#endif
