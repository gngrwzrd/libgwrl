
#include "gwrl/proactor.h"
#include "gwrl/event_iocp.h"

#ifdef __cplusplus
extern "C" {
#endif

void gwpr_src_activity(gwrl * rl, gwrlevt * evt) {
	if(evt->src->type == GWRL_SRC_TYPE_FILE && evt->flags & GWRL_IOCP) {
		gwpr_io_op_id op;
		gwpr * pr = _gwpr(rl->pr);
		gwpr_ovlp * ovlp = _gwprovlp(evt->userdata);
		gwrlsrc_file * fsrc = _gwrlsrcf(evt->src);
		gwrlsrc * src = evt->src;
		gwprdata * pdata = _gwprdata(fsrc->pdata);
		gwpr_io_info ioinfo;
		gwpr_error_info errinfo;
		gwprbuf * buf = NULL;
		DWORD numBytes = 0;
		bool ovlpres = GetOverlappedResult(fsrc->fd,(LPOVERLAPPED)ovlp,&numBytes,TRUE);
		
		#define TRY_READFILTERS \
			if(pdata->rdfilters && pdata->rdfilters[0] != NULL) {\
				int i = 0;\
				for(; i<GWPR_FILTERS_MAX; i++) {\
					if(!pdata->rdfilters[i]) break;\
					pdata->rdfilters[i](pr,&ioinfo);\
				}\
			}\
		
		bzero(&ioinfo,sizeof(ioinfo));
		bzero(&errinfo,sizeof(errinfo));
		
		switch(ovlp->op) {
		case gwpr_ovlp_op_accept:
			if(pdata->acceptcb) {
				ioinfo.src = _gwrlsrcf(evt->src);
				ioinfo.peersrc = _gwrlsrcf(gwrl_src_file_create(ovlp->acceptsock,0,NULL,NULL));
				ioinfo.peerlen = sizeof(ioinfo.peer);
				getpeername((SOCKET)ovlp->acceptsock,_sockaddr(&ioinfo.peer),&ovlp->peerlen);
				pdata->acceptcb(pr,&ioinfo);
			}
			pdata->accept_count--;
			gwpr_accept(pr,ioinfo.src); //keeps the accept queue full
			break;
		
		case gwpr_ovlp_op_connect:
			if(pdata->connectcb) {
				ioinfo.src = _gwrlsrcf(evt->src);
				ioinfo.peerlen = ovlp->peerlen;
				memcpy(&ioinfo.peer,&ovlp->peer,ovlp->peerlen);
				pdata->connectcb(pr,&ioinfo);
			}
			break;
		
		case gwpr_ovlp_op_read:
		case gwpr_ovlp_op_recv:
			if(ovlp->op == gwpr_ovlp_op_read) op = gwpr_read_op_id;
			if(ovlp->op == gwpr_ovlp_op_recv) op = gwpr_recv_op_id;
			ioinfo.buf = ovlp->buf;
			ioinfo.src = fsrc;
			ioinfo.op = op;
			if(!ovlpres && GetLastError() == ERROR_HANDLE_EOF) {
				if(pdata->closedcb) pdata->closedcb(pr,&ioinfo);
			} else if((ovlp->op == gwpr_ovlp_op_read || ovlp->op == gwpr_ovlp_op_recv) && numBytes == 0) {
				if(pdata->closedcb) pdata->closedcb(pr,&ioinfo);
			} else {
				TRY_READFILTERS
				if(pdata->didreadcb) pdata->didreadcb(pr,&ioinfo);
				if(src->flags & GWRL_RD) {
					//make sure the kernel will keep reading if it can
					gwprbuf * buf = NULL;
					buf = gwpr_buf_get(pr,pdata->rdbufsize);
					while(!buf) buf = gwpr_buf_get(pr,pdata->rdbufsize);
					if(ovlp->op == gwpr_ovlp_op_read) gwpr_read(pr,ioinfo.src,buf);
					if(ovlp->op == gwpr_ovlp_op_recv) gwpr_recv(pr,ioinfo.src,buf);
				}
			}
			break;
			
		case gwpr_ovlp_op_recvfrom:
			ioinfo.op = gwpr_recvfrom_op_id;
			ioinfo.buf = ovlp->buf;
			ioinfo.src = fsrc;
			ioinfo.peerlen = ovlp->peerlen;
			memcpy(&ioinfo.peer,&ovlp->peer,ovlp->peerlen);
			TRY_READFILTERS
			if(pdata->didreadcb) pdata->didreadcb(pr,&ioinfo);
			if(src->flags & GWRL_RD) {
				//make sure the kernel will keep reading if it can
				gwprbuf * buf = gwpr_buf_get(pr,pdata->rdbufsize);
				while(!buf) buf = gwpr_buf_get(pr,pdata->rdbufsize);
				gwpr_recvfrom(pr,ioinfo.src,buf);
			}
			break;
		
		case gwpr_ovlp_op_write:
		case gwpr_ovlp_op_send:
			ioinfo.op = gwpr_write_op_id;
			ioinfo.buf = ovlp->buf;
			ioinfo.src = _gwrlsrcf(evt->src);
			if(pdata->didwritecb) pdata->didwritecb(pr,&ioinfo);
			break;
		
		case gwpr_ovlp_op_sendto:
			ioinfo.op = gwpr_sendto_op_id;
			ioinfo.buf = ovlp->buf;
			ioinfo.src = _gwrlsrcf(evt->src);
			ioinfo.peerlen = ovlp->peerlen;
			memcpy(&ioinfo.peer,&ovlp->peer,sizeof(ioinfo.peer));
			if(pdata->didwritecb) pdata->didwritecb(pr,&ioinfo);
			break;
		}
		
		ovlp->buf = NULL;
		gwpr_ovlp_free(pr,ovlp);
	}
}

int gwpr_accept(gwpr * pr, gwrlsrc_file * fsrc) {
	int i = 0;
	int res = 0;
	gwrlbkd_iocp * iobkd = _gwrlbkdi(pr->rl->backend);
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = NULL;
	SOCKET acceptsock;
	int accept_count = pr->options.gwpr_max_accept - pdata->accept_count;
	for(i; i < accept_count; i++) {
		acceptsock = socket(AF_INET,SOCK_STREAM,0);
		if(acceptsock == INVALID_SOCKET) {
			gwprintsyserr("(3DXlS) socket() error",WSAGetLastError());
			return WSAGetLastError();
		}
		pdata->accept_count++;
		ovlp = gwpr_ovlp_get(pr);
		while(!ovlp) ovlp = gwpr_ovlp_get(pr);
		ovlp->op = gwpr_ovlp_op_accept;
		ovlp->pr = pr;
		ovlp->src = fsrc;
		ovlp->buf = gwpr_buf_get(pr,256);
		while(!ovlp->buf) ovlp->buf = gwpr_buf_get(pr,128);
		ovlp->acceptsock = (fileid_t)acceptsock;
		res = iobkd->AcceptEx((SOCKET)fsrc->fd,acceptsock,ovlp->buf->buf,0,128,128,NULL,(LPOVERLAPPED)ovlp);
		if(res == true) continue;
		if(!res && WSAGetLastError() != WSA_IO_PENDING) {
			gwprintsyserr("(3Qw2F) AcceptEx error",WSAGetLastError());
			res = WSAGetLastError();
			break;
		}
	}
	return res;
}

int gwpr_connect(gwpr * pr, gwrlsrc_file * fsrc, struct sockaddr_storage * addr) {
	int res = 0;
	gwrlbkd_iocp * iobkd = _gwrlbkdi(pr->rl->backend);
	gwprdata * prdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->op = gwpr_ovlp_op_connect;
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->peerlen = gwsk_sockaddr_size(addr);
	memcpy(&ovlp->peer,addr,ovlp->peerlen);
	res = iobkd->ConnectEx((SOCKET)fsrc->fd,_sockaddr(addr),gwsk_sockaddr_size(addr),NULL,0,NULL,(LPOVERLAPPED)ovlp);
	if(res == true) return 0;
	if(!res && WSAGetLastError() != WSA_IO_PENDING) {
		gwprintsyserr("(pR4FA) ConnectEx error",WSAGetLastError());
		res = WSAGetLastError();
	}
	return res;
}

void gwpr_stop_read(gwpr * pr, gwrlsrc_file * fsrc) {
	gwrlsrc * src = _gwrlsrc(fsrc);
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	if(pdata->rdovlp) {
		int res = CancelIoEx(fsrc->fd,(LPOVERLAPPED)pdata->rdovlp);
		if(res == 0 && GetLastError() != ERROR_NOT_FOUND) {
			gwprintsyserr("(RF8K3) CancelExIO error:", GetLastError());
		}
	}
}

int
gwpr_asynchronous_read(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
gwpr_ovlp_op op) {
	int res = 0;
	DWORD flags = 0;
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->op = op;
	ovlp->buf = buf;
	pdata->rdovlp = ovlp;
	if(op == gwpr_ovlp_op_read) {
		res = ReadFile(fsrc->fd,buf->buf,(DWORD)buf->bufsize,NULL,(LPOVERLAPPED)ovlp);
		if(res == true) return 0;
		if(!res && GetLastError() != ERROR_IO_PENDING) {
			gwprintsyserr("(3FVs8) ReadFile error",GetLastError());
			res = GetLastError();
		}
	} else if(op == gwpr_ovlp_op_recv) {
		ovlp->wsabuf.buf = buf->buf;
		ovlp->wsabuf.len = (ULONG)buf->bufsize;
		res = WSARecv((SOCKET)fsrc->fd,&ovlp->wsabuf,1,NULL,&flags,(LPOVERLAPPED)ovlp,NULL);
		if(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			gwprintsyserr("(L4FdS) recv error",WSAGetLastError());
			res = WSAGetLastError();
		}
	} else if(op == gwpr_ovlp_op_recvmsg) {
		ovlp->wsabuf.buf = buf->buf;
		ovlp->wsabuf.len = (ULONG)buf->bufsize;
		ovlp->peerlen = sizeof(ovlp->peer);
		res = WSARecvFrom((SOCKET)fsrc->fd,&ovlp->wsabuf,1,NULL,&flags,_sockaddr(&ovlp->peer),&ovlp->peerlen,(WSAOVERLAPPED*)ovlp,NULL);
		if(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			gwprintsyserr("(3FDsK) recvfrom error",WSAGetLastError());
			res = WSAGetLastError();
		}
	}
	return res;
}

int gwpr_read(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	int res = 0;
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->op = gwpr_ovlp_op_read;
	ovlp->buf = buf;
	pdata->rdovlp = ovlp;
	res = ReadFile(fsrc->fd,buf->buf,(DWORD)buf->bufsize,NULL,(LPOVERLAPPED)ovlp);
	if(res == true) return 0;
	if(!res && GetLastError() != ERROR_IO_PENDING) {
		gwprintsyserr("(3FVs8) ReadFile error",GetLastError());
		res = GetLastError();
	}
	return res;
}

int gwpr_recv(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	int res = 0;
	DWORD flags = 0;
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->op = gwpr_ovlp_op_recv;
	ovlp->buf = buf;
	ovlp->wsabuf.buf = buf->buf;
	ovlp->wsabuf.len = (ULONG)buf->bufsize;
	pdata->rdovlp = ovlp;
	res = WSARecv((SOCKET)fsrc->fd,&ovlp->wsabuf,1,NULL,&flags,(LPOVERLAPPED)ovlp,NULL);
	if(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		gwprintsyserr("(L4FdS) recv error",WSAGetLastError());
		res = WSAGetLastError();
	}
	return res;
}

int gwpr_recvfrom(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	int res = 0;
	DWORD flags = 0;
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->op = gwpr_ovlp_op_recvfrom;
	ovlp->buf = buf;
	ovlp->wsabuf.buf = buf->buf;
	ovlp->wsabuf.len = (ULONG)buf->bufsize;
	ovlp->peerlen = sizeof(ovlp->peer);
	pdata->rdovlp = ovlp;
	res = WSARecvFrom((SOCKET)fsrc->fd,&ovlp->wsabuf,1,NULL,&flags,_sockaddr(&ovlp->peer),&ovlp->peerlen,(WSAOVERLAPPED*)ovlp,NULL);
	if(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		gwprintsyserr("(3FDsK) recvfrom error",WSAGetLastError());
		res = WSAGetLastError();
	}
	return res;
}

int gwpr_recvmsg(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	return 0;
}

int gwpr_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	int res = 0;
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->op = gwpr_ovlp_op_write;
	ovlp->buf = buf;
	
	if(pdata->wrfilters && pdata->wrfilters[0] != NULL) {
		int i = 0;
		gwpr_io_info ioinfo;
		bzero(&ioinfo,sizeof(ioinfo));
		ioinfo.src = fsrc;
		ioinfo.buf = buf;
		for(; i<GWPR_FILTERS_MAX; i++) {
			if(!pdata->wrfilters[i]) break;
			pdata->wrfilters[i](pr,&ioinfo);
		}
	}
	
	res = WriteFile(fsrc->fd,buf->buf,(DWORD)buf->len,NULL,(LPOVERLAPPED)ovlp);
	if(res == true) return 0;
	if(!res && GetLastError() != ERROR_IO_PENDING) {
		gwprintsyserr("(3DJ5F) WriteFile error",GetLastError());
		res = GetLastError();
	}
	return res;
}

int gwpr_send(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	int res = 0;
	DWORD flags = 0;
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->op = gwpr_ovlp_op_sendto;
	ovlp->buf = buf;
	ovlp->wsabuf.buf = buf->buf;
	ovlp->wsabuf.len = (ULONG)buf->len;
	
	if(pdata->wrfilters && pdata->wrfilters[0] != NULL) {
		int i = 0;
		gwpr_io_info ioinfo;
		bzero(&ioinfo,sizeof(ioinfo));
		ioinfo.src = fsrc;
		ioinfo.buf = buf;
		for(; i<GWPR_FILTERS_MAX; i++) {
			if(!pdata->wrfilters[i]) break;
			pdata->wrfilters[i](pr,&ioinfo);
		}
	}
	
	res = WSASend((SOCKET)fsrc->fd,&ovlp->wsabuf,1,NULL,flags,(LPOVERLAPPED)ovlp,NULL);
	if(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		gwprintsyserr("(F4eDS) send() error",GetLastError());
		res = WSAGetLastError();
	}
	return res;
}

int gwpr_sendto(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf, struct sockaddr_storage * peer, socklen_t peerlen) {
	int res = 0;
	DWORD flags = 0;
	gwprdata * pdata = _gwprdata(fsrc->pdata);
	gwpr_ovlp * ovlp = gwpr_ovlp_get(pr);
	while(!ovlp) ovlp = gwpr_ovlp_get(pr);
	ovlp->pr = pr;
	ovlp->src = fsrc;
	ovlp->op = gwpr_ovlp_op_sendto;
	ovlp->buf = buf;
	ovlp->wsabuf.buf = buf->buf;
	ovlp->wsabuf.len = (ULONG)buf->len;
	if(pdata->wrfilters && pdata->wrfilters[0] != NULL) {
		int i = 0;
		gwpr_io_info ioinfo;
		bzero(&ioinfo,sizeof(ioinfo));
		ioinfo.peerlen = peerlen;
		memcpy(&ioinfo.peer,peer,peerlen);
		ioinfo.src = fsrc;
		ioinfo.buf = buf;
		for(; i<GWPR_FILTERS_MAX; i++) {
			if(!pdata->wrfilters[i]) break;
			pdata->wrfilters[i](pr,&ioinfo);
		}
	}
	res = WSASendTo((SOCKET)fsrc->fd,&ovlp->wsabuf,1,NULL,flags,_sockaddr(peer),peerlen,(LPWSAOVERLAPPED)ovlp,NULL);
	if(res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		gwprintsyserr("(L94FD) sendto() error",GetLastError());
		res = WSAGetLastError();
	}
	return res;
}

int gwpr_sendmsg(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf) {
	return 0;
}

gwpr_ovlp * gwpr_ovlp_get(gwpr * pr) {
	gwpr_ovlp * ovlp = NULL;
	if(!pr->ovlpcache) {
		ovlp = _gwprovlp(gwrl_mem_calloc(1,sizeof(*ovlp)));
		if(!ovlp) {
			gwerr("(3FGkd) calloc error");
			return NULL;
		}
		return ovlp;
	} else {
		ovlp = pr->ovlpcache;
		if(ovlp->next) pr->ovlpcache = ovlp->next;
		else pr->ovlpcache = NULL;
		pr->novlpcache--;
	}
	return ovlp;
}

void gwpr_ovlp_free(gwpr * pr, gwpr_ovlp * ovlp) {
	if(pr->novlpcache >= GWPR_IOCP_OVLP_CACHE_MAX) {
		free(ovlp);
	} else {
		bzero(&ovlp->wsabuf,sizeof(WSABUF));
		pr->novlpcache++;
		if(!pr->ovlpcache) {
			pr->ovlpcache = ovlp;
			ovlp->next = NULL;
		} else {
			ovlp->next = pr->ovlpcache;
			pr->ovlpcache = ovlp;
		}
	}
}

#ifdef __cplusplus
}
#endif
