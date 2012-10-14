
#include "gwrl/event.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
int asserts_var1 = 0;
bool asserts_var2 = 0;
#endif

gwrl *
gwrl_create() {
	gwrl * rl = _gwrl(gwrl_mem_calloc(1,sizeof(gwrl)));
	gwrl_options defaults = GWRL_DEFAULT_OPTIONS;
	
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!rl) {
		gwerr("(6GlI8) calloc error");
		return NULL;
	}
	#endif
	
	//setup default options.
	memcpy(&rl->options,&defaults,sizeof(defaults));
	
	//initialize the backend.
	rl->backend = gwrl_bkd_init(rl);
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!rl->backend) {
		free(rl);
		return NULL;
	}
	#endif
	
	//setup the custom gather functions.
	rl->gatherfncs = NULL;
	if(GWRL_GATHER_FUNCS_MAX > 0) {
		rl->gatherfncs = gwrl_mem_calloc(1,sizeof(gwrl_gather_fnc *) * GWRL_GATHER_FUNCS_MAX);
		#ifndef GWRL_HIDE_FROM_COVERAGE
		if(!rl->gatherfncs) {
			gwerr("(7VB3R) calloc error");
			free(rl);
			return NULL;
		}
		#endif
	}
	
	//initialize locks for thread safe functions.
	lockid_init(&rl->_qsrclk);
	lockid_init(&rl->_qevtlk);

	//initialize the backend waking mechanism.
	gwrl_wake_init(rl);
	return rl;
}

gwrlevt *
gwrl_evt_create(gwrl * rl, gwrlsrc * src, gwrlevt_cb * callback,
void * userdata, fileid_t fd, gwrlevt_flags_t flags) {
	gwrlevt * evt = NULL;
	if(rl->cevents) {
		evt = rl->cevents;
		rl->cevents = evt->next;
		rl->ncevents--;
		evt->next = NULL;
	} else {
		evt = _gwrlevt(gwrl_mem_calloc(1,sizeof(*evt)));
		#ifndef GWRL_HIDE_FROM_COVERAGE
		if(!evt) {
			gwerr("(8FxlC) calloc error");
			return NULL;
		}
		#endif
	}
	evt->callback = callback;
	evt->userdata = userdata;
	evt->src = src;
	evt->fd = fd;
	evt->flags = flags;
	return evt;
}

gwrlevt *
gwrl_evt_createp(gwrl * rl, gwrlsrc * src, gwrlevt_cb * callback,
void * userdata, fileid_t fd, gwrlevt_flags_t flags) {
	//creates a new event but keeps trying if it's null.
	gwrlevt * evt = gwrl_evt_create(rl,src,callback,userdata,fd,flags);
	while(!evt) {
		gwerr("(4LOP0) evt error");
		evt = gwrl_evt_create(rl,src,callback,userdata,fd,flags);
	}
	return evt;
}

gwrlsrc *
gwrl_src_time_create(int64_t ms, bool repeat, int whence,
bool persist, gwrlevt_cb * callback, void * userdata) {
	gwrlsrc_time * tsrc = _gwrlsrct(gwrl_mem_calloc(1,sizeof(gwrlsrc_time)));
	gwrlsrc * src = _gwrlsrc(tsrc);
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!tsrc) {
		gwerr("(5Gn3K) caloc error");
		return NULL;
	}
	#endif
	src->type = GWRL_SRC_TYPE_TIME;
	src->userdata = userdata;
	src->callback = callback;
	tsrc->ms = ms;
	if(repeat && whence != GWRL_ABS) flset(src->flags,GWRL_REPEAT);
	if(whence == GWRL_NOW) {
		tsrc->when.tv_sec = sec_min;
		tsrc->when.tv_nsec = nsec_min;
	}
	else if(whence == GWRL_ABS) {
		flset(src->flags,GWRL_WHENCE_ABS);
		gwtm_ms_to_timespec(ms,&tsrc->when);
	}
	flset(src->flags,GWRL_ENABLED);
	if(persist) flset(src->flags,GWRL_PERSIST);
	return src;
}

gwrlsrc *
gwrl_src_file_create(fileid_t fd, gwrlsrc_flags_t flags,
gwrlevt_cb * callback, void * userdata) {
	gwrlsrc_file * fsrc = _gwrlsrcf(gwrl_mem_calloc(1,sizeof(gwrlsrc_file)));
	gwrlsrc * src = _gwrlsrc(fsrc);
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!src) {
		gwerr("(25FnG) calloc error");
		return NULL;
	}
	#endif
	src->flags = flags;
	src->type = GWRL_SRC_TYPE_FILE;
	src->userdata = userdata;
	src->callback = callback;
	flset(src->flags,GWRL_ENABLED);
	fsrc->fd = fd;
	return src;
}

gwrlsrc *
gwrl_set_fd(gwrl * rl, fileid_t fd, gwrlsrc_flags_t flags,
gwrlevt_cb * callback, void * userdata) {
	gwrlsrc * fsrc = gwrl_src_file_create(fd,flags,callback,userdata);
	while(!fsrc) fsrc = gwrl_src_file_create(fd,flags,callback,userdata);
	gwrl_src_add(rl,fsrc);
	return fsrc;
}

gwrlsrc *
gwrl_set_timeout(gwrl * rl, int64_t ms, bool persist,
gwrlevt_cb * callback, void * userdata) {
	gwrlsrc * tsrc = gwrl_src_time_create(ms,false,GWRL_NOW,persist,callback,userdata);
	while(!tsrc) tsrc = gwrl_src_time_create(ms,false,GWRL_NOW,persist,callback,userdata);
	gwrl_src_add(rl,tsrc);
	return tsrc;
}

gwrlsrc *
gwrl_set_interval(gwrl * rl, int64_t ms, gwrlevt_cb * callback,
void * userdata) {
	gwrlsrc * tsrc = gwrl_src_time_create(ms,true,GWRL_NOW,false,callback,userdata);
	while(!tsrc) tsrc = gwrl_src_time_create(ms,true,GWRL_NOW,false,callback,userdata);
	gwrl_src_add(rl,tsrc);
	return tsrc;
}

gwrlsrc *
gwrl_set_date_timeout(gwrl * rl, int64_t ms, gwrlevt_cb * callback,
void * userdata) {
	gwrlsrc * tsrc = gwrl_src_time_create(ms,false,GWRL_ABS,false,callback,userdata);
	while(!tsrc) tsrc = gwrl_src_time_create(ms,false,GWRL_ABS,false,callback,userdata);
	gwrl_src_add(rl,tsrc);
	return tsrc;
}

void
gwrl_post_function(gwrl * rl, gwrlevt_cb * cb, void * userdata) {
	gwrlevt * evt = gwrl_evt_create(rl,NULL,cb,userdata,0,0);
	while(!evt) evt = gwrl_evt_create(rl,NULL,cb,userdata,0,0);
	gwrl_post_evt(rl,evt);
}

void
gwrl_post_function_safely(gwrl * rl, gwrlevt_cb * cb, void * userdata) {
	gwrlevt * evt = gwrl_evt_create(rl,NULL,cb,userdata,0,0);
	while(!evt) evt = gwrl_evt_create(rl,NULL,cb,userdata,0,0);
	gwrl_post_evt_safely(rl,evt);
}

void
gwrl_set_options(gwrl * rl, gwrl_options * opts) {
	//set runtime overridable options.
	if(opts->gwrl_gather_funcs_max > 0) {

		//this always callocs a new chunk of memory for the new options
		//and just copies old stuff in if needed. old memory is freed. this
		//is used so infrequently that using calloc vs realloc doesn't matter.

		int orig = rl->options.gwrl_gather_funcs_max;
		int diff = opts->gwrl_gather_funcs_max - orig;
		void * tmp = gwrl_mem_calloc(1,sizeof(gwrl_gather_fnc*) * opts->gwrl_gather_funcs_max);
		while(!tmp) tmp = gwrl_mem_calloc(1,sizeof(gwrl_gather_fnc*) * opts->gwrl_gather_funcs_max);
		
		if(diff < 0) {
			//the new memory chunk is smaller than the old (original) one.
			gwerr("(L3JF8) gather function(s) were truncated");
			memcpy(tmp,rl->gatherfncs,sizeof(gwrl_gather_fnc*) * opts->gwrl_gather_funcs_max);
		} else {
			//new memory chunk is same or bigger.
			memcpy(tmp,rl->gatherfncs,sizeof(gwrl_gather_fnc*) * orig);
		}

		//if there was an old gatherfncs just free it.
		if(rl->gatherfncs) {
			free(rl->gatherfncs);
		}
		
		rl->gatherfncs = (gwrl_gather_fnc **)tmp;
	} else {
		//all gatherfnc memory will be freed and not re-allocated.
		if(rl->gatherfncs) {
			gwerr("(35FH8) gather function(s) were all truncated");
			free(rl->gatherfncs);
			rl->gatherfncs = NULL;
		}
	}

	//copy new options into reactor.
	memcpy(&rl->options,opts,sizeof(rl->options));

	//let the backend operate on new options
	gwrl_bkd_set_options(rl,opts);
}

void
gwrl_add_gather_fnc(gwrl * rl, gwrl_gather_fnc * fnc) {
	int i = 0;
	bool added = false;
	for(; i < rl->options.gwrl_gather_funcs_max; i++) {
		if(!rl->gatherfncs[i]) {
			rl->gatherfncs[i] = fnc;
			added = true;
			break;
		}
	}
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!added) {
		gwerr("(3F85R) no open gather slots.");
		return;
	}
	#endif
}

void
gwrl_reset_gather_fncs(gwrl * rl) {
	if(rl->options.gwrl_gather_funcs_max > 0) {
		bzero(rl->gatherfncs,sizeof(gwrl_gather_fnc*)*rl->options.gwrl_gather_funcs_max);
	}
}

void
gwrl_free(gwrl * rl, gwrlsrc ** sources) {
	int i = 0;
	gwrlevt * del = NULL;
	gwrlevt * evt = NULL;
	gwrlsrc * head = NULL;
	gwrlsrc * last = NULL;
	gwrlsrc * delsrc = NULL;
	gwrlsrc * src = NULL;

	//make sure there is no proactor associated with the reactor.
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(rl && rl->pr) {
		gwerr("(RF4L3) gwrl_free error, you can't free a reactor before freeing the proactor.");
		return;
	}
	#endif

	//if rl is NULL, just free all sources passed in
	if(!rl && sources) {
		head = *sources;
		while(head) {
			delsrc = head;
			head = head->next;
			free(delsrc);
		}
		return;
	}

	//dispatch one last time, if events exist after
	//this they're ignored and freed.
	gwrl_dispatch(rl);
	evt = rl->events;
	
	//no events, set to cached events
	if(!evt) {
		evt = rl->cevents;
		rl->cevents = NULL;
	}
	
	//go through all events and free them
	while(evt) {
		del = evt;
		evt = evt->next;
		free(del);
		if(!evt) {
			evt = rl->cevents;
			rl->cevents = NULL;
		}
	}
	
	//loop over all input sources for each type. either
	//assembling the sources list for the user, or freeing them.
	for(; i<GWRL_SRC_TYPES_COUNT; i++) {
		src = rl->sources[i];
		rl->sources[i] = NULL;
		while(src) {
			delsrc = src;
			src = src->next;
			if(!sources) {
				free(delsrc);
			} else {
				delsrc->next = NULL;
				if(!head) {
					head = delsrc;
				}
				if(!last) {
					last = delsrc;
				} else {
					last->next = delsrc;
					last = delsrc;
				}
			}
		}
	}
	
	//get and free queued input sources
	if(rl->_qsrc) {
		lockid_lock(&rl->_qsrclk);
		src = rl->_qsrc;
		rl->_qsrc = NULL;
		lockid_unlock(&rl->_qsrclk);
		while(src) {
			delsrc = src;
			src = src->next;
			if(!sources) {
				free(delsrc);
			} else {
				delsrc->next = NULL;
				if(!head) {
					head = delsrc;
				}
				if(!last) {
					last = delsrc;
				} else {
					last->next = delsrc;
					last = delsrc;
				}
			}
		}
	}
	
	//get and free all queued events
	if(rl->_qevt) {
		lockid_lock(&rl->_qevtlk);
		evt = rl->_qevt;
		rl->_qevt = NULL;
		lockid_unlock(&rl->_qevtlk);
		while(evt) {
			del = evt;
			evt = evt->next;
			free(del);
		}
	}
	
	if(sources) {
		*sources = head;
	}
	lockid_free(&rl->_qevtlk);
	lockid_free(&rl->_qsrclk);
	gwrl_bkd_free(rl);
	gwrl_wake_free(rl);
	free(rl);
}

void
gwrl_evt_free(gwrl * rl, gwrlevt * evt) {
	//cache or free a gwrlevt
	if(rl->ncevents >= rl->options.gwrl_event_cache_max) {
		free(evt);
	} else {
		bzero(evt,sizeof(*evt));
		if(rl->cevents) {
			evt->next = rl->cevents;
		}
		rl->cevents = evt;
		rl->ncevents++;
	}
}

void
gwrl_evt_free_list(gwrl * rl, gwrlevt * evt) {
	gwrlevt * nev = NULL;
	while(evt) {
		nev = evt->next;
		gwrl_evt_free(rl,evt);
		evt = nev;
	}
}

void
gwrl_allow_poll_sleep(gwrl * rl, int onoff) {
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(onoff) flclr(rl->flags,GWRL_NOSLEEP);
	else flset(rl->flags,GWRL_NOSLEEP);
	#else
	flset(rl->flags,GWRL_NOSLEEP);
	#endif
}

void
gwrl_src_add(gwrl * rl, gwrlsrc * src) {
	gwrlsrc * head = rl->sources[src->type];
	if(head) src->next = head;
	rl->sources[src->type] = src;
	if(src->type == GWRL_SRC_TYPE_FILE) gwrl_bkd_src_add(rl,src);
	gwrl_wake(rl);
}

void
gwrl_src_add_safely(gwrl * rl, gwrlsrc * src) {
	gwrlsrc * head = NULL;
	lockid_lock(&rl->_qsrclk);
	head = rl->_qsrc;
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(head) src->next = head;
	else src->next = NULL;
	#endif
	rl->_qsrc = src;
	lockid_unlock(&rl->_qsrclk);
	if(rl->flags & GWRL_SLEEPING) gwrl_wake(rl);
}

void
gwrl_post_evt_safely(gwrl * rl, gwrlevt * evt) {
	gwrlevt * head = NULL;
	lockid_lock(&rl->_qevtlk);
	head = rl->_qevt;
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(head) evt->next = head;
	else evt->next = NULL;
	#endif
	rl->_qevt = evt;
	lockid_unlock(&rl->_qevtlk);
	if(rl->flags & GWRL_SLEEPING) gwrl_wake(rl);
}

void
gwrl_src_del(gwrl * rl, gwrlsrc * src, gwrlsrc * prev, bool freesrc) {
	gwrlsrc * head = rl->sources[src->type];
	gwrlsrc ** search = &head;

	if(prev) {
		prev->next = src->next;
		src->next = NULL;
	} else if(src == head) {
		rl->sources[src->type] = head->next;
		src->next = NULL;
	} else {
		while(*search != NULL) {
			if(*search == src) {
				*search = (*search)->next;
				src->next = NULL;
				break;
			} else {
				search = &(*search)->next;
			}
		}
	}

	if(src->type == GWRL_SRC_TYPE_FILE) gwrl_bkd_del_src(rl,src);
	if(freesrc) free(src);
}

void
gwrl_src_remove(gwrl * rl, gwrlsrc * src) {
	gwrl_src_del(rl,src,NULL,false);
}

void
gwrl_del_persistent_timeouts(gwrl * rl) {
	gwrlsrc * src = rl->sources[GWRL_SRC_TYPE_TIME];
	gwrlsrc * shead = src;
	gwrlsrc * phead = NULL;
	gwrlsrc * psrc = NULL;
	gwrlsrc * del = NULL;
	gwrlsrc * deltmp = NULL;
	rl->sources[GWRL_SRC_TYPE_TIME] = NULL;

	//first gather all the ones to free, and
	//take them out of the source list
	while(src) {
		if(src->flags & GWRL_PERSIST) {
			del = src;
			if(src == shead) shead = shead->next;
			if(psrc) psrc->next = src->next;
			if(!phead) phead = del;
			else phead->next = del, phead = del;
		}
		psrc = src;
		src = src->next;
	}

	//delete the persistent ones
	while(phead) {
		deltmp = phead->next;
		free(phead);
		phead = deltmp;
	}

	//replace time input sources with updated list
	rl->sources[GWRL_SRC_TYPE_TIME] = shead;
}

void
gwrl_post_evt(gwrl * rl, gwrlevt * evt) {
	//post an event
	if(rl->events) evt->next = rl->events;
	rl->events = evt;
}

void
gwrl_src_enable(gwrl * rl, gwrlsrc * src) {
	flset(src->flags,GWRL_ENABLED);
	if(src->type == GWRL_SRC_TYPE_TIME) {
		gwrlsrc_time * tsrc = (gwrlsrc_time *)src;
		if(!flisset(src->flags,GWRL_WHENCE_ABS)) {
			gwtm_gettimeofday_timespec(&tsrc->when);
			gwtm_add_ms_to_timespec(tsrc->ms,&tsrc->when);
		}
	} else if(src->type == GWRL_SRC_TYPE_FILE) {
		gwrl_bkd_enable_src(rl,src);
	}
	gwrl_wake(rl);
}

void
gwrl_src_disable(gwrl * rl, gwrlsrc * src) {
	flclr(src->flags,GWRL_ENABLED);
	if(src->type == GWRL_SRC_TYPE_FILE) gwrl_bkd_disable_src(rl,src);
	gwrl_wake(rl);
}

void
gwrl_stop(gwrl * rl) {
	flset(rl->flags,GWRL_STOP);
}

void
gwrl_time_gather(gwrl * rl) {
	bool update_tsrc = true;
	bool posted_event = false;
	struct timespec current;
	struct timespec timediff;
	gwrlsrc * src = NULL;
	gwrlsrc * del = NULL;
	gwrlsrc_time * tsrc_prev = NULL;
	gwrlsrc_time * tsrc = _gwrlsrct(rl->sources[GWRL_SRC_TYPE_TIME]);
	while(tsrc) {
		src = _gwrlsrc(tsrc);
		update_tsrc = true;
		if(!flisset(src->flags,GWRL_ENABLED)) goto update_tsrc_l;
		
		gwtm_gettimeofday_timespec(&current);
		
		if(!flisset(src->flags,GWRL_WHENCE_ABS) && 
			tsrc->when.tv_sec == sec_min && tsrc->when.tv_nsec == nsec_min) {
			gwtm_gettimeofday_timespec(&tsrc->when);
			gwtm_add_ms_to_timespec(tsrc->ms,&tsrc->when);
		}
		
		gwtm_timespec_sub_into(&tsrc->when,&current,&timediff);
		
		if(gwtm_timespec_is_expired(&timediff)) {
			gwrlevt * evt = gwrl_evt_create(rl,src,src->callback,src->userdata,0,0);
			while(!evt) evt = gwrl_evt_create(rl,src,src->callback,src->userdata,0,0);
			gwrl_post_evt(rl,evt);
			posted_event = true;
			
			if(flisset(src->flags,GWRL_REPEAT)) {
				
				gwtm_add_ms_to_timespec(tsrc->ms,&tsrc->when);
				gwtm_timespec_sub_into(&tsrc->when,&current,&timediff);
				
				if(rl->backend->timeout.tv_sec == sec_min && rl->backend->timeout.tv_nsec == nsec_min) {
					memcpy(&rl->backend->timeout,&timediff,sizeof(timediff));
				} else {
					gwtm_timespec_copy_if_smaller(&timediff,&rl->backend->timeout);
				}
				
			} else {
				
				if(flisset(src->flags,GWRL_PERSIST)) {
					gwrl_src_disable(rl,src);
					tsrc->when.tv_sec = sec_min;
					tsrc->when.tv_nsec = nsec_min;
				} else {
					del = src;
					tsrc = (gwrlsrc_time *)src->next;
					gwrl_src_del(rl,del,_gwrlsrc(tsrc_prev),true);
					tsrc_prev = NULL;
					update_tsrc = false;
				}
			}
		} else {
			if(rl->backend->timeout.tv_sec == sec_min && rl->backend->timeout.tv_nsec == nsec_min) {
				memcpy(&rl->backend->timeout,&timediff,sizeof(timediff));
			} else {
				gwtm_timespec_copy_if_smaller(&timediff,&rl->backend->timeout);
			}
		}
		
	update_tsrc_l:
		if(update_tsrc) {
			tsrc_prev = tsrc;
			tsrc = _gwrlsrct((_gwrlsrc(tsrc))->next);
		}
	}
	
	if(posted_event) {
		rl->backend->timeout.tv_sec = 0;
		rl->backend->timeout.tv_nsec = 0;
	}
}

void
gwrl_install_queued_sources(gwrl * rl) {
	gwrlsrc * src = rl->_qsrc;
	gwrlsrc * isrc = NULL;
	lockid_lock(&rl->_qsrclk);
	rl->_qsrc = NULL;
	lockid_unlock(&rl->_qsrclk);
	while(src) {
		isrc = src;
		src = src->next;
		gwrl_src_add(rl,isrc);
	}
}

void
gwrl_install_queued_events(gwrl * rl) {
	gwrlevt * evt = rl->_qevt;
	gwrlevt * pevt = NULL;
	lockid_lock(&rl->_qevtlk);
	rl->_qevt = NULL;
	lockid_unlock(&rl->_qevtlk);
	while(evt) {
		pevt = evt;
		evt = evt->next;
		gwrl_post_evt(rl,pevt);
	}
}

void
gwrl_dispatch(gwrl * rl) {
	uint8_t repost_count = 0;
	gwrlevt * evt = rl->events;
	gwrlevt * del = evt;
	rl->events = NULL;
	while(evt) {
		if(evt->callback) evt->callback(rl,evt);
		evt = evt->next;
		if(!evt && rl->events && repost_count < GWRL_REDISPATCH_MAX) {
			gwrl_evt_free_list(rl,del);
			repost_count++;
			evt = rl->events;
			del = evt;
			rl->events = NULL;
		}
	}
	if(del) gwrl_evt_free_list(rl,del);
}

void
gwrl_run_once(gwrl * rl) {
	int i = 0;
	if(rl->_qsrc) {
		gwrl_install_queued_sources(rl);
	}
	if(rl->_qevt) {
		gwrl_install_queued_events(rl);
	}
	gwrl_dispatch(rl);
	if(flisset(rl->flags,GWRL_STOP)) {
		flclr(rl->flags,GWRL_STOP);
		return;
	}
	rl->backend->timeout.tv_sec = sec_min;
	rl->backend->timeout.tv_nsec = nsec_min;
	if(rl->sources[GWRL_SRC_TYPE_TIME]) gwrl_time_gather(rl);
	gwrl_bkd_gather(rl);
	for(; i<rl->options.gwrl_gather_funcs_max; i++) {
		if(!rl->gatherfncs[i]) break;
		((gwrl_gather_fnc *)rl->gatherfncs[i])(rl);
	}
}

void
gwrl_run(gwrl * rl) {
	int i = 0;
	flclr(rl->flags,GWRL_STOP);
	for(;;) {
		if(rl->_qsrc) {
			gwrl_install_queued_sources(rl);
		}
		if(rl->_qevt) {
			gwrl_install_queued_events(rl);
		}
		gwrl_dispatch(rl);
		if(flisset(rl->flags,GWRL_STOP)) {
			flclr(rl->flags,GWRL_STOP);
			break;
		}
		rl->backend->timeout.tv_sec = sec_min;
		rl->backend->timeout.tv_nsec = nsec_min;
		if(rl->sources[GWRL_SRC_TYPE_TIME]) gwrl_time_gather(rl);
		gwrl_bkd_gather(rl);
		for(i=0; i<rl->options.gwrl_gather_funcs_max; i++) {
			if(!rl->gatherfncs[i]) break;
			((gwrl_gather_fnc *)rl->gatherfncs[i])(rl);
		}
	}
}

#ifdef __cplusplus
}
#endif
