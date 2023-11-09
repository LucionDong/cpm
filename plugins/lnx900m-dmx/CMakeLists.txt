cmake_minimum_required(VERSION 3.13)
project(plugin-lnx900m-dmx VERSION 1.0)

enable_language(C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_definitions(NEU_PLATFORM_LINUX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(NOT CMAKE_SYSTEM_NAME)
  set(CMAKE_SYSTEM_NAME "Linux")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always -Wall -Wextra -g ")
set(CMAKE_C_FLAGS_RELEASE "-O1")

if(NOT DISABLE_ASAN)
endif()

if (CMAKE_STAGING_PREFIX)
else()
endif()

set(LNX-DMX_SRC
	src/lnx900m-dmx.c
	)

add_library(${PROJECT_NAME} SHARED
	${LNX-DMX_SRC}
	)
target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../../include/core-plugin-manager
	)

# message(STATUS include: ${CMAKE_CURRENT_SOURCE_DIR}/../../include/core-plugin-manager)
message(STATUS PROJECT_NAME: ${PROJECT_NAME})

find_library(DL_LIB NAMES dl REQUIRED)
find_library(MATH_LIB NAMES m REQUIRED)
find_library(PTHREAD_LIB NAMES pthread REQUIRED)
find_library(ZLOG_LIB NAMES zlog REQUIRED)
find_library( JANSSON_LIB NAMES jansson REQUIRED)
find_library(NNG_LIB NAMES nng REQUIRED)
find_library(JWT_LIB NAMES jwt REQUIRED)
find_library(CRYPTO_LIB NAMES crypto REQUIRED)
find_library(SSL_LIB NAMES ssl REQUIRED)
find_library(SQLITE3_LIB NAMES sqlite3 REQUIRED)
find_library(CPMBASE_LIB NAMES cpm-base REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
	# ${DL_LIB}
	# ${MATH_LIB}
	# ${PTHREAD_LIB}
	# ${ZLOG_LIB}
	# ${JANSSON_LIB}
	# ${NNG_LIB}
	# ${JWT_LIB}
	# ${CRYPTO_LIB}
	# ${SSL_LIB}
	# ${SQLITE3_LIB}
	${CPMBASE_LIB}
	)

