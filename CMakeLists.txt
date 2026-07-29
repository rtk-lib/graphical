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

find_package(Vulkan REQUIRED)
target_link_libraries(graphical_test PRIVATE Vulkan::Vulkan)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowLinux.cpp")
    find_package(X11 REQUIRED)
    target_link_libraries(graphical_test PRIVATE X11::X11)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowWindows.cpp")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(APPEND SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowMac.mm")
    find_library(COCOA_LIBRARY Cocoa)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    target_link_libraries(graphical_test PRIVATE ${COCOA_LIBRARY} ${QUARTZCORE_LIBRARY})
endif()

target_sources(graphical_test PRIVATE ${SRC_FILES})

find_program(GLSLC_COMPILER glslc)
if(GLSLC_COMPILER)
    file(GLOB_RECURSE SHADERS "src/shaders/*.vert" "src/shaders/*.frag")
    set(SHADER_OUTPUTS "")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
    foreach(SHADER ${SHADERS})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(OUTPUT_SPV "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
        add_custom_command(
            OUTPUT ${OUTPUT_SPV}
            COMMAND ${GLSLC_COMPILER} ${SHADER} -o ${OUTPUT_SPV}
            DEPENDS ${SHADER}
            COMMENT "Compiling shader ${SHADER_NAME}"
        )
        list(APPEND SHADER_OUTPUTS ${OUTPUT_SPV})
    endforeach()
    
    add_custom_target(compile_shaders ALL DEPENDS ${SHADER_OUTPUTS})
    add_dependencies(graphical_test compile_shaders)
else()
    message(WARNING "glslc not found. Shaders will not be compiled automatically.")
endif()
