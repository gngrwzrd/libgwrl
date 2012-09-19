include_directories("${gwrl_BINARY_DIR}")
include_directories("${gwrl_SOURCE_DIR}/src")

add_executable(timers timers.c)
target_link_libraries(timers gwrl)

add_executable(event_gatherfunc_example event_gatherfunc.c)
target_link_libraries(event_gatherfunc_example gwrl)

add_executable(echo_proactor echo_proactor.c)
target_link_libraries(echo_proactor gwrl)
	
add_executable(reactor_custom_gathering reactor_custom_gathering.c)
target_link_libraries(reactor_custom_gathering gwrl)

add_executable(stdin stdin.c)
target_link_libraries(stdin gwrl)

if(NOT PLATFORM_WINDOWS)
	add_executable(echo_reactor echo_reactor.c)
	target_link_libraries(echo_reactor gwrl)
endif()
