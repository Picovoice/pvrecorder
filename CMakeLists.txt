cmake_minimum_required(VERSION 3.4)
project(pv_recorder VERSION 1.0.0 DESCRIPTION "Audio recorder library for Picovoice.")

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(pv_recorder SHARED src/pv_recorder.c src/pv_circular_buffer.c)

set_target_properties(pv_recorder PROPERTIES
        PUBLIC_HEADER include/pv_recorder.h)

target_include_directories(pv_recorder PUBLIC include)
target_include_directories(pv_recorder PRIVATE src/miniaudio)

add_executable(demo EXCLUDE_FROM_ALL example/demo.c )

target_include_directories(demo PUBLIC include)

if (NOT WIN32)
    target_link_libraries(pv_recorder pthread dl m)
    target_link_libraries(demo pv_recorder pthread dl m)
else()
    target_link_libraries(demo pv_recorder)
endif()

add_subdirectory(test)

if(DEFINED OUTPUT_DIR)
    add_custom_command(TARGET pv_recorder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:pv_recorder>
            "${CMAKE_SOURCE_DIR}/${OUTPUT_DIR}/$<TARGET_FILE_NAME:pv_recorder>"
            COMMENT "Copying to output directory.")
endif()