include_directories("${gwrl_BINARY_DIR}")
include_directories("${gwrl_SOURCE_DIR}/src")

if(NOT PLATFORM_WINDOWS)
	add_executable(reactor_echo reactor_echo.c)
	target_link_libraries(reactor_echo gwrl)
endif()

add_executable(proactor_echo proactor_echo.c)
target_link_libraries(proactor_echo gwrl)
