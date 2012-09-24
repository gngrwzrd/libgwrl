include_directories("${gwrl_BINARY_DIR}")
include_directories("${gwrl_SOURCE_DIR}/src")

add_executable(time_fncs time_fncs.c)
target_link_libraries(time_fncs gwrl)

add_executable(reactor_src reactor_src.c)
target_link_libraries(reactor_src gwrl)

add_executable(reactor_gatherfunc reactor_gatherfunc.c)
target_link_libraries(reactor_gatherfunc gwrl)

add_executable(reactor_events reactor_events.c)
target_link_libraries(reactor_events gwrl)

add_executable(reactor_options reactor_options.c)
target_link_libraries(reactor_options gwrl)

add_executable(reactor_timers reactor_timers.c)
target_link_libraries(reactor_timers gwrl)

add_executable(reactor_custom_gathering reactor_custom_gathering.c)
target_link_libraries(reactor_custom_gathering gwrl)

add_executable(reactor_timeout_rearm reactor_timeout_rearm.c)
target_link_libraries(reactor_timeout_rearm gwrl)

add_executable(reactor_free1 reactor_free1.c)
target_link_libraries(reactor_free1 gwrl)

add_executable(reactor_free2 reactor_free2.c)
target_link_libraries(reactor_free2 gwrl)

add_executable(reactor_free3 reactor_free3.c)
target_link_libraries(reactor_free3 gwrl)

add_executable(reactor_del_pers_timeouts reactor_del_pers_timeouts.c)
target_link_libraries(reactor_del_pers_timeouts gwrl)

add_executable(reactor_del_pers_timeouts2 reactor_del_pers_timeouts2.c)
target_link_libraries(reactor_del_pers_timeouts2 gwrl)

add_executable(reactor_del_pers_timeouts3 reactor_del_pers_timeouts3.c)
target_link_libraries(reactor_del_pers_timeouts3 gwrl)

add_executable(reactor_timer_interval reactor_timer_interval.c)
target_link_libraries(reactor_timer_interval gwrl)

if(NOT PLATFORM_WINDOWS)
	add_executable(reactor_wake_pipe reactor_wake_pipe.c)
	target_link_libraries(reactor_wake_pipe gwrl pthread)
	
	add_executable(proactor_wrq proactor_wrq.c)
	target_link_libraries(proactor_wrq gwrl)

	add_executable(proactor_thread_specific_nix proactor_thread_specific_nix.c)
	target_link_libraries(proactor_thread_specific_nix gwrl pthread)
endif()

if(${WIN32})
	add_executable(event_wake_iocp event_wake_iocp.c)
	target_link_libraries(event_wake_iocp gwrl)
endif()

add_executable(proactor_buf proactor_buf.c)
target_link_libraries(proactor_buf gwrl)

add_executable(proactor_filters proactor_filters.c)
target_link_libraries(proactor_filters gwrl)

add_executable(proactor_free1 proactor_free1.c)
target_link_libraries(proactor_free1 gwrl)

add_executable(proactor_tcp proactor_tcp.c)
target_link_libraries(proactor_tcp gwrl)

add_executable(proactor_mem_errors proactor_mem_errors.c)
target_link_libraries(proactor_mem_errors gwrl)