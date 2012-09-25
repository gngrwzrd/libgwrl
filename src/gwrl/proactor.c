
#include "gwrl/proactor.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void gwpr_src_activity(gwrl * rl, gwrlevt * evt);

gwpr *
gwpr_create(gwrl * rl) {
	gwpr * pr = (gwpr *)gwrl_mem_calloc(1,sizeof(gwpr));
	gwpr_options options = {
		GWPR_MAX_ACCEPT,
		GWPR_WRQUEUE_CACHE_MAX,
		GWPR_IOCP_OVLP_CACHE_MAX,
		GWPR_SYNCHRONOUS_WRITE_MAX_BYTES
	};
	if(!pr) {
		gwerr("(Dle3d) calloc error");
		return NULL;
	}
	pr->bufctl = gwprbufctl_create();
	if(!pr->bufctl) {
		free(pr);
		return NULL;
	}
	gwpr_set_options(pr,&options);
	pr->rl = rl;
	rl->pr = pr;
	return pr;
}

gwprdata *
gwprdata_create() {
	gwprdata * data = _gwprdata(gwrl_mem_calloc(1,sizeof(gwprdata)));
	if(!data) {
		gwerr("(Do4Kd) calloc error");
		return NULL;
	}
	data->rdbufsize = 512;
	data->rdfilters = NULL;
	data->wrfilters = NULL;
	if(GWPR_FILTERS_MAX > 0) {
		data->rdfilters = gwrl_mem_calloc(1,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
		if(!data->rdfilters) {
			free(data);
			return NULL;
		}
		data->wrfilters = gwrl_mem_calloc(1,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
		if(!data->wrfilters) {
			free(data);
			free(data->rdfilters);
			return NULL;
		}
	}
	return data;
}

gwrlsrc_file *
gwpr_set_fd(gwpr * pr, fileid_t fd, void * udata) {
	gwrlsrc_file * fsrc = NULL;
	gwrlsrc * src = gwrl_src_file_create(fd,0,0,NULL);
	while(!src) src = gwrl_src_file_create(fd,0,0,NULL);
	fsrc = (gwrlsrc_file *)src;
	src->userdata = udata;
	gwpr_src_add(pr,fsrc);
	return fsrc;
}

gwprbufctl *
gwprbufctl_create() {
	gwprbufctl * ctl = (gwprbufctl *)gwrl_mem_calloc(1,sizeof(gwprbufctl));
	if(!ctl) {
		gwerr("(p4P0R) calloc error");
		return NULL;
	}
	return ctl;
}

void
gwprbuf_free(gwprbufctl * bufctl, gwprbuf * buf) {
	free(buf->buf);
	free(buf);
}

void
gwpr_set_options(gwpr * pr, gwpr_options * opts) {
	memcpy(&pr->options,opts,sizeof(pr->options));
}

void
gwpr_src_add(gwpr * pr, gwrlsrc_file * fsrc) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwprdata * data = gwprdata_create();
	while(!data) data = gwprdata_create();
	src->flags = GWRL_RD; //read but disabled
	src->callback = gwpr_src_activity;
	fsrc->pdata = data;
	gwrl_src_add(pr->rl,src);
}

void
gwpr_src_add_safely(gwpr * pr, gwrlsrc_file * fsrc) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwprdata * data = gwprdata_create();
	while(!data) data = gwprdata_create();
	src->flags = GWRL_RD; //read but disabled
	src->callback = gwpr_src_activity;
	fsrc->pdata = data;
	gwrl_src_add_safely(pr->rl,src);
}

void
gwpr_src_remove(gwpr * pr, gwrlsrc_file * src) {
	if(src->pdata) free(src->pdata);
	gwrl_src_remove(pr->rl,(gwrlsrc *)src);
}

void
gwpr_src_del(gwpr * pr, gwrlsrc_file * src) {
	if(src->pdata) free(src->pdata);
	gwrl_src_del(pr->rl,(gwrlsrc *)src,NULL,true);
}

void
gwpr_filter_add(gwpr * pr, gwrlsrc_file * fsrc, gwpr_filter_id fid, gwpr_io_cb * fnc) {
	#if GWPR_FILTERS_MAX > 0
		int i = 0;
		bool added = false;
		gwprdata * pdata = _gwprdata(fsrc->pdata);
	#else
		gwerr("gwper_filter_add, no filter slots available.");
		return;
	#endif
	
	#if GWPR_FILTERS_MAX > 0
		if(fid == gwpr_rdfilter_id && GWPR_FILTERS_MAX > 0) {
			for(; i<GWPR_FILTERS_MAX; i++) {
				if(!pdata->rdfilters[i]) {
					pdata->rdfilters[i] = fnc;
					added = true;
					break;
				}
			}
		} else if(fid == gwpr_wrfilter_id && GWPR_FILTERS_MAX > 0) {
			for(; i<GWPR_FILTERS_MAX; i++) {
				if(!pdata->wrfilters[i]) {
					pdata->wrfilters[i] = fnc;
					added = true;
					break;
				}
			}
		}
		if(!added) gwerr("gwper_filter_add, no filter slots available.");
	#endif
}

void
gwpr_filter_reset(gwpr * pr, gwrlsrc_file * fsrc, gwpr_filter_id fid) {
	#if GWPR_FILTERS_MAX > 0
		gwprdata * pdata = _gwprdata(fsrc->pdata);
		if(GWPR_FILTERS_MAX > 0) {
			if(fid == gwpr_rdfilter_id && pdata->rdfilters) {
				bzero(pdata->rdfilters,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
			} else if(fid == gwpr_wrfilter_id && pdata->wrfilters) {
				bzero(pdata->wrfilters,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
			}
		}
	#endif
}

void
gwpr_set_cb(gwpr * pr, gwrlsrc_file * fsrc, gwpr_cb_id cbid, void * cb) {
	gwprdata * data = _gwprdata(fsrc->pdata);
	switch(cbid) {
	case gwpr_error_cb_id:
		data->errorcb = (gwpr_error_cb *)cb;
		break;
	case gwpr_accept_cb_id:
		data->acceptcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_connect_cb_id:
		data->connectcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_closed_cb_id:
		data->closedcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_did_read_cb_id:
		data->didreadcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_did_write_cb_id:
		data->didwritecb = (gwpr_io_cb *)cb;
		break;
	}
}

gwprbuf *
gwpr_buf_get(gwpr * pr, size_t size) {
	gwprbuf * data = (gwprbuf *)gwrl_mem_calloc(1,sizeof(gwprbuf));
	if(!data) {
		gwerr("(Erl3F) calloc error");
		return NULL;
	}
	data->buf = (char *)gwrl_mem_calloc(1,size);
	if(!data->buf) {
		gwerr("(RF4Hk) calloc error");
		free(data);
		return NULL;
	}
	data->buf = data->buf;
	data->bufsize = size;
	return data;
}

gwprbuf *
gwpr_buf_getp(gwpr * pr, size_t size) {
	gwprbuf * data = (gwprbuf *)gwrl_mem_calloc(1,sizeof(gwprbuf));
	while(!data) {
		gwerr("(Erl3F) calloc error");
		data = (gwprbuf *)gwrl_mem_calloc(1,sizeof(gwprbuf));
	}
	data->buf = (char *)gwrl_mem_calloc(1,size);
	while(!data->buf) {
		gwerr("(RF4Hk) calloc error");
		data->buf = (char *)gwrl_mem_calloc(1,size);
	}
	data->buf = data->buf;
	data->bufsize = size;
	return data;
}

gwprbuf *
gwpr_buf_get_tagged(gwpr * pr, size_t size, int tag) {
	gwprbuf * buf = gwpr_buf_get(pr,size);
	if(buf) buf->tag = tag;
	return buf;
}

gwprbuf *
gwpr_buf_get_with_data(gwpr * pr, size_t size, char * data, size_t datasize) {
	if(datasize > size) return NULL;
	gwprbuf * buf = gwpr_buf_getp(pr,size);
	memcpy(buf->buf,data,datasize);
	buf->len = datasize;
	return buf;
}

void
gwpr_buf_free(gwpr * pr, gwprbuf * buf) {
	gwprbuf_free(pr->bufctl,buf);
}


#ifdef __cplusplus
}
#endif
