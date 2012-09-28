
#include "gwrl/event.h"

#ifdef HAVE_FCNTL
#	define fcntl_is_valid_fd(fd)( fcntl(fd,F_GETFL) !=-1 || errno!= EBADF )
#endif

//select backend for gwrl->backend
typedef struct _gwrlbkd_select {
	gwrlbkd _;     //base structure
	int maxfd;     //max fd for select
	fd_set src[3]; //src fdset for select
} gwrlbkd_select;

#ifdef __cplusplus
extern "C" {
#endif

void gwrl_src_file_find_badfd_post_evt(gwrl * rl);
void gwrl_bkd_set_options(gwrl * rl,gwrl_options * opts){};

gwrlbkd * gwrl_bkd_init(gwrl * rl) {
	gwrlbkd_select * sbkd = _gwrlbkds(gwrl_mem_calloc(1,sizeof(gwrlbkd_select)));
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!sbkd) {
		gwprintsyserr("(p3F7r) calloc error",errno);
		return NULL;
	}
	#endif
	sbkd->maxfd = -1;
	FD_ZERO(&sbkd->src[0]);
	FD_ZERO(&sbkd->src[1]);
	FD_ZERO(&sbkd->src[2]);
	return _gwrlbkd(sbkd);
}

void gwrl_bkd_free(gwrl * rl) {
	free(rl->backend);
	rl->backend = NULL;
}

void gwrl_src_file_find_badfd_post_evt(gwrl * rl) {
	gwrlsrc_file * fsrc = _gwrlsrcf(rl->sources[GWRL_SRC_TYPE_FILE]);
	gwrlsrc * src = rl->sources[GWRL_SRC_TYPE_FILE];
	while(fsrc) {
		if(!fcntl_is_valid_fd(fsrc->fd)) {
			gwrlevt * evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,GWRL_RD);
			while(!evt) evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,GWRL_RD);
			gwrl_post_evt(rl,evt);
			gwrl_src_disable(rl,_gwrlsrc(fsrc));
		}
		fsrc = _gwrlsrcf(src->next);
	}
}

void gwrl_src_file_update_flags(gwrl * rl, gwrlsrc * src, gwrlsrc_flags_t flags) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	gwrlbkd_select * sbkd = _gwrlbkds(rl->backend);
	FD_CLR(fsrc->fd,&sbkd->src[0]);
	FD_CLR(fsrc->fd,&sbkd->src[1]);
	FD_CLR(fsrc->fd,&sbkd->src[2]);
	if(flags & GWRL_RD) FD_SET(fsrc->fd,&sbkd->src[0]);
	if(flags & GWRL_WR) FD_SET(fsrc->fd,&sbkd->src[1]);
	FD_SET(fsrc->fd,&sbkd->src[2]);
	src->flags = flags;
}

void gwrl_bkd_recalc_srcs(gwrl * rl) {
	gwrlsrc * _src = NULL;
	gwrlsrc_file * fsrc = _gwrlsrcf(rl->sources[GWRL_SRC_TYPE_FILE]);
	gwrlbkd_select * sbkd = _gwrlbkds(rl->backend);
	sbkd->maxfd = -1;
	while(fsrc) {
		_src = _gwrlsrc(fsrc);
		if(!flisset(_src->flags,GWRL_ENABLED)) {
			fsrc = _gwrlsrcf(_src->next);
			continue;
		}
		if(fsrc->fd > sbkd->maxfd) sbkd->maxfd = fsrc->fd;
		fsrc = _gwrlsrcf(_src->next);
	}
}

void gwrl_bkd_src_add(gwrl * rl, gwrlsrc * src) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	gwrlbkd_select * sbkd = _gwrlbkds(rl->backend);
	FD_CLR(fsrc->fd,&(sbkd->src[0]));
	FD_CLR(fsrc->fd,&(sbkd->src[1]));
	FD_CLR(fsrc->fd,&(sbkd->src[2]));
	if(src->flags & GWRL_RD) FD_SET(fsrc->fd,&(sbkd->src[0]));
	if(src->flags & GWRL_WR) FD_SET(fsrc->fd,&(sbkd->src[1]));
	FD_SET(fsrc->fd,&(sbkd->src[2]));
	if(fsrc->fd > sbkd->maxfd) sbkd->maxfd = fsrc->fd;
}

void gwrl_bkd_del_src(gwrl * rl, gwrlsrc * src) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	//select can't easily be removed. the fd can be cleared
	//from the FD_SET but otherwise it has to be recalc'd.
	gwrlbkd_select * sbkd = _gwrlbkds(rl->backend);
	FD_CLR(fsrc->fd,&(sbkd->src[0]));
	FD_CLR(fsrc->fd,&(sbkd->src[1]));
	FD_CLR(fsrc->fd,&(sbkd->src[2]));
	gwrl_bkd_recalc_srcs(rl);
}

void gwrl_bkd_enable_src(gwrl * rl, gwrlsrc * src) {
	//select can't easily be re-enabled, has to be recalc'd
	gwrl_bkd_recalc_srcs(rl);
}

void gwrl_bkd_disable_src(gwrl * rl, gwrlsrc * src) {
	//select can't easily be disabled, have to recalc.
	gwrl_bkd_recalc_srcs(rl);
}

void gwrl_bkd_gather(gwrl * rl) {
	int res = 0;
	gwrlevt_flags_t newflags = 0;
	fd_set fds[3];
	gwrlsrc * src = NULL;
	gwrlsrc_file * fsrc = NULL;
	gwrlevt * evt = NULL;
	gwrlbkd * bkd = rl->backend;
	gwrlbkd_select * sbkd = _gwrlbkds(bkd);
	
	//setup timeout for select
	struct timeval timeout;
	struct timeval * timeoutp = &timeout;
	if(bkd->timeout.tv_sec == sec_min && bkd->timeout.tv_nsec == nsec_min) {
		timeoutp = NULL;
	} else {
		gwtm_timespec_to_timeval(&bkd->timeout,&timeout);
	}
	
	//if sleep isn't allowed set timeout to 0
	if(!timeoutp && flisset(rl->flags,GWRL_NOSLEEP)) {
		timeoutp = &timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
			if(asserts_var1 == gwrlbkd_no_sleep_assert_true) {
				asserts_var2 = true;
			}
		#endif
	}
	
	//initialize fds
	FD_ZERO(&(fds[0]));
	FD_ZERO(&(fds[1]));
	FD_ZERO(&(fds[2]));
	
	//reset fds
	memcpy(&(fds[0]),&(sbkd->src[0]),sizeof(fd_set));
	memcpy(&(fds[1]),&(sbkd->src[1]),sizeof(fd_set));
	memcpy(&(fds[2]),&(sbkd->src[2]),sizeof(fd_set));
	
	//call select, wrapped in sleeping flags so other threads can wake us
	flset(rl->flags,GWRL_SLEEPING);
	res = select(sbkd->maxfd+1,&(fds[0]),&(fds[1]),&(fds[2]),timeoutp);
	flclr(rl->flags,GWRL_SLEEPING);
	
	if(res == 0) return; //timeout
	
	//break and let the event loop continue or start over.
	//if a signal did happen, the event loop may have an
	//event that needs processing.
	if(res < 0 && (errno == EINTR || errno == EAGAIN)) return;
	
	//bad fd, unforunately select doesn't tell us which
	//one it was so we have to search for it.
	if(res < 0 && errno == EBADF) {
		gwrl_src_file_find_badfd_post_evt(rl);
		return;
	}
	
	//invalid timeout or invalid number of fds.
	if(res < 0 && errno == EINVAL) {
		//invalid timeout, break and let process events recalculate timeouts.
		if(timeout.tv_sec < 0 || timeout.tv_usec < 0) return;
		
		//nfds parameter to select() is too large, not sure how to handle
		fprintf(stderr,"select: file descriptor limit reached. exiting. \n");
		exit(1);
	}
	
	//valid events are ready
	if(res > 0) {
		fsrc = _gwrlsrcf(rl->sources[GWRL_SRC_TYPE_FILE]);
		while(fsrc) {
			src = _gwrlsrc(fsrc);
			newflags = 0;
			
			if(!flisset(src->flags,GWRL_ENABLED)) {
				fsrc = _gwrlsrcf(src->next);
				continue;
			}
			
			if(FD_ISSET(fsrc->fd,&fds[0])) flset(newflags,GWRL_RD);
			if(FD_ISSET(fsrc->fd,&fds[1])) flset(newflags,GWRL_WR);
			if(FD_ISSET(fsrc->fd,&fds[2])) flset(newflags,GWRL_RD);
			
			if(newflags > 0) {
				evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,newflags);
				while(!evt) evt = gwrl_evt_create(rl,src,src->callback,src->userdata,fsrc->fd,newflags);
				gwrl_post_evt(rl,evt);
			}
			
			fsrc = _gwrlsrcf(src->next);
		}
	}
}

#ifdef __cplusplus
}
#endif
