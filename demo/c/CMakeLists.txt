cmake_minimum_required(VERSION 3.4)
project(pv_recorder_demo VERSION 1.2.0 DESCRIPTION "Picovoice audio recorder library demo.")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(PV_BUILD_TESTS OFF)
set(PV_BUILD_NODE OFF)

add_subdirectory(../../project ../../project/build)

add_executable(pv_recorder_demo pv_recorder_demo.c)
target_link_libraries(pv_recorder_demo pv_recorder)