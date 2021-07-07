SET(MONTE_CARLO_SRC
    monte_carlo
    )

ADD_EXECUTABLE(monte_carlo ${MONTE_CARLO_SRC})
TARGET_LINK_LIBRARIES(monte_carlo sdmt)

#add_custom_target(copy-files-ex-cpp1 ALL
#    COMMAND ${CMAKE_COMMAND} -E copy
#    ${CMAKE_SOURCE_DIR}/example/cpp/config_cpp_test.xml
#    ${CMAKE_CURRENT_BINARY_DIR}/config_cpp_test.xml
#)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config_cpp_test.xml  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config_cpp_test.xml)

#add_custom_target(copy-files-ex-cpp2 ALL
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${CMAKE_SOURCE_DIR}/example/cpp/checkpoint
#    ${CMAKE_CURRENT_BINARY_DIR}/checkpoint/
#)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/checkpoint DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/checkpoint)

