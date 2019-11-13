# Set files to create library
# If you add new source file(s), add file's name to following lines
SET(SRCS
    test_checkpoint
    test_restart
    test_iter
    test_parameter
    )

ADD_EXECUTABLE(unit_test ${SRCS})
TARGET_LINK_LIBRARIES(unit_test sdmt)
TARGET_LINK_LIBRARIES(unit_test ${TEST_ONLY_LIBS} ${LIBS} gtest_main)

add_custom_command(TARGET unit_test PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/test/cpp/config_cpp_test.xml
    ${CMAKE_CURRENT_BINARY_DIR}/config_cpp_test.xml
)

add_custom_command(TARGET unit_test PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/test/cpp/checkpoint
    ${CMAKE_CURRENT_BINARY_DIR}/checkpoint/
)


ADD_TEST(unit_test unit_test)
