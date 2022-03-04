cmake_minimum_required(VERSION 3.13)

project(testV8use)
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Generator: ${CMAKE_GENERATOR} / ${CMAKE_GENERATOR_PLATFORM}")
cmake_policy(SET CMP0078 NEW)
cmake_policy(SET CMP0086 NEW)
include(cmake/ConfigureBuildType.cmake)
include(cmake/DetectAndConfigurePlatform.cmake)
include(cmake/CPM.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/ ${CMAKE_SOURCE_DIR})
set(PROJECT_MODULES_DIR ${PROJECT_SOURCE_DIR}/modules)
set(BINARY_MODULES_DIR ${CMAKE_BINARY_DIR}/modules)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(FG_ALL_LIBRARIES "")
set(FG_ALL_INCLUDE_DIRS "")
# This is a kind of global setting, it will affect clang and gcc 
set(CMAKE_CXX_STANDARD 17 CACHE STRING "Default value for CXX_STANDARD property of targets")
# by default the main executable name will be the same as the project
set(EXECUTABLE_NAME ${PROJECT_NAME})

find_program(VCPKG_EXE NAMES vcpkg)
if(VCPKG_EXE MATCHES "VCPKG_EXE-NOTFOUND")
    message(FATAL_ERROR "Could not find VCPKG executable on the system - please properly configure the PATH system variable")
else()
    message(STATUS "VCPKG executable was found: ${VCPKG_EXE}")
endif()

file(MAKE_DIRECTORY ${BINARY_MODULES_DIR})

set(FG_USING_JSON_MSG "Use JSON for Modern C++ template library with this build")
set(FG_USING_LOG4CPLUS_MSG "Use log4cplus C++ logging API with this build")
set(FG_USING_V8PP_MSG "Use v8pp to bind C++ functions and classes into V8 JavaScript engine")
set(FG_USING_CPPBTREE_MSG "Use C++ B-tree custom headers for STL containers replacement")
set(FG_USING_ZEROMQ_MSG "Use ZeroMQ")
set(FG_USING_MINIZIP_NG_MSG "Use MiniZip NG")

option(USING_JSON ${FG_USING_JSON_MSG} ON)
option(USING_LOG4CPLUS ${FG_USING_LOG4CPLUS_MSG} OFF)
option(USING_V8PP ${FG_USING_V8PP_MSG} ON)
option(USING_CPPBTREE ${FG_USING_CPPBTREE_MSG} OFF)
option(USING_ZEROMQ ${FG_USING_ZEROMQ_MSG} ON)
option(USING_MINIZIP_NG ${FG_USING_MINIZIP_NG_MSG} ON)
option(USING_TESTING "Use testing framework and build tests" ON)

find_package(V8 CONFIG REQUIRED)
set(V8_INCLUDE_DIRS ${V8_INCLUDE_DIR}) # for possible compatibility with different packages

CPMAddPackage(
    NAME magic_enum
    GITHUB_REPOSITORY Neargye/magic_enum
    VERSION 0.7.3
)
if (magic_enum_ADDED)
    set(FG_ALL_INCLUDE_DIRS ${FG_ALL_INCLUDE_DIRS} ${magic_enum_SOURCE_DIR}/include)
endif()
if(OS MATCHES "windows")
    set(FG_ALL_LIBRARIES ${FG_ALL_LIBRARIES} winmm.lib)
endif() # OS
#
# 
if(USING_MINIZIP_NG)
    message(STATUS "Project will use minizip-ng/zlib-ng for compression support")
    CPMAddPackage(
        NAME minizip
        GITHUB_REPOSITORY zlib-ng/minizip-ng
        GIT_TAG 3.0.4
        OPTIONS
        "MZ_COMPAT ON"
        "MZ_BUILD_TESTS OFF"
        "MZ_CODE_COVERAGE OFF"
        "MZ_LZMA OFF"
        "MZ_ZSTD OFF"
    )
    if(minizip_ADDED)
        set(FG_ALL_INCLUDE_DIRS ${FG_ALL_INCLUDE_DIRS} ${minizip_SOURCE_DIR})
        set(FG_ALL_LIBRARIES ${FG_ALL_LIBRARIES} minizip)
    endif()
endif() #USING_MINIZIP_NG

#
# ZeroMQ & separate C++ bindings
if(USING_ZEROMQ)
    message(STATUS "Project will use ZeroMQ with C++ bindings")
    set(ZMQ_BUILD_TESTS OFF CACHE BOOL "Build the tests for ZeroMQ" FORCE)
    set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "Whether or not to build the tests" FORCE)
    set(ENABLE_DRAFTS OFF CACHE BOOL "Build and install draft classes and methods" FORCE)
    CPMAddPackage(
        NAME libzmq
        GITHUB_REPOSITORY zeromq/libzmq
        VERSION 4.3.4
        OPTIONS
        "BUILD_TESTS OFF"
        "ZMQ_BUILD_TESTS OFF"
        "WITH_LIBSODIUM OFF"
        "WITH_LIBSODIUM_STATIC OFF"
    )
    if (libzmq_ADDED)
    endif()
    #
    CPMAddPackage(
        NAME cppzmq
        GITHUB_REPOSITORY zeromq/cppzmq
        VERSION 4.8.1
        OPTIONS
        "CPPZMQ_BUILD_TESTS OFF"
        "ENABLE_DRAFTS OFF"
    )
    if(cppzmq_ADDED)
        set(FG_ALL_INCLUDE_DIRS ${FG_ALL_INCLUDE_DIRS} ${cppzmq_SOURCE_DIR})
        set(FG_ALL_LIBRARIES ${FG_ALL_LIBRARIES} cppzmq)
    endif()
endif() #USING_ZEROMQ

#
# JSON for Modern C++ (embedded build)
if(USING_JSON)
    message(STATUS "Project will use JSON for Modern C++ (with embedded build)")
    CPMAddPackage(
        NAME nlohmann_json
        VERSION 3.7.2
        # the git repo is incredibly large, so we download the archived include directory
        URL https://github.com/nlohmann/json/releases/download/v3.7.2/include.zip
        URL_HASH SHA256=67f69c9a93b7fa0612dc1b6273119d2c560317333581845f358aaa68bff8f087
    )
    if (nlohmann_json_ADDED)
        set(FG_ALL_INCLUDE_DIRS ${FG_ALL_INCLUDE_DIRS} ${nlohmann_json_SOURCE_DIR}/include)
        #set(FG_ALL_LIBRARIES ${FG_ALL_LIBRARIES} nlohmann_json)
        add_library(nlohmann_json INTERFACE IMPORTED)
    endif()
endif() #USING_JSON
#-----------------------------------------------------------------------------------------

#
# log4cplus is a simple to use C++17 logging API providing thread--safe, flexible, and 
# arbitrarily granular control over log management and configuration.
# It is modeled after the Java log4j API.
if(USING_LOG4CPLUS)
    CPMAddPackage(
        NAME log4cplus
        GITHUB_REPOSITORY log4cplus/log4cplus
        VERSION 2.0.7
        OPTIONS
        "LOG4CPLUS_BUILD_TESTING Off"
        "LOG4CPLUS_BUILD_LOGGINGSERVER Off"
        "BUILD_SHARED_LIBS Off"
        "WITH_UNIT_TESTS Off"
        "LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME Off"
    )
    if(log4cplus_ADDED)
        set(FG_ALL_INCLUDE_DIRS ${FG_ALL_INCLUDE_DIRS} "${log4cplus_SOURCE_DIR}/include")
        set(FG_ALL_LIBRARIES ${FG_ALL_LIBRARIES} log4cplus)
    endif() #log4cplus_ADDED
endif() #USING_LOG4CPLUS
#-----------------------------------------------------------------------------------------

#
# 
if(USING_CPPBTREE)    
    CPMAddPackage(
        NAME cppbtree
        GITHUB_REPOSITORY Kronuz/cpp-btree
        GIT_TAG master
        DOWNLOAD_ONLY YES
    )
    if(cppbtree_ADDED)
        set(FG_ALL_INCLUDE_DIRS ${FG_ALL_INCLUDE_DIRS} "${cppbtree_SOURCE_DIR}")
    endif() #cppbtree_ADDED
endif() #USING_CPPBTREE

#
# Header-only library to expose C++ classes and functions into V8 for use in JavaScript code.
if(USING_V8PP)
    CPMAddPackage(
        NAME v8pp
        GITHUB_REPOSITORY pmed/v8pp
        GIT_TAG master
        DOWNLOAD_ONLY YES
        OPTIONS
        "BUILD_TESTING OFF"
        "BUILD_DOCUMENTATION OFF"
        "BUILD_SHARED_LIBS OFF"
    )
    if(v8pp_ADDED)
        # Apply special patches (by string replace feature) in order to make the v8pp build
        # compatible with vcpkg and CMake subdirectory build.
        file(READ ${v8pp_SOURCE_DIR}/v8pp/CMakeLists.txt FILE_CONTENTS)
        string(REPLACE "V8::v8" "V8::V8" FILE_CONTENTS "${FILE_CONTENTS}")
        string(REPLACE " V8::libplatform" "" FILE_CONTENTS "${FILE_CONTENTS}")
        string(REPLACE "V8 REQUIRED" "V8 CONFIG REQUIRED" FILE_CONTENTS "${FILE_CONTENTS}")
        
        string(REPLACE "-fno-rtti" "" FILE_CONTENTS "${FILE_CONTENTS}")
        string(REPLACE "/GR-" "" FILE_CONTENTS "${FILE_CONTENTS}")
        file(WRITE ${v8pp_SOURCE_DIR}/v8pp/CMakeLists.txt "${FILE_CONTENTS}")

        # Update the original CMakeLists to forcefully change the CXX standard property
        #file(READ ${v8pp_SOURCE_DIR}/CMakeLists.txt FILE_CONTENTS)
        #string(REPLACE "CMAKE_CXX_STANDARD 14" "CMAKE_CXX_STANDARD 17" FILE_CONTENTS "${FILE_CONTENTS}")
        #file(WRITE ${v8pp_SOURCE_DIR}/CMakeLists.txt "${FILE_CONTENTS}")

        # Add the subdirectory as a normal build
        set(BUILD_TESTING OFF CACHE BOOL "")
        set(BUILD_DOCUMENTATION OFF CACHE BOOL "")
        set(V8PP_HEADER_ONLY 1 CACHE BOOL "")
        set(v8pp_BUILD_DIR "${CMAKE_BINARY_DIR}/_deps/v8pp-build")
        add_subdirectory("${v8pp_SOURCE_DIR}" "${v8pp_BUILD_DIR}")

        set(FG_ALL_INCLUDE_DIRS ${FG_ALL_INCLUDE_DIRS} "${v8pp_SOURCE_DIR}" "${v8pp_BUILD_DIR}")
        endif() #v8pp_ADDED
endif() #USING_V8PP
#-----------------------------------------------------------------------------------------

# Final configuration file
configure_file (
        "${PROJECT_SOURCE_DIR}/preConfiguration.h.in"
        "${PROJECT_BINARY_DIR}/fg/preConfiguration.h"
        )
include_directories("${PROJECT_BINARY_DIR}")

subdirs(src)

if(USING_TESTING)
    CPMAddPackage(
        NAME Catch2
        GITHUB_REPOSITORY catchorg/Catch2
        VERSION 2.13.8
        OPTIONS
        "CATCH_BUILD_EXAMPLES OFF"
        "CATCH_BUILD_STATIC_LIBRARY ON"
        "CATCH_INSTALL_HELPERS ON"
        "CATCH_INSTALL_DOCS ON"
    )
    if(NOT Catch2_ADDED)
        message(FATAL "Unable to add Catch2 for unit testing")
    endif()
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)
    enable_testing()
    add_subdirectory(tests)
endif()