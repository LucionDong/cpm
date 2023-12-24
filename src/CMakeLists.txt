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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always -Wall -Wextra -g -Wno-missing-braces")
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


aux_source_directory(src/parser ESV_SRC_PARSE)

set(PERSIST_SOURCES
	src/persist/persist.c
	src/persist/json/persist_json_plugin.c
	)

set(ESVCPM_BASE_SOURCE
	src/base/neu_plugin_common.c
	src/base/tag.c
	src/base/group.c
	src/event/event_linux.c
	src/utils/json.c
	src/utils/log.c
	src/utils/http_handler.c
	src/utils/http_proxy.c
	src/utils/http.c
	src/utils/neu_jwt.c
	src/utils/asprintf.c
	${PERSIST_SOURCES}
	)

add_library(esvcpm-base SHARED)
target_sources(esvcpm-base PRIVATE 
	${ESVCPM_BASE_SOURCE}
	${ESV_SRC_PARSE}
	)
target_include_directories(esvcpm-base PRIVATE 
	include/${PROJECT_NAME}
	src
	)

find_library(DL_LIB NAMES dl REQUIRED)
find_library(MATH_LIB NAMES m REQUIRED)
find_library(PTHREAD_LIB NAMES pthread REQUIRED)
find_library(ZLOG_LIB NAMES zlog REQUIRED)
find_library(JANSSON_LIB NAMES jansson REQUIRED)
find_library(NNG_LIB NAMES nng REQUIRED)
find_library(JWT_LIB NAMES jwt REQUIRED)
find_library(CRYPTO_LIB NAMES crypto REQUIRED)
find_library(SSL_LIB NAMES ssl REQUIRED)
find_library(SQLITE3_LIB NAMES sqlite3 REQUIRED)

target_link_libraries(esvcpm-base PRIVATE 
	# ${PTHREAD_LIB}
	# ${SSL_LIB}
	# ${CRYPTO_LIB}
	# ${NNG_LIB}
	# ${ZLOG_LIB}
	# ${JANSSON_LIB}
	# ${JWT_LIB}
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

set(ESVCPM_SOURCE
    src/main.c
	src/argparse.c
	src/daemon.c
	src/core/manager.c
	src/core/manager_internal.c
	src/core/plugin_manager.c
	src/core/node_manager.c
	src/core/storage.c
	src/core/manager_adapter_msg.c
	# src/core/manager_thing_model.c
	src/core/outside_service_manager.c
	src/adapter/adapter.c
	src/adapter/storage.c
	src/adapter/driver/cache.c
	src/adapter/driver/driver.c
	src/adapter/driver/device.c
	)

# set(CMAKE_BUILD_RPATH ./)
# add_executable(${PROJECT_NAME} ${ESVCPM_SOURCE})
message(STATUS CMAKE_FIND_ROOT_PATH:${CMAKE_FIND_ROOT_PATH})
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE 
	${ESVCPM_SOURCE}
	${PLUGIN_RESTFUL_SOURCE}
	)
target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/plugins
	)


target_link_libraries(${PROJECT_NAME} PRIVATE 
	esvcpm-base
	${DL_LIB}
	${MATH_LIB}
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
# 	${PROJECT_NAME}/c_pair.c
# 	)
# target_sources(nngtest PRIVATE
# 	${NNGTEST_SRC}
# 	)
# target_link_libraries(nngtest 
# 	${PTHREAD_LIB}
# 	${NNG_LIB}
# 	)


# set(CMAKE_INSTALL_PREFIX "/home/winston/Works/mac_work/dev/3rdparty/c/build_armv5l/easemore/easeview/esvcpm")
message(STATUS ${CMAKE_INSTALL_PREFIX})
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR}/include/)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.h")

install(TARGETS esvcpm esvcpm-base
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
