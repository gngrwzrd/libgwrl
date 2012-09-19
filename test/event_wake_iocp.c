
#include "gwrl/event.h"

gwrl * rl;
bool did_timeout = false;
HANDLE thread1;

void timeout(gwrl * rl, gwrlevt * evt) {
	did_timeout = true;
	gwrl_stop(rl);
}

DWORD threadmain(LPVOID args) {
	Sleep(1000);
	gwrl_post_function_safely(rl,&timeout,NULL);
	return 0;
}

void create_thread(gwrl * rl, gwrlevt * evt) {
	thread1 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&threadmain,NULL,0,NULL);
}

int main(int argc, char ** argv) {
	rl = gwrl_create();
	gwrl_post_function(rl,&create_thread,NULL);
	gwrl_run(rl);
	assert(did_timeout);
	return 0;
}
