include_directories("${gwrl_BINARY_DIR}")
include_directories("${gwrl_SOURCE_DIR}/src")

add_executable(time time.c)
target_link_libraries(time gwrl)

add_executable(reactor_src reactor_src.c)
target_link_libraries(reactor_src gwrl)

add_executable(reactor_gatherfunc reactor_gatherfunc.c)
target_link_libraries(reactor_gatherfunc gwrl)

add_executable(reactor_events reactor_events.c)
target_link_libraries(reactor_events gwrl)

#if(NOT ${WIN32})
	add_executable(reactor_wake_pipe reactor_wake_pipe.c)
	target_link_libraries(reactor_wake_pipe gwrl pthread)
	
	add_executable(proactor_wrq proactor_wrq.c)
	target_link_libraries(proactor_wrq gwrl)
#endif()

if(${WIN32})
	add_executable(event_wake_iocp event_wake_iocp.c)
	target_link_libraries(event_wake_iocp gwrl)
endif()

add_executable(proactor_buf proactor_buf.c)
target_link_libraries(proactor_buf gwrl)

add_executable(reactor_timeout_rearm reactor_timeout_rearm.c)
target_link_libraries(reactor_timeout_rearm gwrl)

add_executable(event_free1 event_free1.c)
target_link_libraries(event_free1 gwrl)

add_executable(event_free2 event_free2.c)
target_link_libraries(event_free2 gwrl)

add_executable(event_free3 event_free3.c)
target_link_libraries(event_free3 gwrl)

add_executable(reactor_del_pers_timeouts reactor_del_pers_timeouts.c)
target_link_libraries(reactor_del_pers_timeouts gwrl)

add_executable(reactor_del_pers_timeouts2 reactor_del_pers_timeouts2.c)
target_link_libraries(reactor_del_pers_timeouts2 gwrl)

add_executable(proactor_filters proactor_filters.c)
target_link_libraries(proactor_filters gwrl)
