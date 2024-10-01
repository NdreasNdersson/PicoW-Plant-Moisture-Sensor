cmake_minimum_required(VERSION 3.13...3.27)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories(${CMAKE_SOURCE_DIR}/test)
include_directories(${CMAKE_SOURCE_DIR}/test/stubs)

package_add_test(RestApiCommandHandlerTest
    ${CMAKE_CURRENT_SOURCE_DIR}/rest_api_command_handler_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rest_api_command_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../json_converter.cpp
)
