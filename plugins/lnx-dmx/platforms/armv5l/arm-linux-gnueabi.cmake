set(CMAKE_SYSTEM_NAME Linux)
set(COMPILER_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/armv5l/arm-linux-gnueabi)
set(CMAKE_SYSTEM_PROCESSOR armv5l)
set(LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/armv5l/lib/libc)
set(ROOT_PATH_DIR ${LIBRARY_DIR})
set(INSTALL_PATH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deploy)

set(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)
set(CMAKE_AR ${COMPILER_PREFIX}-ar)
set(CMAKE_LINKER ${COMPILER_PREFIX}-ld)
set(CMAKE_NM ${COMPILER_PREFIX}-nm)
set(CMAKE_OBJDUMP ${COMPILER_PREFIX}-objdump)
set(CMAKE_RANLIB ${COMPILER_PREFIX}-ranlib)
set(CMAKE_STAGING_PREFIX ${INSTALL_PATH_DIR})
set(CMAKE_PREFIX_PATH ${CMAKE_STAGING_PREFIX})

# set(CMAKE_FIND_ROOT_PATH  ${CMAKE_STAGING_PREFIX})
set(CMAKE_FIND_ROOT_PATH  ${ROOT_PATH_DIR})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
