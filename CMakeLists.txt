cmake_minimum_required(VERSION 3.10)
project(graphical_test)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE TEST_FILES "test/*.cpp")

add_executable(graphical_test ${SRC_FILES} ${TEST_FILES})
target_include_directories(graphical_test PRIVATE include)
