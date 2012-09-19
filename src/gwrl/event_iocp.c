
#include "gwrl/proactor.h"
#include "gwrl/event_iocp.h"

void gwrlsrc_file_update_flags(gwrl * rl, gwrlsrc * src, uint8_t flags){}
void gwrl_bkd_del_src(gwrl * rl, gwrlsrc * src){}
void gwrl_bkd_enable_src(gwrl * rl, gwrlsrc * src){}
void gwrl_bkd_disable_src(gwrl * rl, gwrlsrc * src){}
void gwrl_wake_init(gwrl * rl){}
void gwrl_wake_free(gwrl * rl){}
void gwrl_bkd_set_options(gwrl * rl,gwrl_options * opts){}

gwrlbkd * gwrl_bkd_init(gwrl * rl) {
	int res = 0;
	gwrlbkd_iocp * iobkd = _gwrlbkdi(gwrl_mem_calloc(1,sizeof(gwrlbkd_iocp)));
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidConnectEx = WSAID_CONNECTEX;
	GUID GuidRecvMsg = WSAID_WSARECVMSG;
	GUID GuidSendMsg = WSAID_WSASENDMSG;
	DWORD bytes = 0;
	SOCKET s = 0;
	LPSTR msg = NULL;
	
	if(!iobkd) {
		gwerr("(5F8bB) calloc error");
		return NULL;
	}
	
	gwsk_startup();
	
	s = socket(AF_INET,SOCK_STREAM,0);
	PRINT_ERROR_FM(s,==,INVALID_SOCKET,WSAGetLastError(),NULL,iobkd);
	
	iobkd->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);
	PRINT_ERROR_FM(iobkd->iocp,==,NULL,GetLastError(),NULL,iobkd);

	res = WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,sizeof(GuidAcceptEx),
		&iobkd->AcceptEx,sizeof(iobkd->AcceptEx),
		&bytes,NULL,NULL);
	PRINT_ERROR_FM(res,==,SOCKET_ERROR,WSAGetLastError(),NULL,iobkd);

	res = WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidConnectEx,sizeof(GuidConnectEx),
		&iobkd->ConnectEx,sizeof(iobkd->ConnectEx),
		&bytes,NULL,NULL);
	PRINT_ERROR_FM(res,==,SOCKET_ERROR,WSAGetLastError(),NULL,iobkd);
	
	res = WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidRecvMsg,sizeof(GuidRecvMsg),
		&iobkd->WSARecvMsg,sizeof(iobkd->WSARecvMsg),
		&bytes,NULL,NULL);
	PRINT_ERROR_FM(res,==,SOCKET_ERROR,WSAGetLastError(),NULL,iobkd);

	res = WSAIoctl(s,SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidSendMsg,sizeof(GuidSendMsg),
		&iobkd->WSASendMsg,sizeof(iobkd->WSASendMsg),
		&bytes,NULL,NULL);
	PRINT_ERROR_FM(res,==,SOCKET_ERROR,WSAGetLastError(),NULL,iobkd);

	closesocket(s);
	return _gwrlbkd(iobkd);
}

void gwrl_bkd_free(gwrl * rl) {
	//TODO: close completion port?
	free(rl->backend);
	rl->backend = NULL;
}

void gwrl_wake(gwrl * rl) {
	if(flisset(rl->flags,GWRL_SLEEPING)) {
		int res = 0;
		gwrlbkd_iocp * iobkd = _gwrlbkdi(rl->backend);
		gwpr * pr = (gwpr *)rl->pr;
		gwpr_ovlp * ovlp = NULL;
		if(pr) ovlp = gwpr_ovlp_get(pr);
		else ovlp = (gwpr_ovlp *)calloc(1,sizeof(gwpr_ovlp));
		ovlp->op = gwpr_ovlp_op_wake;
		res = PostQueuedCompletionStatus(iobkd->iocp,0,0,(LPOVERLAPPED)ovlp);
		if(res == 0) {
			gwprintsyserr("(0P4Xk) PostQueuedCompletionStatus error",GetLastError());
		}
	}
}

void gwrl_bkd_src_add(gwrl * rl, gwrlsrc * src) {
	fileid_t res;
	gwrlbkd_iocp * iobkd = _gwrlbkdi(rl->backend);
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	res = CreateIoCompletionPort(fsrc->fd,iobkd->iocp,0,1);
	if(res == NULL) {
		gwprintsyserr("(P03CF) CreateIOCompletionPort Error",GetLastError());
		gwerr("(3FD4R) Error adding input source to IO Completion Ports");
	}
}

void gwrl_bkd_gather(gwrl * rl) {
	bool res = false;
	gwpr * pr = _gwpr(rl->pr);
	gwrlbkd_iocp * iobkd = _gwrlbkdi(rl->backend);
	gwrlsrc_file * fsrc = NULL;
	gwrlsrc * src = NULL;
	gwrlevt * evt = NULL;
	gwpr_ovlp * ovlp = NULL;
	DWORD numOfBytes = 0;
	ULONG_PTR ckey = 0;
	gwpr_ovlp * overlapped = NULL;
	int64_t milliseconds = 0;
	
	if(rl->backend->timeout.tv_sec == sec_min && rl->backend->timeout.tv_nsec == nsec_min) {
		milliseconds = INFINITE;
	} else {
		gwtm_timespec_to_ms(&rl->backend->timeout,&milliseconds);
	}
	
	if(milliseconds == INFINITE && flisset(rl->flags,GWRL_NOSLEEP)) {
		milliseconds = 0;
	}
	
	flset(rl->flags,GWRL_SLEEPING);
	res = GetQueuedCompletionStatus(iobkd->iocp,&numOfBytes,&ckey,(LPOVERLAPPED *)&overlapped,(DWORD)milliseconds);
	flclr(rl->flags,GWRL_SLEEPING);
	
	if(res == false) return;
	
	if(res == true) {
		ovlp = _gwprovlp(overlapped);
		if(ovlp->op == gwpr_ovlp_op_wake) {
			if(!pr) free(ovlp);
			else gwpr_ovlp_free(pr,ovlp);
			return;
		}
		fsrc = ovlp->src;
		src = _gwrlsrc(fsrc);
		ovlp->buf->len = numOfBytes;
		evt = gwrl_evt_create(rl,src,src->callback,ovlp,fsrc->fd,GWRL_IOCP);
		while(!evt) evt = gwrl_evt_create(rl,src,src->callback,ovlp,fsrc->fd,GWRL_IOCP);
		gwrl_post_evt(rl,evt);
	}
}
