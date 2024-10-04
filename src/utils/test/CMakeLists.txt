cmake_minimum_required(VERSION 3.13...3.27)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories(${CMAKE_SOURCE_DIR}/test)
include_directories(${CMAKE_SOURCE_DIR}/test/stubs)

package_add_test(ConfigHandlerTest
    ${CMAKE_CURRENT_SOURCE_DIR}/config_handler_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../config_handler_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../json_converter.cpp
    ${CMAKE_SOURCE_DIR}/test/stubs/linker_definitions_stub.cpp
)
