ADD_SUBDIRECTORY(cpp)
#ADD_SUBDIRECTORY(fortran)

add_custom_target(copy-files-ex ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/example/python
    ${CMAKE_CURRENT_BINARY_DIR}/python)
