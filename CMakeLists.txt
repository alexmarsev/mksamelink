cmake_minimum_required(VERSION 2.4)

project("mksamelink")

if(NOT WIN32) 
	message(FATAL_ERROR "win32 only")
endif()

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

if(MSVC)
	if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /OPT:REF")
	endif()
endif()

# TODO somehow broken
#include_directories(blake2)
add_executable(mksamelink mksamelink.cpp)
#find_library(BLAKE2 libblake2 PATHS "${CMAKE_BINARY_DIR}/blake2" NO_DEFAULT_PATH)
#target_link_libraries(mksamelink ${BLAKE2})

if(WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
	add_custom_command(TARGET mksamelink POST_BUILD COMMAND
		mt.exe -manifest "\"${CMAKE_SOURCE_DIR}/mksamelink.manifest\"" "-outputresource:\"${CMAKE_BINARY_DIR}/mksamelink.exe\";#1")
endif()
