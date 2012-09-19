
//http://suacommunity.com/dictionary/gettimeofday-entry.php

#include <windows.h>
#include <time.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
	#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
	#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
struct timezone {
	int tz_minuteswest; /* minutes W of Greenwich */
	int tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval * tv, struct timezone * tz) {
	//define a structure to receive the current Windows filetime
	FILETIME ft;

	//initialize the present time to 0 and the timezone to UTC
	unsigned __int64 tmpres = 0;
	static int tzflag = 0;
	
	if(NULL != tv) {
		GetSystemTimeAsFileTime(&ft);
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;
		tmpres /= 10;
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}
	
	if(NULL != tz) {
		long tzdiff;
		int tzdaylight;
		if(!tzflag) {
			_tzset();
			tzflag++;
		}
		//Adjust for the timezone west of Greenwich
		_get_timezone(&tzdiff);
		_get_daylight(&tzdaylight);
		//tz->tz_minuteswest = _timezone / 60;
		//tz->tz_dsttime = _daylight;
		tz->tz_minuteswest = tzdiff / 60;
		tz->tz_dsttime = tzdaylight;
  	}
	return 0;
}
