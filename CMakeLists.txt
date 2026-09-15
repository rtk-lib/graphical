cmake_minimum_required(VERSION 3.16)

project(rtk_graphical VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(APPLE)
    enable_language(OBJCXX)
endif()

option(RTK_REGENERATE_SHADERS "Regenerate embedded SPIR-V shaders" ON)

set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/stb")

set(STB_IMAGE_HEADER "${STB_DIR}/stb_image.h")

if(NOT EXISTS "${STB_IMAGE_HEADER}")
    message(STATUS "Downloading stb_image.h...")

    file(MAKE_DIRECTORY "${STB_DIR}")

    file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "${STB_IMAGE_HEADER}" STATUS STB_DOWNLOAD_STATUS)

    list(GET STB_DOWNLOAD_STATUS 0 STB_DOWNLOAD_ERROR)

    if(NOT STB_DOWNLOAD_ERROR EQUAL 0)
        message(FATAL_ERROR "Failed to download stb_image.h")
    endif()
endif()

# --------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------

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

add_library(rtk_graphical SHARED ${RTK_GRAPHICAL_SOURCES})

add_library(rtk::graphical ALIAS rtk_graphical)

set_target_properties(rtk_graphical PROPERTIES OUTPUT_NAME rtk_graphical VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
                                               POSITION_INDEPENDENT_CODE ON WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_compile_features(rtk_graphical PUBLIC cxx_std_20)

target_include_directories(
    rtk_graphical PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${STB_DIR}")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rtk_graphical PRIVATE $<$<CONFIG:Debug>:-Wall> $<$<CONFIG:Debug>:-Wextra> $<$<CONFIG:Release>:-O3>)
endif()

find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(rtk_graphical PUBLIC Vulkan::Vulkan glm::glm)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)

    find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)

    find_library(COREGRAPHICS_LIBRARY CoreGraphics REQUIRED)

    find_library(METAL_LIBRARY Metal REQUIRED)

    target_link_libraries(rtk_graphical PRIVATE ${COCOA_LIBRARY} ${QUARTZCORE_LIBRARY} ${COREGRAPHICS_LIBRARY} ${METAL_LIBRARY})

    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/window/windowMac.mm" PROPERTIES LANGUAGE OBJCXX)

    set_target_properties(rtk_graphical PROPERTIES OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED YES)

    target_compile_definitions(rtk_graphical PRIVATE VK_USE_PLATFORM_METAL_EXT)

elseif(WIN32)

    target_compile_definitions(rtk_graphical PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

elseif(UNIX)

    find_package(X11 REQUIRED)

    target_link_libraries(rtk_graphical PRIVATE X11::X11)

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

    target_include_directories(rtk_graphical PRIVATE "${RTK_GENERATED_DIRECTORY}")

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

    target_include_directories(rtk_graphical PRIVATE "${RTK_PRECOMPILED_SHADER_DIRECTORY}")
endif()

target_sources(rtk_graphical PRIVATE ${RTK_GENERATED_SHADER_HEADERS})

set_source_files_properties(${RTK_GENERATED_SHADER_HEADERS} PROPERTIES GENERATED TRUE)

add_custom_target(rtk_graphical_shaders DEPENDS ${RTK_GENERATED_SHADER_HEADERS})

add_dependencies(rtk_graphical rtk_graphical_shaders)

install(
    TARGETS rtk_graphical
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
add_executable(graphical_test test/manyTexture.cpp)
target_link_libraries(graphical_test PRIVATE rtk_graphical)
target_include_directories(graphical_test PRIVATE src include)
