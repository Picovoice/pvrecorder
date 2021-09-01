cmake_minimum_required(VERSION 3.4)
project(pv_recorder_napi)

include(ExternalProject)

set(NODE_VERSION 14.17.5)

ExternalProject_Add(node_headers
  URL https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-headers.tar.xz
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(node_headers SOURCE_DIR)
set(NODE_INCLUDE_DIR ${SOURCE_DIR}/include/node/)

if (WIN32)
  set(NODE_WIN_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/node_win_lib )
  ExternalProject_Add(node_win_lib
    URL https://nodejs.org/dist/v${NODE_VERSION}/win-x64/node.exe 
    DOWNLOAD_NO_EXTRACT on
    DOWNLOAD_DIR ${NODE_WIN_LIB_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND gendef.exe ${NODE_WIN_LIB_DIR}/node.exe 
          COMMAND dlltool.exe -d node.def -y ${NODE_WIN_LIB_DIR}/node.a
    INSTALL_COMMAND ""
  )
  set(NODE_LIB ${NODE_WIN_LIB_DIR}/node.a)
endif()

add_library(${PROJECT_NAME} SHARED pv_recorder_napi.c $<TARGET_OBJECTS:pv_recorder_object>)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    OUTPUT_NAME "pv_recorder"
    SUFFIX ".node"
)

add_dependencies(${PROJECT_NAME} node_headers)

if (WIN32)
    add_dependencies(${PROJECT_NAME} node_win_lib)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../include)
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${NODE_LIB})

if (APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE "-undefined" "dynamic_lookup")
endif()

if(DEFINED OUTPUT_DIR)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${PROJECT_NAME}>
            "${CMAKE_SOURCE_DIR}/lib/node/${OUTPUT_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            COMMENT "Copying node lib to output directory.")
endif()
