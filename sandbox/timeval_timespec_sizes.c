
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char ** argv) {
	
	struct timespec ts;
	struct timeval tv;
	
	printf("sizeof long: %lu\n",sizeof(long));
	printf("sizeof ulong: %lu\n",sizeof(unsigned long));
	printf("sizeof long long: %lu\n",sizeof(long long));
	printf("---\n");
	printf("sizeof tv: %lu\n", sizeof(tv));
	printf("sizeof tv.tv_sec : %lu\n", sizeof(tv.tv_sec));
	printf("sizeof tv.tv_usec : %lu\n", sizeof(tv.tv_usec));
	printf("---\n");
	printf("sizeof ts: %lu\n", sizeof(ts));
	printf("sizeof ts.tv_sec : %lu\n", sizeof(ts.tv_sec));
	printf("sizeof ts.tv_usec : %lu\n", sizeof(ts.tv_nsec));
	
	return 0;
}
