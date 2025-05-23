cmake_minimum_required(VERSION 3.10)
project(pv_recorder_demo VERSION 1.2.0 DESCRIPTION "Picovoice audio recorder library demo.")

if(NOT PV_RECORDER_PLATFORM)
    message(FATAL_ERROR "No `PV_RECORDER_PLATFORM` value was given. Valid platforms are: \n"
            "linux, mac-arm64, mac-x86_64, windows-amd64, windows-arm64, "
            "raspberry-pi, raspberry-pi3-32, raspberry-pi3-64, "
            "raspberry-pi4-32, raspberry-pi4-64, raspberry-pi5-32, raspberry-pi5-64")
endif()

if (${PV_RECORDER_PLATFORM} STREQUAL "mac-arm64")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/mac/arm64)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "mac-x86_64")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/mac/x86_64)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "linux")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/linux/x86_64)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/raspberry-pi/arm11)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi3")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/raspberry-pi/cortex-a53)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi3-64")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/raspberry-pi/cortex-a53-aarch64)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi4")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/raspberry-pi/cortex-a72)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi4-64")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/raspberry-pi/cortex-a72-aarch64)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi5")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/raspberry-pi/cortex-a76)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "raspberry-pi5-64")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/raspberry-pi/cortex-a76-aarch64)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "windows-amd64")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/windows/amd64)
elseif (${PV_RECORDER_PLATFORM} STREQUAL "windows-arm64")
    set(PV_RECORDER_LIB_DIR ${CMAKE_SOURCE_DIR}/../../lib/windows/arm64)
else ()
    message(FATAL_ERROR "Unknown platform `${PV_RECORDER_PLATFORM}`.")
endif ()

include_directories(../../project/include)
link_directories(${PV_RECORDER_LIB_DIR})
add_executable(pv_recorder_demo pv_recorder_demo.c)
target_link_libraries(pv_recorder_demo pv_recorder)

add_custom_command(TARGET pv_recorder_demo
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PV_RECORDER_LIB_DIR} ${CMAKE_BINARY_DIR}
)
