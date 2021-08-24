enable_testing()

add_executable(test_circular_buffer pv_circular_buffer_test.c ../src/pv_circular_buffer.c)

target_include_directories(test_circular_buffer PUBLIC ../include)
target_link_libraries(test_circular_buffer pthread)
add_test(
        NAME test_circular_buffer
        COMMAND test_circular_buffer
)

add_custom_command(
        TARGET test_circular_buffer
        COMMENT "Run Tests"
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test
        COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "^test_circular_buffer" --output-on-failures
)
