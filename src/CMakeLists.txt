
include_directories(${FG_ALL_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

message(STATUS "Include: '${FG_ALL_INCLUDE_DIRS}'")
message(STATUS "Libraries: '${FG_ALL_LIBRARIES}'")
message(STATUS "Parallel level: '${CMAKE_BUILD_PARALLEL_LEVEL}'")

set(CMAKE_VERBOSE_MAKEFILE ON)
if("${CMAKE_BUILD_PARALLEL_LEVEL}" STREQUAL "")
    set(CMAKE_BUILD_PARALLEL_LEVEL "8")
endif()
#-----------------------------------------------------------------------------------------
#
# Util sources
set(FG_Util_Headers
    util/AbstractFactory.hpp
    util/Bindings.hpp
    util/BindingsJson.hpp
    util/CallbackHelper.hpp
    util/Callbacks.hpp
    util/Dirent.hpp
    util/EnumFlags.hpp
    util/EnumName.hpp
    util/File.hpp
    util/FileBase.hpp
    util/FpsControl.hpp
    util/Handle.hpp
    util/HandleManager.hpp
    util/JsonFile.hpp
    util/Logger.hpp
    util/NamedHandle.hpp
    util/Profiling.hpp
    util/RegularFile.hpp
    util/SimpleThread.hpp
    util/Tag.hpp
    util/Timesys.hpp
    util/UniversalId.hpp
    util/UnpackCaller.hpp
    util/Util.hpp
    util/Vector.hpp
    util/ZipFile.hpp
)
set(FG_Util_Sources
    util/Dirent.cpp
    util/File.cpp
    util/Logger.cpp
    util/Profiling.cpp
    util/RegularFile.cpp
    util/Timesys.cpp
    util/SimpleThread.cpp
    util/Util.cpp
    util/ZipFile.cpp
)
#
# Resource sources
set(FG_Resource_Headers
    resource/DataManager.hpp
    resource/GlobalObjectRegistry.hpp
    resource/ManagedDataFile.hpp
    resource/ManagedObject.hpp
    resource/Resource.hpp
    resource/ResourceConfig.hpp
    resource/ResourceConfigJson.hpp
    resource/ResourceManager.hpp
)
set(FG_Resource_Sources
    resource/ResourceManager.cpp
)
#
# Event source
set(FG_Event_Headers
    event/EventDefinitions.hpp
    event/EventHelper.hpp
    event/EventManager.hpp
    event/KeyVirtualCodes.hpp
    event/ThrownEvent.hpp
    event/TimerEntryInfo.hpp
)
set(FG_Event_Sources
    event/EventManager.cpp
)
#
# Script source
set(FG_Script_Headers
    script/modules/Console.hpp
    script/InternalModule.hpp    
    script/Module.hpp
    script/ScriptManager.hpp
    script/V8.hpp
)
set(FG_Script_Sources
    script/modules/Console.cpp
    script/ScriptManager.cpp
    script/V8.cpp
)
#
# Root sources / base
set(FG_Root_Headers
    Application.hpp
    BuildConfig.hpp
    ConsoleApplication.hpp
    EngineMain.hpp
    Manager.hpp
    Quality.hpp
    Queue.hpp
    Singleton.hpp
    Unistd.hpp
    WindowsStandard.hpp
)
set(FG_Root_Sources
    EngineMain.cpp
)
#
# Application sources & headers
set(FG_Application_Headers)
set(FG_Main_Sources
    main.cpp
    test.cpp
    serverx.cpp
)
set(FG_Application_Sources
    ${FG_Main_Sources}
)
#
# Static library files listing 
set(FG_StaticLibrary_Headers
    ${FG_Util_Headers}
    ${FG_Resource_Headers}
    ${FG_Event_Headers}
    ${FG_Script_Headers}
    ${FG_Root_Headers}
)
set(FG_StaticLibrary_Sources
    ${FG_Util_Sources}
    ${FG_Resource_Sources}
    ${FG_Event_Sources}
    ${FG_Script_Sources}
    ${FG_Root_Sources}
)
source_group(main FILES ${FG_Root_Sources} ${FG_Main_Sources})
source_group(main\\inc FILES ${FG_Root_Headers})
source_group(util FILES ${FG_Util_Sources})
source_group(util\\inc FILES ${FG_Util_Headers})
source_group(resource FILES ${FG_Resource_Sources})
source_group(resource\\inc FILES ${FG_Resource_Headers})
source_group(event FILES ${FG_Event_Sources})
source_group(event\\inc FILES ${FG_Event_Headers})
source_group(script FILES ${FG_Script_Sources})
source_group(script\\inc FILES ${FG_Script_Headers})
#-----------------------------------------------------------------------------------------

add_definitions(-DUSE_PRE_CONFIG)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DRELEASE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
    add_definitions(-DDEBUG)
endif()
add_definitions(-D_HAS_AUTO_PTR_ETC=1)
add_definitions(-DV8_31BIT_SMIS_ON_64BIT_ARCH=1)
add_definitions(-DV8_COMPRESS_POINTERS)

## Static library definition
add_library(${STATIC_LIB_NAME} STATIC 
    ${FG_StaticLibrary_Headers}
    ${FG_StaticLibrary_Sources}
)
## Target definitions
add_executable(${EXECUTABLE_NAME}
    ${FG_Application_Headers}
    ${FG_Application_Sources}
)

## Target libraries - FG_ALL_LIBRARIES contains everything
target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${FG_ALL_LIBRARIES})
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${STATIC_LIB_NAME})
target_link_libraries(${EXECUTABLE_NAME} PRIVATE V8::V8)
target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)
if(OS MATCHES "windows")
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_CONFIGURATION_TYPE "Application")
    target_compile_options(${EXECUTABLE_NAME} PRIVATE /MP${CMAKE_BUILD_PARALLEL_LEVEL})

    set_target_properties(${STATIC_LIB_NAME} PROPERTIES VS_CONFIGURATION_TYPE "StaticLibrary")
    target_compile_options(${STATIC_LIB_NAME} PRIVATE /MP${CMAKE_BUILD_PARALLEL_LEVEL})
endif() # OS

message(STATUS "Output (bin): ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release")
file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug")
add_custom_command(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${V8_SNAPSHOT_BLOB}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
    COMMAND ${CMAKE_COMMAND} -E copy "${V8_SNAPSHOT_BLOB}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
)
