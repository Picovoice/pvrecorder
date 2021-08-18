cmake_minimum_required(VERSION 3.4)
project(pv_recorder VERSION 1.0.0 DESCRIPTION "Audio recorder library for Picovoice.")

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Release)

add_library(pv_recorder SHARED src/pv_recorder.c)

set_target_properties(pv_recorder PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER include/pv_recorder.h)

target_include_directories(pv_recorder PUBLIC include)
target_include_directories(pv_recorder PRIVATE src/miniaudio)

if (NOT WIN32)
    target_link_libraries(pv_recorder pthread dl m)
endif()

install(TARGETS pv_recorder
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include_directories("${PROJECT_SOURCE_DIR}/../porcupine/include")

add_executable(
        mic
        mic.c)

target_link_libraries(mic ${CMAKE_CURRENT_SOURCE_DIR}/libpv_recorder.a pthread m dl)
