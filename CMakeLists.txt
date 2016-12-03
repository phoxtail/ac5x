cmake_minimum_required(VERSION 2.6)

project(ac50sdk C)
add_definitions("")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

set(SOURCES
	ini.c
	ac5x.c
	ac5x_timer.c
	ac5x_network.c
	ac5x_program_block.c
	ac5x_config.c
	ac5x_device.c
	ac5x_circular_queue.c
	ac5x_proprietary.c)

include_directories("${PROJECT_SOURCE_DIR}/include")
#find_package (Threads)
add_executable(ac5x ${SOURCES})
target_link_libraries (ac5x uv)
#target_link_libraries (ac5x ${CMAKE_THREAD_LIBS_INIT} libuv)
#add_library(ac50sdk STATIC ${SOURCES})
#target_include_directories(proslic)
#target_link_libraries(proslic)
#install(TARGETS ac50sdk DESTINATION lib)
#install(FILES AC5xDrv_Api.h DESTINATION include)
