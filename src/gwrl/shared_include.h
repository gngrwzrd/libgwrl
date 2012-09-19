
//This file is used as a way to reduce putting the same #ifdef in every
//header for standard includes. You must define an INCLUDE_* before
//including this file to indicate which file to include. The include
//is wrapped with appropriate HAVE_*_H checks so you don't need to
//put them in your headers.

#if defined(INCLUDE_ASSERT)
#	if defined(HAVE_ASSERT_H)
#		include <assert.h>
#	endif
#endif

#if defined(INCLUDE_ERRNO)
#	if defined(HAVE_ERRNO_H)
#		include <errno.h>
#	endif
#endif

#if defined(INCLUDE_MATH)
#	if defined(HAVE_MATH_H)
#		include <math.h>
#	endif
#endif

#if defined(INCLUDE_LIMITS)
#	if defined(HAVE_LIMITS_H)
#		include <limits.h>
#	endif
#endif

#if defined(INCLUDE_STDBOOL)
#	if defined(HAVE_STDBOOL_H)
#		include <stdbool.h>
#	endif
#endif

#if defined(INCLUDE_STDINT)
#	if defined(HAVE_STDINT_H)
#		include <stdint.h>
#	endif
#endif

#if defined(INCLUDE_STDIO)
#	if defined(HAVE_STDIO_H)
#		include <stdio.h>
#	endif
#endif

#if defined(INCLUDE_STDLIB)
#	if defined(HAVE_STDLIB_H)
#		include <stdlib.h>
#	endif
#endif

#if defined(INCLUDE_UNISTD)
#	if defined(HAVE_UNISTD_H)
#		include <unistd.h>
#	endif
#endif

#if defined(INCLUDE_STRING)
#	if defined(HAVE_STRING_H)
#		include <string.h>
#	endif
#endif

#if defined(INCLUDE_STRINGS)
#	if defined(HAVE_STRINGS_H)
#		include <strings.h>
#	endif
#endif

#if defined(INCLUDE_TIME)
#	if defined(HAVE_TIME_H)
#		include <time.h>
#	endif
#endif

#if defined(INCLUDE_SYS_TIME)
#	if defined(HAVE_SYS_TIME_H)
#		include <sys/time.h>
#	endif
#endif

#if defined(INCLUDE_SYS_TYPES)
#	if defined(HAVE_SYS_TYPES_H)
#		include <sys/types.h>
#	endif
#endif

#if defined(INCLUDE_SYS_UIO)
#	if defined(HAVE_SYS_UIO_H)
#		include <sys/uio.h>
#	endif
#endif

#if defined(INCLUDE_ARPA_INET)
#	if defined(HAVE_ARPA_INET_H)
#		include <arpa/inet.h>
#	endif
#endif

#if defined(INCLUDE_NETINET)
#	if defined(HAVE_NETINET_IN_H)
#		include <netinet/in.h>
#	endif
#endif

#if defined(INCLUDE_NET_ROUTE)
#	if defined(HAVE_NET_ROUTE_H)
#		include <net/route.h>
#	endif
#endif

#if defined(INCLUDE_NETDB)
#	if defined(HAVE_NETDB_H)
#		include <netdb.h>
#	endif
#endif

#if defined(INCLUDE_SYS_SOCKET)
#	if defined(HAVE_SYS_SOCKET_H)
#		include <sys/socket.h>
#	endif
#endif

#if defined(INCLUDE_PTHREAD)
#	if defined(HAVE_PTHREAD_H)
#		include <pthread.h>
#	endif
#endif

#if defined(WIN_LEAN_MEAN) && defined(PLATFORM_WINDOWS)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#endif

#if defined(INCLUDE_WINSOCK2)
#	if defined(HAVE_WINSOCK2_H) && defined(PLATFORM_WINDOWS)
#		include <WinSock2.h>
#	endif
#endif

#if defined(INCLUDE_WS2TCPIP_H)
#	if defined(HAVE_WINSOCK2_H) && defined(PLATFORM_WINDOWS)
#		include <WS2tcpip.h>
#	endif
#endif

#if defined(INCLUDE_WINDOWS)
#	if defined(HAVE_WINDOWS_H) && defined(PLATFORM_WINDOWS)
#		include <Windows.h>
#	endif
#endif

#if defined(INCLUDE_WINBASE)
#	if defined(HAVE_WINDOWS_H) && defined(PLATFORM_WINDOWS)
#		include <WinBase.h>
#	endif
#endif

#if defined(INCLUDE_BASETSD)
#	if defined(HAVE_WINDOWS_H) && defined(PLATFORM_WINDOWS)
#		include <BaseTsd.h>
#	endif
#endif

#if defined(INCLUDE_IPHLPAPI)
#	if defined(HAVE_WINSOCK2_H) && defined(PLATFORM_WINDOWS)
#		include <IPHlpApi.h>
#	endif
#endif

#if defined(INCLUDE_KQUEUE)
#	if defined(HAVE_KQUEUE)
#		include <sys/event.h>
#	endif
#endif

#if defined(INCLUDE_EPOLL)
#	if defined(HAVE_EPOLL)
#		include <sys/epoll.h>
#	endif
#endif

#if defined(INCLUDE_POLL)
#	if defined(HAVE_POLL)
#		include <poll.h>
#	endif
#endif

#if defined(INCLUDE_SELECT)
#	if defined(HAVE_SELECT)
#		include <sys/select.h>
#	endif
#endif

#if defined(INCLUDE_FCNTL)
#	if defined(HAVE_FCNTL_H)
#		include <fcntl.h>
#	endif
#endif

#if defined(INCLUDE_SYS_SYSCTL)
#	if defined(HAVE_SYS_SYSCTL_H)
#		include <sys/sysctl.h>
#	endif
#endif

#if defined(INCLUDE_LIBKERN_OSATOMIC)
#	if defined(PLATFORM_DARWIN)
#		include <libkern/OSAtomic.h>
#	endif
#endif
