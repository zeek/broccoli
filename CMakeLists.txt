project(Broccoli)

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

########################################################################
## Project/Build Configuration

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION LIMIT_COUNT 1)

set(SOVersion 3) # this must be incremented when broccoli interface changes
set(SOVersionMinor 0)
set(SOVersionRelease 0)

if (ENABLE_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else ()
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

# don't set extra compile flags again if already declared in this scope
# (i.e. this project is included from Bro)
if (NOT EXTRA_COMPILE_FLAGS)
    set(EXTRA_COMPILE_FLAGS "-Wall -Wno-unused")

    # Compiler flags may already exist in CMake cache (e.g. when specifying
    # CFLAGS environment variable before running cmake for the the first time)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_COMPILE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILE_FLAGS}")
endif ()

# Broccoli's debug preprocessor definition differs from other Bro projects
if (ENABLE_DEBUG)
    set(EXTRA_COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} -DBRO_DEBUG")
endif ()

if (BRO_SYSCONF_FILE)
    # BRO_SYSCONF_FILE already defined from the --with-configfile option
    set(install_config_file false)
else ()
    # Figure out where the default broccoli.conf file will be installed
    # if the install prefix of /usr was given, change the install location
    # of the config file to be /etc instead of /usr/etc
    if (${CMAKE_INSTALL_PREFIX} STREQUAL /usr)
        set(BRO_SYSCONF_DIR /etc)
    else ()
        set(BRO_SYSCONF_DIR ${CMAKE_INSTALL_PREFIX}/etc)
    endif ()

    set(BRO_SYSCONF_FILE ${BRO_SYSCONF_DIR}/broccoli.conf)
    set(install_config_file true)
endif ()

site_name(hostname)
if (UNIX)
    execute_process(COMMAND date OUTPUT_VARIABLE builddate
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
else ()
    set(builddate "build date not available")
endif ()
set(BUILDINFO "${hostname}, ${builddate}, Debugging support: ${ENABLE_DEBUG}")

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

if (install_config_file)
    install(FILES broccoli.conf DESTINATION ${BRO_SYSCONF_DIR})
    set(INSTALLED_CONFIG_FILES "${INSTALLED_CONFIG_FILES} ${BRO_SYSCONF_FILE}"
        CACHE STRING "" FORCE)
endif ()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/broccoli-config DESTINATION bin)

########################################################################
## Recurse on sub-directories

add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(docs)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bindings/broccoli-python/CMakeLists.txt)
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
    "\nShared libs:       ${BUILD_SHARED_LIBS}"
    "\n"
    "\nConfig file:       ${BRO_SYSCONF_FILE}"
    "\nPacket support:    ${BRO_PCAP_SUPPORT}"
    "\n"
    "\nCC:                ${CMAKE_C_COMPILER}"
    "\nCFLAGS:            ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BuildType}}"
    "\nCXX:               ${CMAKE_CXX_COMPILER}"
    "\nCXXFLAGS:          ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BuildType}}"
    "\nCPP:               ${CMAKE_CXX_COMPILER}"
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
