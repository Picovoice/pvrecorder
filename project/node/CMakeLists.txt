cmake_minimum_required(VERSION 3.10)
project(pv_recorder_napi)

include(ExternalProject)

if (${PV_RECORDER_PLATFORM} STREQUAL "windows-amd64" OR ${PV_RECORDER_PLATFORM} STREQUAL "windows-arm64")
    if (NOT PV_WINDOWS_NODE_ARCH)
        message(FATAL_ERROR "`PV_WINDOWS_NODE_ARCH` is a required value")
    endif ()
endif ()

set(NODE_VERSION 18.16.1)
if (PV_WINDOWS_NODE_ARCH STREQUAL "win-arm64")
  set(NODE_VERSION 22.11.0)
endif ()

ExternalProject_Add(
  node_headers
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  URL https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-headers.tar.xz
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(node_headers SOURCE_DIR)
set(NODE_INCLUDE_DIR ${SOURCE_DIR}/include/node/)

if (${PV_RECORDER_PLATFORM} STREQUAL "windows-amd64" OR ${PV_RECORDER_PLATFORM} STREQUAL "windows-arm64")
  set(NODE_WIN_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/node_win_lib)

  set(dlltool_param "-y")
  if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(dlltool_param "-l")
  endif ()

  ExternalProject_Add(
    node_win_lib
    URL https://nodejs.org/dist/v${NODE_VERSION}/${PV_WINDOWS_NODE_ARCH}/node.exe
    DOWNLOAD_NO_EXTRACT on
    DOWNLOAD_DIR ${NODE_WIN_LIB_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND gendef.exe ${NODE_WIN_LIB_DIR}/node.exe
    COMMAND dlltool.exe -d node.def ${dlltool_param} ${NODE_WIN_LIB_DIR}/node.a
    INSTALL_COMMAND ""
  )
  set(NODE_LIB ${NODE_WIN_LIB_DIR}/node.a)
endif()

add_library(pv_recorder_napi SHARED pv_recorder_napi.c $<TARGET_OBJECTS:pv_recorder_object>)
set_target_properties(pv_recorder_napi PROPERTIES
    PREFIX ""
    OUTPUT_NAME "pv_recorder"
    SUFFIX ".node"
)
add_dependencies(pv_recorder_napi node_headers)

if (${PV_RECORDER_PLATFORM} STREQUAL "windows-amd64" OR ${PV_RECORDER_PLATFORM} STREQUAL "windows-arm64")
    add_dependencies(pv_recorder_napi node_win_lib)
    target_sources(pv_recorder_napi PRIVATE delayhook.c)
endif()

target_include_directories(pv_recorder_napi PUBLIC ${PROJECT_SOURCE_DIR}/../include)
target_include_directories(pv_recorder_napi PRIVATE ${NODE_INCLUDE_DIR})

target_link_libraries(pv_recorder_napi ${NODE_LIB})

if (${PV_RECORDER_PLATFORM} STREQUAL "mac-x86_64" OR ${PV_RECORDER_PLATFORM} STREQUAL "mac-arm64")
    target_link_options(pv_recorder_napi PRIVATE "-undefined" "dynamic_lookup")
endif()

if(DEFINED OUTPUT_DIR)
    add_custom_command(TARGET pv_recorder_napi POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${PROJECT_NAME}>
            "${CMAKE_SOURCE_DIR}/../lib/node/${OUTPUT_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            COMMENT "Copying node lib to output directory.")
endif()
