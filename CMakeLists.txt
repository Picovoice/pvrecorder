cmake_minimum_required(VERSION 3.4)
project(pv_recorder VERSION 1.0.0 DESCRIPTION "Audio recorder library for Picovoice.")

set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(pv_recorder_object OBJECT src/pv_circular_buffer.c src/pv_recorder.c)

target_include_directories(pv_recorder_object PUBLIC include)
target_include_directories(pv_recorder_object PRIVATE src/miniaudio)

add_library(pv_recorder SHARED $<TARGET_OBJECTS:pv_recorder_object>)

set_target_properties(pv_recorder PROPERTIES
        PUBLIC_HEADER include/pv_recorder.h)

if (NOT WIN32)
    target_link_libraries(pv_recorder pthread dl m)
endif()

if(DEFINED OUTPUT_DIR)
    add_custom_command(TARGET pv_recorder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${PROJECT_NAME}>
            "${CMAKE_SOURCE_DIR}/lib/${OUTPUT_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            COMMENT "Copying to output directory.")
endif()

add_executable(demo example/demo.c)

target_include_directories(demo PUBLIC include)

if (NOT WIN32)
    target_link_libraries(demo pv_recorder pthread dl m)
    if((${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") AND (UNIX AND NOT APPLE))
        target_link_libraries(demo pv_recorder atomic)
    endif()
else()
    target_link_libraries(demo pv_recorder)
endif()

enable_testing()

add_executable(test_circular_buffer test/test_pv_circular_buffer.c src/pv_circular_buffer.c)

target_include_directories(test_circular_buffer PUBLIC include)

add_test(
        NAME test_circular_buffer
        COMMAND test_circular_buffer
)

add_custom_command(
        TARGET test_circular_buffer
        COMMENT "Run Tests"
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "^test_circular_buffer" --output-on-failures
)

add_subdirectory(node)