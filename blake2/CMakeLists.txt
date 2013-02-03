cmake_minimum_required(VERSION 2.4)

project("blake2")

set(CMAKE_C_FLAGS "-std=c99 -Wall -pedantic -msse2")
set(blake2_sources blake2s.c blake2b.c blake2sp.c blake2bp.c)
add_library(blake2 STATIC ${blake2_sources})
if(WIN32)
	add_custom_command(TARGET blake2 POST_BUILD COMMAND
		cp ${CMAKE_STATIC_LIBRARY_PREFIX}blake2${CMAKE_STATIC_LIBRARY_SUFFIX}
		${CMAKE_STATIC_LIBRARY_PREFIX}blake2.lib)
endif()
