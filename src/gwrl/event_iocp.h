
#include <MSWSock.h>

typedef struct gwrlbkd_iocp {
	gwrlbkd _;
	fileid_t iocp;
	bool (*AcceptEx)(SOCKET,SOCKET,PVOID,DWORD,DWORD,DWORD,LPDWORD,LPOVERLAPPED);
	bool (*ConnectEx)(SOCKET,struct sockaddr *,int,PVOID,DWORD,LPDWORD,LPOVERLAPPED);
	int (*WSARecvMsg)();
	int (*WSASendMsg)();
} gwrlbkd_iocp;

#define _gwrlbkdi(o) ((gwrlbkd_iocp *)o)
#define _gwprovlp(o) ((gwpr_ovlp *)o)

#define PRINT_ERROR_FM(res,compare,chk,err,ret,fre)\
	if(res compare chk) {\
		LPSTR msg;\
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,0,err,0,(LPSTR)&msg,0,0);\
		fprintf(stderr,"IOCP Backend Init Error(%i): %s\n",err,msg);\
		LocalFree(msg);\
		if(fre) free(fre);\
		return ret;\
	}\

