cmake_minimum_required(VERSION 3.16)

project(rtk_graphical VERSION 1.2.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(APPLE)
    enable_language(OBJCXX)
endif()

option(RTK_REGENERATE_SHADERS "Regenerate embedded SPIR-V shaders" OFF)

file(GLOB_RECURSE RTK_GRAPHICAL_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

list(REMOVE_ITEM RTK_GRAPHICAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowLinux.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowWindows.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/window/window.cpp")

if(APPLE)
    list(APPEND RTK_GRAPHICAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowMac.mm")
elseif(WIN32)
    list(APPEND RTK_GRAPHICAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowWindows.cpp")
elseif(UNIX)
    list(APPEND RTK_GRAPHICAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowLinux.cpp")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

option(RTK_BUILD_SHARED "Build the shared library" ON)
option(RTK_BUILD_STATIC "Build the static library (single binary, no rtk_graphical shared lib to ship)" OFF)

if(NOT RTK_BUILD_SHARED AND NOT RTK_BUILD_STATIC)
    message(FATAL_ERROR "Enable at least one of RTK_BUILD_SHARED and RTK_BUILD_STATIC")
endif()

find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(MINIAUDIO_INCLUDE_DIR NAMES miniaudio.h REQUIRED)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

set(RTK_TARGETS)

if(RTK_BUILD_SHARED)
    add_library(rtk_graphical SHARED ${RTK_GRAPHICAL_SOURCES})
    set_target_properties(rtk_graphical PROPERTIES OUTPUT_NAME rtk_graphical VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
                                                   WINDOWS_EXPORT_ALL_SYMBOLS ON)
    list(APPEND RTK_TARGETS rtk_graphical)
endif()

if(RTK_BUILD_STATIC)
    add_library(rtk_graphical_static STATIC ${RTK_GRAPHICAL_SOURCES})
    # on Windows the import lib of the shared build is already rtk_graphical.lib
    if(WIN32)
        set_target_properties(rtk_graphical_static PROPERTIES OUTPUT_NAME rtk_graphical_static)
    else()
        set_target_properties(rtk_graphical_static PROPERTIES OUTPUT_NAME rtk_graphical)
    endif()
    list(APPEND RTK_TARGETS rtk_graphical_static)
endif()

list(GET RTK_TARGETS 0 RTK_MAIN_TARGET)
add_library(rtk::graphical ALIAS ${RTK_MAIN_TARGET})

# a static lib does not link its dependencies, they become the consumer's: PUBLIC
foreach(RTK_TARGET IN LISTS RTK_TARGETS)
    if(RTK_TARGET STREQUAL "rtk_graphical_static")
        set(RTK_LINK_SCOPE PUBLIC)
    else()
        set(RTK_LINK_SCOPE PRIVATE)
    endif()

    set_target_properties(${RTK_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_compile_features(${RTK_TARGET} PUBLIC cxx_std_20)

    target_include_directories(
        ${RTK_TARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${RTK_TARGET} PRIVATE $<$<CONFIG:Debug>:-Wall> $<$<CONFIG:Debug>:-Wextra> $<$<CONFIG:Release>:-O3>)
    endif()

    target_include_directories(
        ${RTK_TARGET} PRIVATE
        "${Stb_INCLUDE_DIR}"
        "${MINIAUDIO_INCLUDE_DIR}"
    )

    target_link_libraries(${RTK_TARGET} PUBLIC Vulkan::Vulkan glm::glm)

    if(APPLE)
        target_link_libraries(${RTK_TARGET} ${RTK_LINK_SCOPE}
            "-framework Cocoa" "-framework QuartzCore" "-framework CoreGraphics" "-framework Metal" "-framework GameController")

        set_target_properties(${RTK_TARGET} PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)

        target_compile_definitions(${RTK_TARGET} PRIVATE VK_USE_PLATFORM_METAL_EXT)
    elseif(WIN32)
        target_compile_definitions(${RTK_TARGET} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        target_link_libraries(${RTK_TARGET} ${RTK_LINK_SCOPE} xinput)
    elseif(UNIX)
        target_link_libraries(${RTK_TARGET} ${RTK_LINK_SCOPE} X11::X11)
    endif()
endforeach()

if(APPLE)
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowMac.mm" PROPERTIES LANGUAGE OBJCXX)

    if(RTK_BUILD_SHARED)
        add_custom_command(TARGET rtk_graphical POST_BUILD
            COMMAND sh -c "VULKAN_LINKED=$(otool -L $<TARGET_FILE:rtk_graphical> | grep libvulkan | awk '{print $1}'); if [ ! -z \"$VULKAN_LINKED\" ]; then install_name_tool -change \"$VULKAN_LINKED\" @rpath/libvulkan.1.dylib $<TARGET_FILE:rtk_graphical>; fi"
            VERBATIM
        )
    endif()
endif()

set(RTK_GENERATED_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(RTK_GENERATED_SHADER_DIRECTORY "${RTK_GENERATED_DIRECTORY}/rtk/shaders")

set(RTK_PRECOMPILED_SHADER_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/precompiled_shaders")

file(GLOB_RECURSE RTK_SHADER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.vert"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.frag")

set(RTK_GENERATED_SHADER_HEADERS)

if(RTK_REGENERATE_SHADERS)
    find_program(GLSLC_COMPILER NAMES glslc REQUIRED)

    if(NOT GLSLC_COMPILER)
        message(FATAL_ERROR "glslc compiler is required when RTK_REGENERATE_SHADERS is ON")
    endif()

    file(MAKE_DIRECTORY "${RTK_GENERATED_SHADER_DIRECTORY}")

    foreach(SHADER_SOURCE IN LISTS RTK_SHADER_SOURCES)
        file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders" "${SHADER_SOURCE}")
        string(TOLOWER "${REL_PATH}" LOWER_REL_PATH)
        string(REGEX REPLACE "[/\\\\.-]" "_" VAR_NAME "${LOWER_REL_PATH}")
        set(VAR_NAME "${VAR_NAME}_spv")

        set(SHADER_SPV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/${REL_PATH}.spv")
        set(SHADER_HEADER_OUTPUT "${RTK_GENERATED_SHADER_DIRECTORY}/${VAR_NAME}.hpp")

        get_filename_component(SHADER_SPV_DIR "${SHADER_SPV_OUTPUT}" DIRECTORY)
        file(MAKE_DIRECTORY "${SHADER_SPV_DIR}")

        add_custom_command(
            OUTPUT "${SHADER_SPV_OUTPUT}" COMMAND "${GLSLC_COMPILER}" "${SHADER_SOURCE}" -o "${SHADER_SPV_OUTPUT}" --target-env=vulkan1.2
            DEPENDS "${SHADER_SOURCE}" COMMENT "Compiling shader ${REL_PATH} to SPIR-V" VERBATIM)

        add_custom_command(
            OUTPUT "${SHADER_HEADER_OUTPUT}"
            COMMAND "${CMAKE_COMMAND}" -DINPUT_FILE=${SHADER_SPV_OUTPUT} -DOUTPUT_FILE=${SHADER_HEADER_OUTPUT} -DVARIABLE_NAME=${VAR_NAME}
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
            DEPENDS "${SHADER_SPV_OUTPUT}"
            COMMENT "Embedding SPIR-V shader ${REL_PATH} into header"
            VERBATIM)

        list(APPEND RTK_GENERATED_SHADER_HEADERS "${SHADER_HEADER_OUTPUT}")
    endforeach()

    foreach(RTK_TARGET IN LISTS RTK_TARGETS)
        target_include_directories(${RTK_TARGET} PRIVATE "${RTK_GENERATED_DIRECTORY}")
    endforeach()

    add_custom_target(
        update_precompiled_shaders COMMAND "${CMAKE_COMMAND}" -E copy_directory "${RTK_GENERATED_DIRECTORY}"
                                           "${RTK_PRECOMPILED_SHADER_DIRECTORY}" DEPENDS rtk_graphical_shaders
        COMMENT "Updating precompiled shaders")

else()
    foreach(SHADER_SOURCE IN LISTS RTK_SHADER_SOURCES)
        file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders" "${SHADER_SOURCE}")
        string(TOLOWER "${REL_PATH}" LOWER_REL_PATH)
        string(REGEX REPLACE "[/\\\\.-]" "_" VAR_NAME "${LOWER_REL_PATH}")
        set(VAR_NAME "${VAR_NAME}_spv")

        set(PRECOMPILED_HEADER "${RTK_PRECOMPILED_SHADER_DIRECTORY}/rtk/shaders/${VAR_NAME}.hpp")
        if(NOT EXISTS "${PRECOMPILED_HEADER}")
            message(
                FATAL_ERROR "Precompiled shader header missing: ${PRECOMPILED_HEADER}. Please set RTK_REGENERATE_SHADERS=ON to generate it."
            )
        endif()
        list(APPEND RTK_GENERATED_SHADER_HEADERS "${PRECOMPILED_HEADER}")
    endforeach()

    foreach(RTK_TARGET IN LISTS RTK_TARGETS)
        target_include_directories(${RTK_TARGET} PRIVATE "${RTK_PRECOMPILED_SHADER_DIRECTORY}")
    endforeach()
endif()

foreach(RTK_TARGET IN LISTS RTK_TARGETS)
    target_sources(${RTK_TARGET} PRIVATE ${RTK_GENERATED_SHADER_HEADERS})
endforeach()

set_source_files_properties(${RTK_GENERATED_SHADER_HEADERS} PROPERTIES GENERATED TRUE)

add_custom_target(rtk_graphical_shaders DEPENDS ${RTK_GENERATED_SHADER_HEADERS})

foreach(RTK_TARGET IN LISTS RTK_TARGETS)
    add_dependencies(${RTK_TARGET} rtk_graphical_shaders)
endforeach()

install(
    TARGETS ${RTK_TARGETS}
    EXPORT rtkGraphicalTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/MissingTexture.png")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/MissingTexture.png" DESTINATION "${CMAKE_INSTALL_DATADIR}/rtk-graphical/assets")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/rtk_graphicalConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/rtk_graphicalConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/rtk_graphical")

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/rtk_graphicalConfigVersion.cmake" VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

install(EXPORT rtkGraphicalTargets FILE rtkGraphicalTargets.cmake NAMESPACE rtk:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/rtk_graphical")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/rtk_graphicalConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/rtk_graphicalConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/rtk_graphical")
option(RTK_BUILD_TESTS "Build graphical tests" OFF)
if(RTK_BUILD_TESTS)
    add_executable(graphical_test test/manyTexture.cpp)

    target_link_libraries(graphical_test PRIVATE ${RTK_MAIN_TARGET})

    target_include_directories(graphical_test PRIVATE src include)
endif()
