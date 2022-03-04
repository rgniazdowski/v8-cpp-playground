set(UNIT_TESTS_EXE unit_tests)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(
    ${UNIT_TESTS_EXE}
    bindings.cpp
)

target_compile_definitions(${UNIT_TESTS_EXE} PUBLIC CATCH_CONFIG_MAIN)

target_include_directories(${UNIT_TESTS_EXE} PUBLIC ${Catch2_SOURCE_DIR}/single_include)
target_include_directories(${UNIT_TESTS_EXE} PUBLIC ${FG_ALL_INCLUDE_DIRS})
target_include_directories(${UNIT_TESTS_EXE} PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(${UNIT_TESTS_EXE} PUBLIC ${FG_ALL_LIBRARIES} winmm.lib)
target_link_libraries(${UNIT_TESTS_EXE} PRIVATE V8::V8)

target_compile_features(${UNIT_TESTS_EXE} PRIVATE cxx_std_17)

include(CTest)
include(Catch)
catch_discover_tests(${UNIT_TESTS_EXE})