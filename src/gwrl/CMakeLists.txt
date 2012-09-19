
include(CheckCSourceCompiles)
include(CheckStructHasMember)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckLibraryExists)
include(TestBigEndian)
include_directories("${gwrl_BINARY_DIR}")
include_directories("${gwrl_BINARY_DIR}/src")
include_directories("${gwrl_SOURCE_DIR}/src")

####################################################platform checks

test_big_endian(BIG_ENDIAN)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(PLATFORM_DARWIN 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	set(PLATFORM_FREEBSD 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(PLATFORM_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Sun")
	set(PLATFORM_SUN 1)
elseif(${WIN32})
	set(PLATFORM_WINDOWS 1)
endif()

####################################################headers

check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
check_include_file(assert.h HAVE_ASSERT_H)
check_include_file(errno.h HAVE_ERRNO_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(math.h HAVE_MATH_H)
check_include_file(netdb.h HAVE_NETDB_H)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)
check_include_file(net/route.h HAVE_NET_ROUTE_H)
check_include_file(pthread.h HAVE_PTHREAD_H)
check_include_file(stdbool.h HAVE_STDBOOL_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdio.h HAVE_STDIO_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(sys/sysctl.h HAVE_SYS_SYSCTL_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/uio.h HAVE_SYS_UIO_H)
check_include_file(time.h HAVE_TIME_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(Windows.h HAVE_WINDOWS_H)
check_include_file(WinSock2.h HAVE_WINSOCK2_H)

####################################################types

check_symbol_exists(bool stdbool.h HAVE_BOOL)
check_type_size(int8_t HAVE_INT8_T)
check_type_size(size_t HAVE_SIZE_T)
check_type_size(ssize_t HAVE_SSIZE_T)

if(PLATFORM_DARWIN OR PLATFORM_FREEBSD OR PLATFORM_SUN)
	check_symbol_exists(socklen_t sys/socket.h HAVE_SOCKLEN_T)
	check_symbol_exists(_SOCKLEN_T sys/socket.h HAVE__SOCKLEN_T)
	check_symbol_exists(sa_family_t sys/socket.h HAVE_SA_FAMILY_T)
	check_symbol_exists(_SA_FAMILY_T sys/socket.h HAVE__SA_FAMILY_T)
endif()

if(PLATFORM_LINUX)
check_c_source_compiles("
#include <sys/socket.h>
int main(int argc, char ** argv){
	socklen_t sl = 0;
	return 0;
}" HAVE_SOCKLEN_T)
check_c_source_compiles("
#include <sys/socket.h>
sa_family_t t = 0;
int main(int argc, char ** argv){
}" HAVE_SA_FAMILY_T)
endif()

if(PLATFORM_WINDOWS)
check_c_source_compiles("
#include <Winsock2.h>
#include <Windows.h>
int main(int argc, char ** argv){
	ADDRESS_FAMILY fam;
	return 0;
}" HAVE_WS_ADDRESS_FAMILY)
endif()

####################################################structures

if(NOT PLATFORM_WINDOWS)
	check_struct_has_member("struct sockaddr_storage" ss_family sys/socket.h HAVE_STRUCT_SOCKADDR_STORAGE)
	check_struct_has_member("struct timeval" tv_sec sys/select.h HAVE_STRUCT_TIMEVAL)
	check_struct_has_member("struct timespec" tv_sec sys/time.h HAVE_STRUCT_TIMESPEC)
endif()

if(PLATFORM_WINDOWS)
check_c_source_compiles("
#include <WinSock2.h>
#include <Windows.h>
struct timeval tv = {0};
int main(int argc, char ** argv){
	tv.tv_sec = 10;
	return 0;
}" HAVE_STRUCT_TIMEVAL)

#re-check struct sockaddr_storage
check_c_source_compiles("
#include <Winsock2.h>
#include <Windows.h>
int main(int argc, char ** argv){
	struct sockaddr_storage ss;
	return 0;
}" HAVE_STRUCT_SOCKADDR_STORAGE)
endif()

####################################################functions or macros

check_function_exists(fcntl HAVE_FCNTL)
check_function_exists(kqueue HAVE_KQUEUE)
check_function_exists(epoll_create HAVE_EPOLL)
check_function_exists(poll HAVE_POLL)
check_function_exists(select HAVE_SELECT)
check_function_exists(sysctl HAVE_SYSCTL)
check_symbol_exists(bzero strings.h HAVE_BZERO)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)

####################################################backends

if(HAVE_KQUEUE)
	set(KQUEUE_BACKEND 1)
elseif(HAVE_EPOLL)
	set(EPOLL_BACKEND 1)
elseif(HAVE_POLL)
	set(POLL_BACKEND 1)
elseif(HAVE_SELECT)
	set(SELECT_BACKEND 1)
elseif(PLATFORM_WINDOWS)
	set(IOCP_BACKEND 1)
endif()

####################################################set the right flags in the config header for the selected backend

if(KQUEUE_BACKEND)
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_kqueue.c)
	set(USING_KQUEUE_BACKEND 1)
	message(STATUS "Using event_kqueue.c for the event backend")
elseif(EPOLL_BACKEND)
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_epoll.c)
	set(USING_EPOLL_BACKEND 1)
	message(STATUS "Using event_epoll.c for the event backend")
elseif(POLL_BACKEND)
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_poll.c)
	set(USING_POLL_BACKEND 1)
	message(STATUS "Using event_poll.c for the event backend")
elseif(SELECT_BACKEND)
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_select.c)
	set(USING_SELECT_BACKEND 1)
	message(STATUS "Using event_select.c for the event backend")
elseif(IOCP_BACKEND)
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_iocp.c)
	set(USING_IOCP_BACKEND 1)
	message(STATUS "Using windows/event_iocp.c for the event backend")
endif()

####################################################check if there was a forced override for backend

if(GWRL_FORCE_KQUEUE)
	message(STATUS "Overriding previous event discovery and using event_kqueue.c")
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_kqueue.c)
	set(USING_KQUEUE_BACKEND 1)
	set(KQUEUE_BACKEND)
	unset(USING_EPOLL_BACKEND)
	unset(USING_POLL_BACKEND)
	unset(USING_SELECT_BACKEND)
elseif(GWRL_FORCE_EPOLL)
	message(STATUS "Overriding previous event discovery and using event_epoll.c")
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_epoll.c)
	set(USING_EPOLL_BACKEND 1)
	set(EPOLL_BACKEND)
	unset(USING_KQUEUE_BACKEND)
	unset(USING_POLL_BACKEND)
	unset(USING_SELECT_BACKEND)
elseif(GWRL_FORCE_POLL)
	message(STATUS "Overriding previous event discovery and using event_poll.c")
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_poll.c)
	set(POLL_BACKEND)
	set(USING_POLL_BACKEND 1)
	unset(USING_KQUEUE_BACKEND)
	unset(USING_EPOLL_BACKEND)
	unset(USING_SELECT_BACKEND)
elseif(GWRL_FORCE_SELECT)
	message(STATUS "Overriding previous event discovery and using event_select.c")
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_select.c)
	set(SELECT_BACKEND 1)
	set(USING_SELECT_BACKEND 1)
	unset(USING_KQUEUE_BACKEND)
	unset(USING_EPOLL_BACKEND)
	unset(USING_POLL_BACKEND)
elseif(GWRL_FORCE_IOCP)
	message(STATUS "Overriding previous event discovery and using windows/event_iocp.c")
	set(HAVE_BACKEND 1)
	set(BACKEND_SOURCE event_iocp.c)
	set(IOCP_BACKEND 1)
	set(USING_IOCP_BACKEND 1)
	unset(USING_KQUEUE_BACKEND)
	unset(USING_EPOLL_BACKEND)
	unset(USING_POLL_BACKEND)
	unset(USING_POLL_SELECT)
endif()

####################################################final build sources

set(SOURCES ${SOURCES} event.c time.c socket.c lock.c ${BACKEND_SOURCE})

if(NOT PLATFORM_WINDOWS)
	set(SOURCES ${SOURCES} event_wake_pipe.c)
endif()

if(GWRL_USE_PROACTOR)
	set(USING_PROACTOR 1)
	set(SOURCES ${SOURCES} proactor.c)
	if(PLATFORM_WINDOWS)
		set(SOURCES ${SOURCES} proactor_imp_iocp.c)
	else()
		set(SOURCES ${SOURCES} proactor_imp.c)
	endif()
endif()

if(NOT HAVE_GETTIMEOFDAY AND PLATFORM_WINDOWS)
	set(SOURCES ${SOURCES} gettimeofday.c)
endif()

####################################################final link libraries

if(PLATFORM_WINDOWS)
	set(FINAL_LINK_LIBRARIES Kernel32.lib WS2_32.lib WSock32.lib)
endif()

if(PLATFORM_LINUX)
	set(FINAL_LINK_LIBRARIES pthread)
endif()

add_library(gwrl STATIC ${SOURCES})
add_library(gwrl_dyld SHARED ${SOURCES})
target_link_libraries(gwrl ${FINAL_LINK_LIBRARIES})
target_link_libraries(gwrl_dyld ${FINAL_LINK_LIBRARIES})
set_target_properties(gwrl_dyld PROPERTIES OUTPUT_NAME "gwrl")
configure_file(shared_config.h.in ${gwrl_SOURCE_DIR}/src/gwrl/shared_config.h)
