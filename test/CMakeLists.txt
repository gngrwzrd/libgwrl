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

add_executable(reactor_free4 reactor_free4.c)
target_link_libraries(reactor_free4 gwrl)

add_executable(reactor_pers_timeouts reactor_pers_timeouts.c)
target_link_libraries(reactor_pers_timeouts gwrl)

add_executable(reactor_timer_interval reactor_timer_interval.c)
target_link_libraries(reactor_timer_interval gwrl)

add_executable(reactor_safely reactor_safely.c)
target_link_libraries(reactor_safely gwrl)

add_executable(reactor_no_sleep reactor_no_sleep.c)
target_link_libraries(reactor_no_sleep gwrl)

add_executable(reactor_wake_reinit reactor_wake_reinit.c)
target_link_libraries(reactor_wake_reinit gwrl)

if(NOT PLATFORM_WINDOWS)
	add_executable(reactor_wake_pipe reactor_wake_pipe.c)
	target_link_libraries(reactor_wake_pipe gwrl pthread)
	
	add_executable(proactor_wrq proactor_wrq.c)
	target_link_libraries(proactor_wrq gwrl)

	add_executable(proactor_gwprwrq_get proactor_gwprwrq_get.c)
	target_link_libraries(proactor_gwprwrq_get gwrl)

	add_executable(proactor_thread_specific_nix proactor_thread_specific_nix.c)
	target_link_libraries(proactor_thread_specific_nix gwrl pthread)

	add_executable(proactor_gwpr_src_activity_write_qitem_result proactor_gwpr_src_activity_write_qitem_result.c)
	target_link_libraries(proactor_gwpr_src_activity_write_qitem_result gwrl)
endif()

if(${WIN32})
	add_executable(reactor_wake_iocp reactor_wake_iocp.c)
	target_link_libraries(reactor_wake_iocp gwrl)
endif()

add_executable(proactor_buf proactor_buf.c)
target_link_libraries(proactor_buf gwrl)

add_executable(proactor_filters proactor_filters.c)
target_link_libraries(proactor_filters gwrl)

add_executable(proactor_free1 proactor_free1.c)
target_link_libraries(proactor_free1 gwrl)

add_executable(proactor_tcp proactor_tcp.c)
target_link_libraries(proactor_tcp gwrl)

add_executable(proactor_udp proactor_udp.c)
target_link_libraries(proactor_udp gwrl)

add_executable(proactor_src proactor_src.c)
target_link_libraries(proactor_src gwrl)

add_executable(proactor_cb proactor_cb.c)
target_link_libraries(proactor_cb gwrl)