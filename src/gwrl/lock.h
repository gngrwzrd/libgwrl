
#define INCLUDE_PTHREAD
#define INCLUDE_WINDOWS
#define INCLUDE_LIBKERN_OSATOMIC
#include "gwrl/shared_config.h"
#include "gwrl/shared_include.h"
#include "gwrl/shared_types.h"

//define the lockid_t type.
#if defined(PLATFORM_DARWIN)
	typedef OSSpinLock lockid_t;
#elif defined(PLATFORM_WINDOWS)
	typedef CRITICAL_SECTION lockid_t;
#elif defined(PLATFORM_LINUX)
	typedef pthread_spinlock_t lockid_t;
#endif

void lockid_init(lockid_t * lk);
void lockid_lock(lockid_t * lk);
void lockid_unlock(lockid_t * lk);
void lockid_free(lockid_t * lk);