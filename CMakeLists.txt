cmake_minimum_required(VERSION 3.10)
project(MemStat)

option(ENABLE_UNICODE "Compile with Unicode support" ON)

if(ENABLE_UNICODE)
	add_definitions(-DUNICODE -D_UNICODE)
endif()

add_executable(memstat WIN32 memstat.c loadstr.c memstat.rc)
