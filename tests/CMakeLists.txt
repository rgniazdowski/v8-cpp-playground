set(UNIT_TESTS_EXE unit_tests)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(
    ${UNIT_TESTS_EXE}
    testmain.cpp
    bindings.cpp
    timers.cpp
    events.cpp
)

target_include_directories(${UNIT_TESTS_EXE} PUBLIC ${Catch2_SOURCE_DIR}/single_include)
target_include_directories(${UNIT_TESTS_EXE} PUBLIC ${FG_ALL_INCLUDE_DIRS})
target_include_directories(${UNIT_TESTS_EXE} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${UNIT_TESTS_EXE} PUBLIC ${FG_ALL_LIBRARIES})
target_link_libraries(${UNIT_TESTS_EXE} PRIVATE ${STATIC_LIB_NAME})
target_link_libraries(${UNIT_TESTS_EXE} PRIVATE V8::V8)
target_compile_features(${UNIT_TESTS_EXE} PRIVATE cxx_std_17)
if(OS MATCHES "windows")
    set_target_properties(${UNIT_TESTS_EXE} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    set_target_properties(${UNIT_TESTS_EXE} PROPERTIES VS_CONFIGURATION_TYPE "Application")
    target_compile_options(${UNIT_TESTS_EXE} PRIVATE /MP${CMAKE_BUILD_PARALLEL_LEVEL})
endif() # OS

include(CTest)
include(Catch)
catch_discover_tests(${UNIT_TESTS_EXE})