
#ifdef TYPES_BOOL
	#if defined(PLATFORM_WINDOWS) && !defined(HAVE_TRUE_FALSE_LC)
	    #define HAVE_TRUE_FALSE_LC
	    #define true 1
	    #define false 0
	#endif
	#ifndef HAVE_BOOL
	    #define HAVE_bool
	    typedef char bool;
	#endif
#endif

#ifdef TYPES_STDINT
	#ifndef HAVE_INT8_T
		#error stdint types need to be defined
	#endif
#endif

#ifdef TYPES_FILEID_T
	#if defined(PLATFORM_WINDOWS) && !defined(_fileid_t)
		#define _fileid_t
	    typedef HANDLE fileid_t;
	#elif !defined(PLATFORM_WINDOWS) && !defined(_fileid_t)
		#define _fileid_t
	    typedef int fileid_t;
	#endif
#endif

#ifdef TYPES_SOCKID_T
	#if defined(PLATFORM_WINDOWS) && !defined(_sockid_t)
		#define _sockid_t
	    typedef SOCKET sockid_t;
	#elif !defined(PLATFORM_WINDOWS) && !defined(_sockid_t)
		#define _sockid_t
	    typedef int sockid_t;
	#endif
#endif

#ifdef TYPES_SSIZE_T
	#ifndef HAVE_SSIZE_T
		#if defined(PLATFORM_WINDOWS)
			#define HAVE_SSIZE_T
			typedef LONG_PTR ssize_t;
		#else
			#error ssize_t needs to be defined
		#endif
	#endif
#endif

#ifdef TYPES_STRUCT_TIMEVAL
	#if !defined(HAVE_STRUCT_TIMEVAL)
	 	#define HAVE_STRUCT_TIMEVAL
	 	struct timeval {
	    	long tv_sec;
	    	int tv_usec;
	 	};
	#endif
#endif

#ifdef TYPES_STRUCT_TIMESPEC
	#ifndef HAVE_STRUCT_TIMESPEC
		#ifdef HAVE_STDINT_H
	 		#define HAVE_STRUCT_TIMESPEC
	 		struct timespec {
	 			long tv_sec;
	 			long tv_nsec;
	 		};
		#endif
	#endif
#endif

#ifdef TYPES_SOCKLEN_T
	#if !defined(HAVE_SOCKLEN_T) && !defined(HAVE__SOCKLEN_T)
		#define HAVE_SOCKLEN_T
		typedef int socklen_t;
	#endif
#endif

#ifdef TYPES_SA_FAMILY_T
	#if !defined(HAVE_SA_FAMILY_T) && !defined(HAVE__SA_FAMILY_T)
		#if defined(PLATFORM_WINDOWS) && defined(HAVE_WS_ADDRESS_FAMILY)
			#define HAVE_SA_FAMILY_T
			typedef ADDRESS_FAMILY sa_family_t;
		#else
			#error sa_family_t needs to be defined
		#endif
	#endif
#endif

#ifdef TYPES_STRUCT_SOCKADDR_STORAGE
	#if !defined(HAVE_STRUCT_SOCKADDR_STORAGE)
		#error struct sockaddr_storage needs to be defined
	#endif
#endif
