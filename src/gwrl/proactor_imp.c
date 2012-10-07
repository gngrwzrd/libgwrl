
#include "gwrl/proactor.h"

#ifdef __cplusplus
extern "C" {
#endif

void
io_activity(gwpr * pr, gwpr_io_info * info) {
}

void 
gwpr_src_activity(gwrl * rl, gwrlevt * evt) {
	if(evt->src->type == GWRL_SRC_TYPE_FILE) {
		char errfnc[16];
		gwpr * pr = _gwpr(rl->pr);
		gwrlsrc * src = evt->src;
		gwrlsrc_file * fsrc = _gwrlsrcf(evt->src);
		gwprdata * pdata = fsrc->pdata;
		gwpr_error_info errinfo = {0};
		gwpr_io_info ioinfo = {0};
		bzero(errfnc,sizeof(errfnc));
		
		if(evt->flags & GWRL_RD) {
			int i = 0;
			ssize_t res = 0;
			gwprbuf * rd = NULL;

			if(pdata->acceptcb) {
				//an accept callback is set, assume they want us to
				//try and accept as many clients up to gwpr_max_accept.
				ioinfo.src = fsrc;
				for(; i<pr->options.gwpr_max_accept; i++) {
					res = accept(evt->fd,_sockaddr(&(ioinfo.peer)),&(ioinfo.peerlen));
					if(res > -1) {
						ioinfo.peersrc = _gwrlsrcf(gwrl_src_file_create(res,0,NULL,NULL));
						pdata->acceptcb(pr,&ioinfo);
					} else if(res < 0) {
						if(errno != EWOULDBLOCK) {
							if(pdata->errorcb) {
								errinfo.src = fsrc;
								errinfo.errnm = errno;
								memcpy(errinfo.fnc,"accept\0",7);
								pdata->errorcb(pr,&errinfo);
							} else {
								gwprintsyserr("(09F6R) accept() error occured with no error callback.",errno);
							}
						}
					}
				}
			}
			
			//clear info objects from past use
			bzero(&ioinfo,sizeof(ioinfo));
			bzero(&errinfo,sizeof(errinfo));

			if(pdata->didreadcb) {
				//did read callback is set so let's try and read data.

				//get or create a read buffer.
				rd = pdata->rdbuf;
				while(!rd) rd = gwpr_buf_get(pr,pdata->rdbufsize);
				pdata->rdbuf = NULL;
				ioinfo.src = fsrc;
				ioinfo.buf = rd;

				if(pdata->rdop == gwpr_read_op_id) {
					ioinfo.op = gwpr_read_op_id;
					memcpy(errfnc,"read\0",5);
					while((res=read(fsrc->fd,rd->buf,rd->bufsize)) < 0 && errno==EINTR);
				}
				
				else if(pdata->rdop == gwpr_recvfrom_op_id) {
					ioinfo.op = gwpr_recvfrom_op_id;
					memcpy(errfnc,"recvfrom\0",9);
					ioinfo.peerlen = sizeof(ioinfo.peer);
					while((res=recvfrom(fsrc->fd,rd->buf,rd->bufsize,0,
						_sockaddr(&ioinfo.peer),&ioinfo.peerlen)) < 0 && errno == EINTR);
				}
				
				else if(pdata->rdop == gwpr_recv_op_id) {
					ioinfo.op = gwpr_recv_op_id;
					memcpy(errfnc,"recv\0",5);
					while((res=recv(fsrc->fd,rd->buf,rd->bufsize,0)) < 0 && errno == EINTR);
				}
				
				if(res > 0) {
					//good read, call rd filters and did_read callback.
					
					rd->len = res;
					if(pdata->rdfilters && pdata->rdfilters[0] != NULL) {
						for(i=0; i<GWPR_FILTERS_MAX; i++) {
							if(!pdata->rdfilters[i]) break;
							pdata->rdfilters[i](pr,&ioinfo);
						}
					}
					
					pdata->didreadcb(pr,&ioinfo);
				}
				
				else if(res == 0 && pdata->closedcb) {
					//closed socket or eof from file
					pdata->closedcb(pr,&ioinfo);
				}
				
				else if(res < 0 && (errno == ECONNRESET || errno == EPIPE)
					&& pdata->closedcb) {
					//closed socket or pipe error
					pdata->closedcb(pr,&ioinfo);
				}
				
				else if(res < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
					//socket or file marked as non blocking,
					//no data to read, nothing to do.
				}
				
				else if(res < 0 && pdata->errorcb) {
					//error we don't want to handle, pass to users.
					errinfo.errnm = errno;
					errinfo.src = fsrc;
					errinfo.op = ioinfo.op;
					memcpy(errinfo.fnc,errfnc,sizeof(errfnc));
					pdata->errorcb(pr,&errinfo);
				}

				else {
					gwprintsyserr("(OKL4F) proactor read error with no error callback",errno);
				}
			} else if(!pdata->didreadcb && !pdata->acceptcb) {
				gwerr("(4FL9KF) proactor can read data but there's no read callback set.");
			}
		}
		
		if(evt->flags & GWRL_WR) {
			//writable activity
			
			if(pdata->connectcb) {
				//the user wanted to know when a socket is writable after
				//calling connect(), call it here and disable the connectcb.
				ioinfo.src = fsrc;
				ioinfo.peerlen = sizeof(ioinfo.peer);
				getpeername(fsrc->fd,_sockaddr(&ioinfo.peer),&ioinfo.peerlen);
				pdata->connectcb(pr,&ioinfo);
				pdata->connectcb = NULL;
			}
			
			if(pdata->didwritecb) {
				//write callback is set, let's try and write some data.
				int errnm = 0;
				bool stopwrite = false;
				bool freebuf = false;
				size_t written = 0;
				gwprwrq * q = pdata->wrq;
				gwprwrq * qn = NULL;
				gwprbuf * rembuf = NULL;
				pdata->wrq = NULL;
				if(pdata->didwritecb == &io_activity) freebuf = true;
				bzero(&ioinfo,sizeof(ioinfo));
				bzero(&errinfo,sizeof(errinfo));
				ioinfo.src = fsrc;
				
				while(q) {
					//loop over the gwprwrq objects associated with the file
					//input source - this is all the data queued for writing.
					
					qn = q->next;
					ioinfo.op = q->wrop;
					ioinfo.buf = q->buf;
					if(q->wrop == gwpr_sendto_op_id) {
						ioinfo.peerlen = q->peerlen;
						memcpy(&ioinfo.peer,&q->peer,q->peerlen);
					} else {
						ioinfo.peerlen = 0;
						bzero(&ioinfo.peer,sizeof(ioinfo.peer));
					}
					
					if(pdata->wrfilters && pdata->wrfilters[0] != NULL) {
						//call write filters before we send the data.
						int i = 0;
						for(; i< GWPR_FILTERS_MAX; i++) {
							if(!pdata->wrfilters[i]) break;
							pdata->wrfilters[i](pr,&ioinfo);
						}
					}
					
					//update the error fnc in case we need it.
					if(q->wrop == gwpr_write_op_id) memcpy(errfnc,"write\0",6);
					else if(q->wrop == gwpr_sendto_op_id) memcpy(errfnc,"sendto\0",7);
					else if(q->wrop == gwpr_send_op_id) memcpy(errfnc,"send\0",5);
					
					//perform the write.
					gwpr_write_buffer(pr,fsrc,q->buf,q->wrop,&q->peer,q->peerlen,&written,&errnm);
					
					if(written == q->buf->len) {
						//successfully wrote all data.
						pdata->didwritecb(pr,&ioinfo);
					}
					
					else if(written > 0 && written < q->buf->len) {
						//partial write. the unwritten data is copied to a new
						//buffer to put back in the write queue. notify the user
						//of the partial write - only the written data is reported.
						size_t remain = q->buf->len - written;
						rembuf = gwpr_buf_get(pr,remain);
						while(!rembuf) rembuf = gwpr_buf_get(pr,remain);
						memcpy(rembuf->buf,q->buf->buf+written,remain);
						q->buf->len = written;
						pdata->didwritecb(pr,&ioinfo);
						
						if(errnm == 0 || errnm == EWOULDBLOCK || errnm == EAGAIN) {
							//all is ok, but since it's a partial write the
							//queue has to be put back for writing again later
							gwprwrq * rembufq = gwprwrq_get(pr,fsrc);
							memcpy(&rembufq->peer,&q->peer,sizeof(rembufq->peer));
							rembufq->peerlen = q->peerlen;
							rembufq->buf = rembuf;
							rembufq->wrop = q->wrop;
							rembufq->next = q->next;
							gwprwrq_putback(pr,fsrc,rembufq);
							break;
						}
					}

					if(errnm == ECONNRESET || errnm == EPIPE) {
						//closed connection or pipe write with no read side connected.
						//buf is nulled out because we should only give the user data
						//that had been unwritten in the case of errors or closed fd.
						if(written > 0) ioinfo.buf = rembuf;
						else ioinfo.buf = NULL;
						if(pdata->closedcb) pdata->closedcb(pr,&ioinfo);
						break;
					}

					else if(written == 0 && (errnm == EWOULDBLOCK || errnm == EAGAIN)) {
						//socket or file is non blocking and this would block.
						//the socket or file is not-writable so restore the write
						//queue starting from this q object for writing later.
						gwprwrq_putback(pr,fsrc,q);
						break;
					}

					else if(errnm != 0 && pdata->errorcb) {
						//error we don't want to handle. pass to the user.
						errinfo.errnm = errno;
						errinfo.src = fsrc;
						errinfo.op = q->wrop;
						errinfo.buf = q->buf;
						if(written > 0) errinfo.buf = rembuf;
						memcpy(errinfo.fnc,errfnc,sizeof(errfnc));
						pdata->errorcb(pr,&errinfo);
						stopwrite = true;
						gwprwrq_putback(pr,fsrc,q);
						break;
					}
					
					if(freebuf) {
						//there was no user provided did_write callback set, so
						//the buffer that's associated with a write queue item will
						//get lost unless freed here.
						gwpr_buf_free(pr,q->buf);
					}

					//give back the gwprwrq
					gwprwrq_free(pr,fsrc,q);
					q = qn;
				}
				
				if(!pdata->wrq || stopwrite) {
					//stop write events since there's nothing in the queue
					gwrlsrc_flags_t flags = src->flags;
					flclr(flags,GWRL_WR);
					gwrl_src_file_update_flags(pr->rl,src,flags);
				}
			}
		}
		
		else if(evt->flags & GWRL_SYNC_WRITE) {
			if(pdata->didwritecb) {
				//a synchronous write completed, notify the user here.
				gwprwrq * q = (gwprwrq *)evt->userdata;
				ioinfo.src = fsrc;
				ioinfo.buf = q->buf;
				ioinfo.op = q->wrop;
				if(q->wrop == gwpr_write_op_id) {
					memcpy(&ioinfo.peer,&q->peer,sizeof(ioinfo.peer));
					ioinfo.peerlen = q->peerlen;
				} else {
					bzero(&ioinfo.peer,sizeof(ioinfo.peer));
					ioinfo.peerlen = 0;
				}
				pdata->didwritecb(pr,&ioinfo);
				if(pdata->didwritecb == &io_activity) gwpr_buf_free(pr,ioinfo.buf);
				gwprwrq_free(pr,fsrc,q);
			} else {
				gwerr("(38FG7) proactor synchronous write completed with no did_write callback set.");
			}
		}
		
		if(evt->flags & GWRL_SYNC_CLOSE) {
			if(pdata->closedcb) {
				//a synchronous write discovered socket or file close.
				bzero(&ioinfo,sizeof(ioinfo));
				ioinfo.src = fsrc;
				pdata->closedcb(pr,&ioinfo);
			} else {
				gwerr("(7GK9F) proactor detected a closed socket or file with no closed callback set.");
			}
		}
		
		else if(evt->flags & GWRL_SYNC_ERROR) {
			if(pdata->errorcb) {
				//a synchronous write had an error.
				gwpr_error_info * errinfo = (gwpr_error_info *)evt->userdata;
				pdata->errorcb(pr,errinfo);
				free(errinfo);
			} else {
				gwerr("(0LOP4) proactor encountered a synchronous write error with no error callback set.");
			}
		}
	}
}

void
gwpr_free(gwpr * pr) {
	int i = 0;
	gwrlsrc * src = NULL;
	gwrlsrc_file * fsrc = NULL;
	
	gwrl_dispatch(pr->rl);

	for(; i<GWRL_SRC_TYPES_COUNT; i++) {
		src = pr->rl->sources[i];
		while(src) {
			if(src->type == GWRL_SRC_TYPE_FILE) {
				gwrl_src_disable(pr->rl,src);
				fsrc = _gwrlsrcf(src);
				src->callback = NULL;
				if(fsrc->pdata) {
					gwprdata * pdata = fsrc->pdata;
					if(pdata->rdfilters) {
						free(pdata->rdfilters);
						pdata->rdfilters = NULL;
					}
					if(pdata->wrfilters) {
						free(pdata->wrfilters);
						pdata->wrfilters = NULL;
					}
					if(pdata->rdbuf) {
						gwpr_buf_free(pr,pdata->rdbuf);
						pdata->rdbuf = NULL;
					}
					if(pdata->wrq) {
						gwprwrq_free_list_no_cache(pr,pdata->wrq);
						pdata->wrq = NULL;
						pdata->wrqlast = NULL;
					}
					free(fsrc->pdata);
					fsrc->pdata = NULL;
				}
			}
			src = src->next;
		}
	}
	if(pr->wrqcache) {
		gwprwrq_free_list_no_cache(pr,pr->wrqcache);
	}
	if(pr->bufctl) {
		free(pr->bufctl);
	}
	pr->rl->pr = NULL;
	pr->rl = NULL;
	free(pr);
}

void
gwpr_write_buffer(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
gwpr_io_op_id op, struct sockaddr_storage * peer, socklen_t peerlen,
size_t * written, int * errnm) {
	ssize_t didwrite = 0;
	size_t towrite = buf->len;
	char * _buf = buf->buf;
	
	while(towrite > 0) {
		*errnm = 0;
		
		if(op == gwpr_write_op_id) {
			while((didwrite=write(fsrc->fd,_buf,towrite)) && didwrite < 0 && (errno == EINTR));
		} else if(op == gwpr_send_op_id) {
			while((didwrite=send(fsrc->fd,_buf,towrite,0)) && didwrite < 0 && (errno == EINTR));
		} else if(op == gwpr_sendto_op_id) {
			while((didwrite=sendto(fsrc->fd,_buf,towrite,0, _sockaddr(peer),peerlen)) && didwrite < 0 && (errno == EINTR));
		}
		
		if(didwrite > 0) {
			*written += didwrite;
			towrite -= didwrite;
			_buf += didwrite;
		} else if(didwrite < 0) {
			*errnm = errno;
			break;
		}
	}
}

bool
gwpr_synchronous_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
gwpr_io_op_id op, struct sockaddr_storage * peer, socklen_t peerlen) {
	bool didwr = false;
	
	#if defined(GWPR_TRY_SYNCHRONOUS_WRITE_UNIX)
	
	if(buf->len <= pr->options.gwpr_synchronous_write_max_bytes
		&& pr->options.gwpr_synchronous_write_max_bytes > 0) {
		//the buffer requested is smaller than the maximum allowed
		//bytes to allow synchronous writes with, so give it a shot.

		//setup vars
		gwrlsrc * src = _gwrlsrc(fsrc);
		gwprdata * pdata = fsrc->pdata;
		gwprbuf * usedbuf = buf;
		gwprbuf * rembuf = NULL;
		gwprwrq * q = NULL;
		gwrlevt * evt = NULL;

		if(pdata->wrfilters && pdata->wrfilters[0] != NULL) {
			//call write filters

			//copy buffer before calling write filters, this is in case
			//everything fails and we need to queue writing for later.
			gwprbuf * bfcp = gwpr_buf_get(pr,buf->bufsize);
			while(!bfcp) bfcp = gwpr_buf_get(pr,buf->bufsize);
			bfcp->len = buf->len;
			memcpy(bfcp->buf,buf->buf,buf->len);
			usedbuf = bfcp;

			//setup ioinfo for write filter
			gwpr_io_info ioinfo = {0};
			ioinfo.src = fsrc;
			ioinfo.buf = usedbuf;
			ioinfo.op = op;
			if(op == gwpr_sendto_op_id) {
				ioinfo.peerlen = peerlen;
				memcpy(&ioinfo.peer,peer,sizeof(ioinfo.peer));
			}
			
			//call filters
			int i = 0;
			for(; i<GWPR_FILTERS_MAX; i++) {
				if(!pdata->wrfilters[i]) break;
				pdata->wrfilters[i](pr,&ioinfo);
			}
		}
		
		//write the buffer
		int errnm = 0;
		size_t written = 0;
		gwpr_write_buffer(pr,fsrc,usedbuf,op,peer,peerlen,&written,&errnm);
		
		if(written == usedbuf->len) {
			//fullwrite success

			//get a wrq for the event userdata
			q = gwprwrq_get(pr,fsrc);

			//get an event to post
			evt = gwrl_evt_create(pr->rl,src,&gwpr_src_activity,q,fsrc->fd,GWRL_SYNC_WRITE);
			while(!evt) evt = gwrl_evt_create(pr->rl,src,&gwpr_src_activity,q,fsrc->fd,GWRL_SYNC_WRITE);

			//post the event back to the proactor to catch.
			q->buf = usedbuf;
			q->wrop = op;
			if(op == gwpr_sendto_op_id) {
				q->peerlen = peerlen;
				memcpy(&q->peer,peer,sizeof(q->peer));
			}
			gwrl_post_evt(pr->rl,evt);
			
			//set result
			didwr = true;
		} else if(written < usedbuf->len) {
			//partial write, copy the unwritten data to a new buffer.
			//post a write event back to the proactor and queue the
			//remaining data buffer for writing.
			
			//copy the remaining buffer data that hasn't been
			//written to requeue later.
			size_t remaining = usedbuf->len - written;
			rembuf = gwpr_buf_get(pr,remaining);
			while(!rembuf) rembuf = gwpr_buf_get(pr,remaining);
			memcpy(rembuf->buf,usedbuf->buf + written,remaining);

			//get a wrq for the user data
			gwprwrq * q = gwprwrq_get(pr,fsrc);
			q->buf = usedbuf;
			q->wrop = op;

			//update the used buffer to reflect what was actually written.
			//even though the remaining unwritten buffer data is still in
			//the buffer it won't matter. users should only use what's in
			//the len property.
			usedbuf->len = written;

			//update wrq for udp
			if(op == gwpr_sendto_op_id) {
				q->peerlen = peerlen;
				memcpy(&q->peer,peer,sizeof(q->peer));
			}

			//get event and post it
			gwrlevt * evt = gwrl_evt_create(pr->rl,_gwrlsrc(fsrc),&gwpr_src_activity,q,fsrc->fd,GWRL_SYNC_WRITE);
			while(!evt) evt = gwrl_evt_create(pr->rl,_gwrlsrc(fsrc),&gwpr_src_activity,q,fsrc->fd,GWRL_SYNC_WRITE);
			gwrl_post_evt(pr->rl,evt);
			
			if(errnm == 0 || errnm == EWOULDBLOCK || errnm == EAGAIN) {
				//either no errors, or the partial write errored out by
				//the kernel telling us there's no more room to write.
				//post the remaining buffer data to the write queue for later.

				gwpr_asynchronous_write(pr,fsrc,buf,op,peer,peerlen);
			}
			
			//set result
			didwr =  true;
		} else {
			didwr = false;
		}

		if(errnm == ECONNRESET || errnm == EPIPE) {
			//closed file descriptor or eof, post close event back to
			//proactor. for dispatching to the user. If there was a partial
			//write eariler we pass back the remaining data buffer in
			//case the user needs to handle it.

			//get a wrq for user data
			q = gwprwrq_get(pr,fsrc);
			if(written > 0) q->buf = rembuf;
			q->wrop = op;

			//get event and post it
			evt = gwrl_evt_create(pr->rl,_gwrlsrc(fsrc),&gwpr_src_activity,q,fsrc->fd,GWRL_SYNC_CLOSE);
			while(!evt) evt = gwrl_evt_create(pr->rl,_gwrlsrc(fsrc),&gwpr_src_activity,q,fsrc->fd,GWRL_SYNC_CLOSE);
			gwrl_post_evt(pr->rl,evt);

			//set result
			didwr = true;
		}

		else if(written == 0 && (errnm == EWOULDBLOCK || errnm == EAGAIN)) {
			//socket or fd is marked as non-blocking and no data could
			//be written without blocking, post it all in the queue for
			//writing later when the file descriptor is writable.
			gwpr_asynchronous_write(pr,fsrc,buf,op,peer,peerlen);
		}

		else if(errnm != 0) {
			//other error we don't want to handle. post this back
			//to the proactor for dispatching to the user.

			//create err info for the event.
			gwpr_error_info * errinfo = calloc(1,sizeof(gwpr_error_info));
			errinfo->errnm = errnm;
			errinfo->src = fsrc;
			errinfo->op = op;
			errinfo->buf = buf;
			if(written > 0) errinfo->buf = rembuf;
			if(op == gwpr_write_op_id) memcpy(errinfo->fnc,"write\0",6);
			else if(op == gwpr_send_op_id) memcpy(errinfo->fnc,"send\0",5);
			else if(op == gwpr_sendto_op_id) memcpy(errinfo->fnc,"sendto\0",7);

			//get an event and post it
			evt = gwrl_evt_create(pr->rl,src,&gwpr_src_activity,errinfo,fsrc->fd,GWRL_SYNC_ERROR);
			while(!evt) evt = gwrl_evt_create(pr->rl,src,&gwpr_src_activity,errinfo,fsrc->fd,GWRL_SYNC_ERROR);
			gwrl_post_evt(pr->rl,evt);
		}
	}
	
	#endif
	return didwr;
}

void
gwpr_asynchronous_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
gwpr_io_op_id op, struct sockaddr_storage * peer, socklen_t peerlen) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwprwrq * q = gwprwrq_get(pr,fsrc);
	gwrlsrc_flags_t flags = src->flags;
	#ifndef GWRL_HIDE_FROM_COVERAGE
	gwprdata * pdata = fsrc->pdata;
	if(!pdata->didwritecb) {
		pdata->didwritecb = &io_activity;
	}
	#endif
	q->buf = buf;
	q->wrop = op;
	if(op == gwpr_sendto_op_id) {
		q->peerlen = peerlen;
		memcpy(&q->peer,peer,sizeof(q->peer));
	}
	gwprwrq_add(pr,fsrc,q);
	flset(flags,GWRL_ENABLED|GWRL_WR);
	gwrl_src_file_update_flags(pr->rl,src,flags);
}

int
gwpr_accept(gwpr * pr, gwrlsrc_file * fsrc) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwprdata * pdata = fsrc->pdata;
	gwrlsrc_flags_t flags = src->flags;
	if(!pdata->acceptcb) {
		gwerr("(6NMC4) cannot call accept without an accept callback set.");
		return -1;
	}
	flset(flags,GWRL_ENABLED|GWRL_RD);
	gwrl_src_file_update_flags(pr->rl,src,flags);
	return 0;
}

int
gwpr_connect(gwpr * pr, gwrlsrc_file * fsrc,
struct sockaddr_storage * addr) {
	int res = 0;
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwprdata * prdata = (gwprdata *)fsrc->pdata;
	gwrlsrc_flags_t flags = src->flags;
	if(!prdata->connectcb) {
		gwerr("(8rFG3) cannot call connect without a connect callback set.");
		return -1;
	}
	gwsk_nonblock(fsrc->fd,1);
	res = gwsk_connect(fsrc->fd,addr);
	flset(flags,GWRL_ENABLED|GWRL_WR|GWRL_RD);
	gwrl_src_file_update_flags(pr->rl,src,flags);
	return res;
}

int
gwpr_asynchronous_read(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
gwpr_io_op_id op) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwprdata * pdata = (gwprdata *)fsrc->pdata;
	gwrlsrc_flags_t flags = src->flags;
	if(!pdata->didreadcb) {
		gwerr("(7GB3H) cannot call recvfrom without a did read callback.");
		return -1;
	}
	flset(flags,GWRL_ENABLED|GWRL_RD);
	pdata->rdbuf = buf;
	pdata->rdbufsize = buf->bufsize;
	pdata->rdop = op;
	gwrl_src_file_update_flags(pr->rl,src,flags);
	return 0;
}

void
gwpr_stop_read(gwpr * pr, gwrlsrc_file * fsrc) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwrlsrc_flags_t flags = src->flags;
	flclr(flags,GWRL_RD);
	gwrl_src_file_update_flags(pr->rl,src,flags);
}

int
gwpr_read(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	return gwpr_asynchronous_read(pr,fsrc,buf,gwpr_read_op_id);
}

int
gwpr_recv(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	return gwpr_asynchronous_read(pr,fsrc,buf,gwpr_recv_op_id);
}

int
gwpr_recvfrom(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	return gwpr_asynchronous_read(pr,fsrc,buf,gwpr_recvfrom_op_id);
}


int
gwpr_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	gwprdata * pdata = fsrc->pdata;
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!pdata->didwritecb) {
		pdata->didwritecb = &io_activity;
	}
	#endif
	if(pdata->wrq) {
		gwpr_asynchronous_write(pr,fsrc,buf,gwpr_write_op_id,NULL,0);
		return 0;
	}
	if(!(gwpr_synchronous_write(pr,fsrc,buf,gwpr_write_op_id,NULL,0))) {
		gwpr_asynchronous_write(pr,fsrc,buf,gwpr_write_op_id,NULL,0);
	}
	return 0;
}

int
gwpr_send(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	gwprdata * pdata = fsrc->pdata;
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!pdata->didwritecb) {
		pdata->didwritecb = &io_activity;
	}
	#endif
	if(pdata->wrq) {
		gwpr_asynchronous_write(pr,fsrc,buf,gwpr_send_op_id,NULL,0);
		return 0;
	}
	if(!(gwpr_synchronous_write(pr,fsrc,buf,gwpr_send_op_id,NULL,0))) {
		gwpr_asynchronous_write(pr,fsrc,buf,gwpr_send_op_id,NULL,0);
	}
	return 0;
}

int
gwpr_sendto(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
	struct sockaddr_storage * peer, socklen_t peerlen) {
	gwprdata * pdata = fsrc->pdata;
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!pdata->didwritecb) {
		pdata->didwritecb = &io_activity;
	}
	#endif
	if(pdata->wrq) {
		gwpr_asynchronous_write(pr,fsrc,buf,gwpr_sendto_op_id,peer,peerlen);
		return 0;
	}
	if(!(gwpr_synchronous_write(pr,fsrc,buf,gwpr_sendto_op_id,peer,peerlen))) {
		gwpr_asynchronous_write(pr,fsrc,buf,gwpr_sendto_op_id,peer,peerlen);
	}
	return 0;
}

gwprwrq *
gwprwrq_get(gwpr * pr, gwrlsrc_file * fsrc) {
	gwprwrq * wrq = NULL;
	if(pr->wrqcache) {
		wrq = pr->wrqcache;
		pr->nwrqcache--;
		if(wrq->next) {
			pr->wrqcache = wrq->next;
		} else {
			pr->wrqcache = NULL;
		}
	} else {
		wrq = gwrl_mem_calloc(1,sizeof(gwprwrq));
		while(!wrq) {
			gwerr("(9DXIL) calloc error");
			wrq = gwrl_mem_calloc(1,sizeof(gwprwrq));
		}
	}
	wrq->next = NULL;
	return wrq;
}

void
gwprwrq_free(gwpr * pr, gwrlsrc_file * fsrc, gwprwrq * wrq) {
	if(pr->nwrqcache == pr->options.gwpr_wrqueue_cache_max) {
		free(wrq);
	} else {
		wrq->next = NULL;
		wrq->buf = NULL;
		if(pr->wrqcache) wrq->next = pr->wrqcache;
		pr->wrqcache = wrq;
		pr->nwrqcache++;
	}
}

void
gwprwrq_free_list_no_cache(gwpr * pr, gwprwrq * wrq) {
	gwprwrq * _wrq = wrq;
	gwprwrq * _del = NULL;
	while(_wrq) {
		_del = _wrq;
		_wrq = _wrq->next;
		gwpr_buf_free(pr,_del->buf);
		free(_del);
	}
}

void
gwprwrq_putback(gwpr * pr, gwrlsrc_file * fsrc, gwprwrq * q) {
	gwprdata * data = fsrc->pdata;
	if(!data->wrq) {
		data->wrq = q;
	} else {
		gwprwrq * tmpq = data->wrq;
		gwprwrq * tmpqlast = NULL;
		data->wrq = q;
		while(q) {
			tmpqlast = q;
			q = q->next;
		}
		if(tmpqlast) tmpqlast->next = tmpq;
		q = tmpq;
		while(q) {
			tmpqlast = q;
			q = q->next;
		}
		if(tmpqlast) {
			data->wrqlast = tmpqlast;
			data->wrqlast->next = NULL;
		}
	}
}

void
gwprwrq_add(gwpr * pr, gwrlsrc_file * fsrc, gwprwrq * q) {
	gwprdata * data = fsrc->pdata;
	if(!data->wrq) {
		data->wrq = q;
		q->next = NULL;
	} else {
		if(!data->wrqlast) {
			data->wrqlast = q;
			data->wrq->next = data->wrqlast;
		} else {
			data->wrqlast->next = q;
			data->wrqlast = q;
		}
	}
}

#ifdef __cplusplus
}
#endif
