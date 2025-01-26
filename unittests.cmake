
macro(package_add_test TESTNAME)
    # create an executable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    # link the Google test infrastructure, mocking library, and a default main function to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TESTNAME}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)

    target_link_libraries(${TESTNAME}
        nlohmann_json::nlohmann_json
    )
endmacro()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

enable_testing()
include(GoogleTest)

mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

SET(GCC_COVERAGE_COMPILE_FLAGS "-coverage")
SET(GCC_COVERAGE_LINK_FLAGS    "-coverage")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

find_program(LCOV lcov REQUIRED)
find_program(GENHTML genhtml REQUIRED)

add_custom_target(coverage
    COMMAND ${LCOV} --directory . --capture --output-file coverage.info
    COMMAND ${LCOV} --remove coverage.info -o coverage_filtered.info '/usr/*' '${CMAKE_CURRENT_SOURCE_DIR}/test/*' '${CMAKE_CURRENT_SOURCE_DIR}/build/*'
    COMMAND ${GENHTML} --demangle-cpp -o coverage coverage_filtered.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/network/test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/utils/config_handler/test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/PicoW-Bootloader/software_download/test)
