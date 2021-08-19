cmake_minimum_required(VERSION 3.4)
project(pv_recorder VERSION 1.0.0 DESCRIPTION "Audio recorder library for Picovoice.")

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(pv_recorder SHARED src/pv_recorder.c)

set_target_properties(pv_recorder PROPERTIES
        PUBLIC_HEADER include/pv_recorder.h)

target_include_directories(pv_recorder PUBLIC include)
target_include_directories(pv_recorder PRIVATE src/miniaudio)

install(TARGETS pv_recorder
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
