ADD_SUBDIRECTORY(cpp)
ADD_SUBDIRECTORY(fortran)

#add_custom_target(copy-files-ex ALL
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${CMAKE_SOURCE_DIR}/example/python
#    ${CMAKE_CURRENT_BINARY_DIR}/python)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/python)
