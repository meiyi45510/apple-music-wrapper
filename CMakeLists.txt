cmake_minimum_required(VERSION 3.25)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

set(
    ANDROID_NDK_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/android-ndk-r23b"
    CACHE PATH "ndk path"
)
set(TOOLCHAIN "${ANDROID_NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64")

set(CMAKE_C_COMPILER "${TOOLCHAIN}/bin/aarch64-linux-android22-clang")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN}/bin/aarch64-linux-android22-clang++")

project(wrapper LANGUAGES C CXX)

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug
        Release
        RelWithDebInfo
        MinSizeRel
    )
endif()

file(
    STRINGS
    "${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
    BUILD_VERSION
    LIMIT_COUNT 1
)
if(NOT BUILD_VERSION)
    message(FATAL_ERROR "VERSION file is missing or empty")
endif()

add_executable(
    wrapper_core
    server.c
    logger.c
    parser.c
    bridge.cpp
)

target_compile_definitions(
    wrapper_core
    PRIVATE
        WRAPPER_VERSION="${BUILD_VERSION}"
)

set_target_properties(
    wrapper_core
    PROPERTIES
        OUTPUT_NAME "wrapper-core"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/rootfs/system/bin"
)

set(SYSTEM_LIB_DIR "${CMAKE_SOURCE_DIR}/rootfs/system/lib64")
find_library(
    ANDROIDAPPMUSIC_LIB
    androidappmusic
    PATHS "${SYSTEM_LIB_DIR}"
)
find_library(
    STORESERVICESCORE_LIB
    storeservicescore
    PATHS "${SYSTEM_LIB_DIR}"
)
find_library(
    MEDIAPLATFORM_LIB
    mediaplatform
    PATHS "${SYSTEM_LIB_DIR}"
)
find_library(
    CXX_SHARED_LIB
    c++_shared
    PATHS "${SYSTEM_LIB_DIR}"
)

target_link_libraries(
    wrapper_core
    PRIVATE
        ${CXX_SHARED_LIB}
        ${ANDROIDAPPMUSIC_LIB}
        ${STORESERVICESCORE_LIB}
        ${MEDIAPLATFORM_LIB}
        dl
)

target_link_options(wrapper_core PRIVATE -Wl,--export-dynamic)

target_compile_options(
    wrapper_core
    PRIVATE
        -Werror
        $<$<CONFIG:Release>:-Wall>
        $<$<NOT:$<CONFIG:Release>>:-g>
)

set(BOOT_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/wrapper")

add_custom_command(
    OUTPUT ${BOOT_OUTPUT}
    COMMAND
        aarch64-linux-gnu-gcc -O3 -Wall -o ${BOOT_OUTPUT}
        ${CMAKE_CURRENT_SOURCE_DIR}/launch.c
        ${CMAKE_CURRENT_SOURCE_DIR}/logger.c
    DEPENDS wrapper_core launch.c logger.c
    VERBATIM
)

add_custom_target(
    boot
    ALL
    DEPENDS ${BOOT_OUTPUT}
)
