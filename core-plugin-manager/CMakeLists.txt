cmake_minimum_required(VERSION 3.13)

enable_language(C)
# set(CMAKE_C_STANDARD 99)
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
# set(CMAKE_C_FLAGS "-Wall -Wextra -g")
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O1")
set(CMAKE_C_FLAGS_RELEASE "-O1")

if(NOT DISABLE_ASAN)
  # set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  # set(CMAKE_C_FLAGS_DEBUG "-fsanitize=address -fno-omit-frame-pointer")
  # set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -fsanitize=address -fno-omit-frame-pointer")
  # set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -fno-omit-frame-pointer")
endif()

if (CMAKE_STAGING_PREFIX)
# 	message(STATUS set_staging_include_directories)
# 	include_directories(
# 		${CMAKE_STAGING_PREFIX}/usr/include 
# 		${CMAKE_STAGING_PREFIX}/usr/local/include 
# 		)
  # link_directories(
	  # ${CMAKE_STAGING_PREFIX}/usr/lib
	  # # /home/winston/Works/mac_work/dev/3rdparty/c/build_armv5l/zlog-1.2.16/lib
	  # )
else()
#   include_directories(/home/landyu/Work/work_raid/arm/Allwinner/A33/Linux_SDK2019/sourcecode/build_ubuntu/include)
#   link_directories(/home/landyu/Work/work_raid/arm/Allwinner/A33/Linux_SDK2019/sourcecode/build_ubuntu/lib)
  # include_directories(/usr/local/include)
  # link_directories(/usr/local/lib)
endif()

set(MANG_SOURCE
    ${MY_PROJECT_NAME}/main.c
	${MY_PROJECT_NAME}/argparse.c
	${MY_PROJECT_NAME}/daemon.c
	${MY_PROJECT_NAME}/core/manager.c
	${MY_PROJECT_NAME}/core/manager_internal.c
	${MY_PROJECT_NAME}/core/plugin_manager.c
	${MY_PROJECT_NAME}/core/node_manager.c
	${MY_PROJECT_NAME}/core/storage.c
	${MY_PROJECT_NAME}/event/event_linux.c
	${MY_PROJECT_NAME}/adapter/adapter.c
	${MY_PROJECT_NAME}/adapter/storage.c
	${MY_PROJECT_NAME}/adapter/driver/cache.c
	${MY_PROJECT_NAME}/adapter/driver/driver.c
	${MY_PROJECT_NAME}/base/neu_plugin_common.c
	${MY_PROJECT_NAME}/base/tag.c
	${MY_PROJECT_NAME}/base/group.c
	${MY_PROJECT_NAME}/parser/neu_json_error.c
	${MY_PROJECT_NAME}/parser/neu_json_fn.c
	${MY_PROJECT_NAME}/parser/neu_json_global_config.c
	${MY_PROJECT_NAME}/parser/neu_json_group_config.c
	${MY_PROJECT_NAME}/parser/neu_json_tag.c
	${MY_PROJECT_NAME}/parser/neu_json_node.c
	${MY_PROJECT_NAME}/parser/neu_json_plugin.c
	${MY_PROJECT_NAME}/parser/neu_json_rw.c
	${MY_PROJECT_NAME}/parser/neu_json_login.c
	${MY_PROJECT_NAME}/parser/neu_json_log.c
	${MY_PROJECT_NAME}/parser/neu_json_file.c
	${MY_PROJECT_NAME}/parser/neu_json_ndriver.c
	${MY_PROJECT_NAME}/utils/json.c
	${MY_PROJECT_NAME}/utils/log.c
	${MY_PROJECT_NAME}/utils/http_handler.c
	${MY_PROJECT_NAME}/utils/http_proxy.c
	${MY_PROJECT_NAME}/utils/http.c
	${MY_PROJECT_NAME}/utils/neu_jwt.c
	${MY_PROJECT_NAME}/utils/asprintf.c
	${MY_PROJECT_NAME}/persist/persist.c
	${MY_PROJECT_NAME}/persist/json/persist_json_plugin.c
	)

set(PLUGIN_RESTFUL_SOURCE
	plugins/restful/rest.c
	plugins/restful/handle.c
	plugins/restful/global_config_handle.c
	plugins/restful/plugin_handle.c
	plugins/restful/adapter_handle.c
	plugins/restful/group_config_handle.c
	plugins/restful/datatag_handle.c
	plugins/restful/rw_handle.c
	plugins/restful/normal_handle.c
	plugins/restful/log_handle.c
	plugins/restful/version_handle.c
	plugins/restful/file_handle.c
	plugins/restful/ndriver_handle.c
	plugins/restful/user.c
	)

# set(CMAKE_BUILD_RPATH ./)
# add_executable(${MY_PROJECT_NAME} ${MANG_SOURCE})
message(STATUS CMAKE_FIND_ROOT_PATH:${CMAKE_FIND_ROOT_PATH})
add_executable(${MY_PROJECT_NAME})
target_sources(${MY_PROJECT_NAME} PRIVATE 
	${MANG_SOURCE}
	${PLUGIN_RESTFUL_SOURCE}
	)
target_include_directories(${MY_PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include/${MY_PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/${MY_PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/plugins
	)

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

target_link_libraries(${MY_PROJECT_NAME} PRIVATE 
	${DL_LIB}
	${MATH_LIB}
	${PTHREAD_LIB}
	${ZLOG_LIB}
	${JANSSON_LIB}
	${NNG_LIB}
	${JWT_LIB}
	${CRYPTO_LIB}
	${SSL_LIB}
	${SQLITE3_LIB}
	)

# add_executable(nngtest)

# set(NNGTEST_SRC
# 	${MY_PROJECT_NAME}/c_pair.c
# 	)
# target_sources(nngtest PRIVATE
# 	${NNGTEST_SRC}
# 	)
# target_link_libraries(nngtest 
# 	${PTHREAD_LIB}
# 	${NNG_LIB}
# 	)

add_library(cpm-base SHARED)
target_sources(cpm-base PRIVATE 
	${MANG_SOURCE}
	${PLUGIN_RESTFUL_SOURCE}
	)
target_include_directories(cpm-base PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include/${MY_PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/${MY_PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/plugins
	)

target_link_libraries(cpm-base PRIVATE 
	${DL_LIB}
	${MATH_LIB}
	${PTHREAD_LIB}
	${ZLOG_LIB}
	${JANSSON_LIB}
	${NNG_LIB}
	${JWT_LIB}
	${CRYPTO_LIB}
	${SSL_LIB}
	${SQLITE3_LIB}
	)
