cmake_minimum_required(VERSION 3.10)
project(graphical_test)

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.mm")
file(GLOB_RECURSE TEST_FILES "test/*.cpp")

list(REMOVE_ITEM SRC_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowLinux.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowWindows.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowMac.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/window/window.cpp"
)

add_executable(graphical_test ${TEST_FILES})
target_include_directories(graphical_test PRIVATE include src)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowLinux.cpp")
    find_package(X11 REQUIRED)
    target_link_libraries(graphical_test PRIVATE X11::X11)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowWindows.cpp")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowMac.mm")
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(graphical_test PRIVATE ${COCOA_LIBRARY})
endif()

target_sources(graphical_test PRIVATE ${SRC_FILES})
