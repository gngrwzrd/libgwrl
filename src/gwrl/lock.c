
#include "gwrl/lock.h"

#ifdef __cplusplus
extern "C" {
#endif

void
lockid_init(lockid_t * lk) {
	#if defined(PLATFORM_WINDOWS)
		InitializeCriticalSection((LPCRITICAL_SECTION)lk);
	#elif defined(PLATFORM_LINUX)
		pthread_spin_init(lk,PTHREAD_PROCESS_PRIVATE);
	#endif
}

void
lockid_lock(lockid_t * lk) {
	#if defined(PLATFORM_DARWIN)
		OSSpinLockLock(lk);
	#elif defined(PLATFORM_WINDOWS)
		EnterCriticalSection((LPCRITICAL_SECTION)lk);
	#elif defined(PLATFORM_LINUX)
		pthread_spin_lock(lk);
	#endif
}

void
lockid_unlock(lockid_t * lk) {
	#if defined(PLATFORM_DARWIN)
		OSSpinLockUnlock(lk);
	#elif defined(PLATFORM_WINDOWS)
		LeaveCriticalSection((LPCRITICAL_SECTION)lk);
	#elif defined(PLATFORM_LINUX)
		pthread_spin_unlock(lk);
	#endif
}

void
lockid_free(lockid_t * lk) {
	#if defined(PLATFORM_WINDOWS)
		DeleteCriticalSection((LPCRITICAL_SECTION)lk);
	#elif defined(PLATFORM_LINUX)
		pthread_spin_destroy(lk);
	#endif
}

#ifdef __cplusplus
}
#endif
