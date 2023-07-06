cmake_minimum_required(VERSION 3.4)
project(pv_recorder VERSION 1.2.0 DESCRIPTION "Picovoice audio recorder library.")

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

option(PV_BUILD_TESTS "Build and run library tests" ON)
option(PV_BUILD_NODE "Build Node.js libraries" ON)

if(NOT PV_RECORDER_PLATFORM)
    message(FATAL_ERROR "No `PV_RECORDER_PLATFORM` value was given. Valid platforms are: \n"
            "linux, mac-arm64, mac-x86_64, windows, raspberry-pi, raspberry-pi2, "
            "raspberry-pi3-32, raspberry-pi3-64, raspberry-pi4-32, raspberry-pi4-64, "
            "jetson-nano, beaglebone")
endif()

if (${PV_RECORDER_PLATFORM} STREQUAL "beaglebone")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_BEAGLEBONE__)
    add_compile_options(-mcpu=cortex-a8 -mtune=cortex-a8 -mfpu=neon)
    add_link_options(-mcpu=cortex-a8 -mtune=cortex-a8 -mfpu=neon)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "mac-arm64")
    add_definitions(-D__PV_RECORDER_PLATFORM_DARWIN__)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
elseif (${PV_RECORDER_PLATFORM} STREQUAL "mac-x86_64")
    add_definitions(-D__PV_RECORDER_PLATFORM_DARWIN__)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
elseif (${PV_RECORDER_PLATFORM} STREQUAL "linux")
    add_definitions(-D__PV_RECORDER_PLATFORM_LINUX__)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "jetson-nano")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_JETSON__)
    add_compile_options(-mcpu=cortex-a57 -mtune=cortex-a57)
    add_link_options(-mcpu=cortex-a57 -mtune=cortex-a57)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_RASPBERRYPI__)
    add_compile_options(-mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp)
    add_link_options(-mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi2")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_RASPBERRYPI__)
    add_compile_options(-mcpu=cortex-a7 -mtune=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4)
    add_link_options(-mcpu=cortex-a7 -mtune=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi3")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_RASPBERRYPI__)
    add_compile_options(-mcpu=cortex-a53 -mtune=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8)
    add_link_options(-mcpu=cortex-a53 -mtune=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi3-64")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_RASPBERRYPI__)
    add_compile_options(-mcpu=cortex-a53 -mtune=cortex-a53)
    add_link_options(-mcpu=cortex-a53 -mtune=cortex-a53)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi4")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_RASPBERRYPI__)
    add_compile_options(-mcpu=cortex-a72 -mtune=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8)
    add_link_options(-mcpu=cortex-a72 -mtune=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi4-64")
    set(PV_LINK_ATOMIC ON)
    add_definitions(-D__PV_RECORDER_PLATFORM_RASPBERRYPI__)
    add_compile_options(-mcpu=cortex-a72 -mtune=cortex-a72)
    add_link_options(-mcpu=cortex-a72 -mtune=cortex-a72)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "windows")
    add_definitions(-D__PV_RECORDER_PLATFORM_WINDOWS__)
else ()
    message(FATAL_ERROR "Unknown platform `${PV_RECORDER_PLATFORM}`.")
endif ()

add_library(pv_recorder_object OBJECT src/pv_circular_buffer.c src/pv_recorder.c)
target_include_directories(pv_recorder_object PUBLIC include)
target_include_directories(pv_recorder_object PRIVATE src/miniaudio)

add_library(pv_recorder SHARED $<TARGET_OBJECTS:pv_recorder_object>)
set_target_properties(pv_recorder PROPERTIES PUBLIC_HEADER include/pv_recorder.h)
target_include_directories(pv_recorder PUBLIC include)

if (NOT ${PV_RECORDER_PLATFORM} STREQUAL "windows")
    target_link_libraries(pv_recorder PRIVATE pthread dl m)
    if(PV_LINK_ATOMIC)
        target_link_libraries(pv_recorder PRIVATE atomic)
    endif()
endif()

if(DEFINED OUTPUT_DIR)
    add_custom_command(TARGET pv_recorder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${PROJECT_NAME}>
            "${CMAKE_SOURCE_DIR}/../lib/${OUTPUT_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            COMMENT "Copying to output directory.")
endif()

if (PV_BUILD_TESTS)
    enable_testing()

    add_executable(test_circular_buffer test/test_pv_circular_buffer.c src/pv_circular_buffer.c)
    target_include_directories(test_circular_buffer PUBLIC include)
    add_test(
            NAME test_circular_buffer
            COMMAND test_circular_buffer
    )

    add_executable(test_recorder test/test_pv_recorder.c)
    target_link_libraries(test_recorder pv_recorder)
    add_test(
            NAME test_recorder
            COMMAND test_recorder
    )
endif()

if (PV_BUILD_NODE)
    add_subdirectory(node)
endif()