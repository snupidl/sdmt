ADD_SUBDIRECTORY(cpp)
ADD_SUBDIRECTORY(fortran)
#ADD_SUBDIRECTORY(python)
#install(DIRECTORY "${CMAKE_SOURCE_DIR}/test/python/" 
#    DESTINATION "${CMAKR_CURRENT_BINARY_DIR}/python" 
#    FILES_MATCHING PATTERN "*.py" )

add_custom_target(copy-files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/test/python
    ${CMAKE_CURRENT_BINARY_DIR}/python)
