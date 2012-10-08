
#include "gwrl/event.h"

void gwrl_wake(gwrl * rl) {
	if(flisset(rl->flags,GWRL_SLEEPING)) {
		ssize_t res = 0;
		while(1) {
			res = write(rl->fds[1],"wake",4);
			#ifndef GWRL_HIDE_FROM_COVERAGE
			if(res < 0 && errno == EINTR) {
				continue;
			}
			#endif
			break;
		}
	}
}

void gwrl_wake_init(gwrl * rl) {
	gwrlsrc * fsrc = NULL;
	
	#ifdef GWRL_HIDE_FROM_COVERAGE
		pipe((int *)rl->fds);
	#endif

	#ifndef GWRL_HIDE_FROM_COVERAGE
		int res = pipe((int *)rl->fds);
		while(res < 0) {
			pipe((int *)rl->fds);
			gwprintsyserr("(3FG9D) pipe error",errno);
		}
	#endif
	
	fcntl(rl->fds[0],F_SETFL,O_NONBLOCK);
	fcntl(rl->fds[1],F_SETFL,O_NONBLOCK);
	fsrc = gwrl_src_file_create((fileid_t)rl->fds[0],GWRL_RD,&gwrl_wake_activity,NULL);
	
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!fsrc) {
		gwerr("(3FDli) gwrl_wake_init couldn't create file input source");
		return;
	}
	#endif
	
	gwrl_src_add(rl,fsrc);
}

void gwrl_wake_free(gwrl * rl) {
	close((int)rl->fds[0]);
	close((int)rl->fds[1]);
}

void gwrl_wake_activity(gwrl * rl, gwrlevt * evt) {
	int res = 0;
	bool reinit = false;
	if(flisset(evt->flags,GWRL_RD)) {
		char buf[4];
		while(1) {
			res = read(evt->fd,buf,4);
			
			if(res > 0) {
				continue;
			}
			
			if(res <= 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
				reinit = true;
			}

			break;
		}
		if(reinit) {
			#ifdef GWRL_COVERAGE_INTERNAL_ASSERT_VARS
			if(asserts_var1 == gwrl_assert_wake_reinit) {
				asserts_var2 = true;
			}
			#endif

			close(rl->fds[0]);
			close(rl->fds[1]);
			gwrl_src_del(rl,evt->src,NULL,true);
			gwrl_wake_init(rl);
		}
	}
}
