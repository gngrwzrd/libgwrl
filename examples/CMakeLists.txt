include_directories("${gwrl_BINARY_DIR}")
include_directories("${gwrl_SOURCE_DIR}/src")

add_executable(reactor_timers reactor_timers.c)
target_link_libraries(reactor_timers gwrl)

add_executable(reactor_custom_gathering reactor_custom_gathering.c)
target_link_libraries(reactor_custom_gathering gwrl)

if(NOT PLATFORM_WINDOWS)
	add_executable(reactor_echo reactor_echo.c)
	target_link_libraries(reactor_echo gwrl)
endif()

add_executable(proactor_echo proactor_echo.c)
target_link_libraries(proactor_echo gwrl)

if(NOT PLATFORM_WINDOWS)
	add_executable(proactor_thread_specific_nix proactor_thread_specific_nix.c)
	target_link_libraries(proactor_thread_specific_nix gwrl pthread)
endif()