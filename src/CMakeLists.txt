
include_directories(${FG_ALL_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

message(STATUS "Include: '${FG_ALL_INCLUDE_DIRS}'")
message(STATUS "Libraries: '${FG_ALL_LIBRARIES}'")

set(CMAKE_VERBOSE_MAKEFILE ON)

#
# Main sources (core lib & code)
set(FlexiGame_FG_Main_Headers
)
set(FlexiGame_FG_Main_Sources
    main.cpp
    test.cpp
    serverx.cpp
)

source_group(main FILES ${FlexiGame_FG_Main_Sources})
source_group(main\\inc FILES ${FlexiGame_FG_Main_Headers})

#------------------------------------------------------------------------------

add_definitions(-DUSE_PRE_CONFIG)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DRELEASE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
    add_definitions(-DDEBUG)
endif()
add_definitions(-D_HAS_AUTO_PTR_ETC=1)

## Target definitions
add_executable(${EXECUTABLE_NAME}
    ${FlexiGame_FG_Main_Headers}
    ${FlexiGame_FG_Main_Sources}
)
## Target libraries - FG_ALL_LIBRARIES contains everything
target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${FG_ALL_LIBRARIES} winmm.lib)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE V8::V8)
target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

message(STATUS "Output (bin): ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release")
file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug")
add_custom_command(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${V8_SNAPSHOT_BLOB}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
    COMMAND ${CMAKE_COMMAND} -E copy "${V8_SNAPSHOT_BLOB}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
)