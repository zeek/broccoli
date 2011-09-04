project(Broccoli C)

if (NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "Could not find prerequisite C compiler")
endif ()

########################################################################
## CMake Configuration

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# Prohibit in-source builds.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
                        "./configure to choose a build directory and "
                        "initialize the build configuration.")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    # uninstall target
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                    @ONLY)

    add_custom_target(uninstall COMMAND
        ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif ()

# Keep RPATH upon installing so that user doesn't have to ensure the linker
# can find internal/private libraries or libraries external to the build
# directory that were explicitly linked against
if (NOT BINARY_PACKAGING_MODE)
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif ()

########################################################################
## Project/Build Configuration

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION LIMIT_COUNT 1)

set(SOVersion 3) # this must be incremented when broccoli interface changes
set(SOVersionMinor 0)
set(SOVersionRelease 0)

if (ENABLE_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    # Broccoli's debug preprocessor definition differs from other Bro projects
    add_definitions(-DBRO_DEBUG)
else ()
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

# don't set extra compile flags again if already declared in this scope
# (i.e. this project is included from Bro)
if (NOT EXTRA_COMPILE_FLAGS)
    set(EXTRA_COMPILE_FLAGS "-Wall -Wno-unused")

    if (ENABLE_DEBUG)
        # manual add of -g works around its omission in FreeBSD's CMake port
        set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -g -DDEBUG")
    endif ()

    # Compiler flags may already exist in CMake cache (e.g. when specifying
    # CFLAGS environment variable before running cmake for the the first time)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}")
endif ()

# Figure out where the default broccoli.conf file will be installed
# if the install prefix of /usr was given, change the install location
# of the config file to be /etc instead of /usr/etc
if (${CMAKE_INSTALL_PREFIX} STREQUAL /usr)
    set(BRO_SYSCONF_DIR /etc)
else ()
    set(BRO_SYSCONF_DIR ${CMAKE_INSTALL_PREFIX}/etc)
endif ()

if (NOT BRO_SYSCONF_FILE)
    # BRO_SYSCONF_FILE not already defined from the --with-configfile option
    set(BRO_SYSCONF_FILE ${BRO_SYSCONF_DIR}/broccoli.conf)
endif ()

site_name(hostname)
if (UNIX)
    execute_process(COMMAND date OUTPUT_VARIABLE builddate
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
else ()
    set(builddate "build date not available")
endif ()
set(BUILDINFO "${hostname}, ${builddate}, Debugging support: ${ENABLE_DEBUG}")

# if no static/shared library preference is given, default to building both
if (NOT DEFINED ENABLE_SHARED)
    set(ENABLE_SHARED true)
endif ()

if (NOT DEFINED ENABLE_STATIC)
    set(ENABLE_STATIC true)
endif ()

########################################################################
## Dependency Configuration

include(MacDependencyPaths)
include(FindRequiredPackage)

FindRequiredPackage(OpenSSL)
FindRequiredPackage(FLEX)
FindRequiredPackage(BISON)
find_package(PCAP)

set(BRO_PCAP_SUPPORT true)
if (NOT PCAP_INCLUDE_DIR)
    set(BRO_PCAP_SUPPORT false)
endif ()

if (MISSING_PREREQS)
    foreach (prereq ${MISSING_PREREQ_DESCS})
        message(SEND_ERROR ${prereq})
    endforeach ()
    message(FATAL_ERROR "Configuration aborted due to missing prerequisites")
endif ()

########################################################################
## System Introspection

if (${CMAKE_SYSTEM_NAME} MATCHES "BSD")
    set(BSD_HOST true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX_HOST true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Solaris")
    set(BRO_LIBADD socket nsl)
elseif (MINGW)
    set(BRO_LIBADD wsock32)
endif ()

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

include(CheckFunctionExists)
check_function_exists(geteuid HAVE_GETEUID)
check_function_exists(getpwuid HAVE_GETPWUID)

include(CheckTypeSize)

set(CMAKE_EXTRA_INCLUDE_FILES sys/types.h)
check_type_size(uint UINT)
if (NOT UINT)
    set (TYPEDEF_UINT "typedef unsigned int uint")
endif ()
set(CMAKE_EXTRA_INCLUDE_FILES)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/broccoli-config.in
               ${CMAKE_CURRENT_BINARY_DIR}/broccoli-config
               @ONLY)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DHAVE_CONFIG_H)

########################################################################
## Install Files

include(InstallPackageConfigFile)

InstallPackageConfigFile(
    ${CMAKE_CURRENT_SOURCE_DIR}/broccoli.conf
    ${BRO_SYSCONF_DIR}
    broccoli.conf)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/broccoli-config DESTINATION bin)

########################################################################
## Recurse on sub-directories

add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(docs)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bindings/broccoli-python/CMakeLists.txt
    AND NOT DISABLE_PYTHON_BINDINGS)
    add_subdirectory(bindings/broccoli-python)
else ()
    message(STATUS "Not building broccoli bindings")
endif ()

########################################################################
## Packaging Setup

# If this CMake project is a sub-project of another, we will not
# configure the generic packaging because CPack will fail in the case
# that the parent project has already configured packaging
if ("${PROJECT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    include(ConfigurePackaging)
    ConfigurePackaging(${VERSION})
endif ()

########################################################################
## Build Summary

if (CMAKE_BUILD_TYPE)
    string(TOUPPER ${CMAKE_BUILD_TYPE} BuildType)
endif ()

message(
    "\n=================|  Broccoli Build Summary  |==================="
    "\n"
    "\nInstall prefix:    ${CMAKE_INSTALL_PREFIX}"
    "\nDebug mode:        ${ENABLE_DEBUG}"
    "\nShared libs:       ${ENABLE_SHARED}"
    "\nStatic libs:       ${ENABLE_STATIC}"
    "\n"
    "\nConfig file:       ${BRO_SYSCONF_FILE}"
    "\nPacket support:    ${BRO_PCAP_SUPPORT}"
    "\n"
    "\nCC:                ${CMAKE_C_COMPILER}"
    "\nCFLAGS:            ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BuildType}}"
    "\nCPP:               ${CMAKE_C_COMPILER}"
    "\n"
    "\n================================================================\n"
)

########################################################################
## Show warning when installing user is different from the one that configured

install(CODE "
    if (NOT $ENV{USER} STREQUAL \$ENV{USER})
        message(STATUS \"ATTENTION: Install is being performed by user \"
                \"'\$ENV{USER}', but the build directory was configured by \"
                \"user '$ENV{USER}'. This may result in a permissions error \"
                \"when writing the install manifest, but you can ignore it \"
                \"and consider the installation as successful if you don't \"
                \"care about the install manifest.\")
    endif ()
")
