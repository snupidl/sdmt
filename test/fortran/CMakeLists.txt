SET(checkpoint_SRC
	fortran_checkpoint.f90
	)

SET(test_para_SRC
	fortran_parameter.f90
	)

ADD_EXECUTABLE(fortran_checkpoint ${checkpoint_SRC})

TARGET_LINK_LIBRARIES(fortran_checkpoint sdmt) 
TARGET_LINK_LIBRARIES(fortran_checkpoint ${LIBS} sdmtfortran)

ADD_EXECUTABLE(fortran_parameter ${test_para_SRC})

TARGET_LINK_LIBRARIES(fortran_parameter sdmt) 
TARGET_LINK_LIBRARIES(fortran_parameter ${LIBS} sdmtfortran)

#add_custom_target(copy-files-ex-fortran_test1 ALL
#	COMMAND ${CMAKE_COMMAND} -E copy
#	${CMAKE_CURRENT_SOURCE_DIR}/config_fortran_test.xml
#	${CMAKE_CURRENT_BINARY_DIR}/config_fortran_test.xml
#)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config_fortran_test.xml  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config_fortran_test.xml)

#add_custom_target(copy-files-ex-fortran_test2 ALL
#	COMMAND ${CMAKE_COMMAND} -E copy
#	${CMAKE_CURRENT_SOURCE_DIR}/config_fortran_test2.xml
#	${CMAKE_CURRENT_BINARY_DIR}/config_fortran_test2.xml
#)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config_fortran_test2.xml  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config_fortran_test2.xml)

#add_custom_target(copy-files-ex-fortran_test3 ALL
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${CMAKE_CURRENT_SOURCE_DIR}/checkpoint
#    ${CMAKE_CURRENT_BINARY_DIR}/checkpoint/
#)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/checkpoint DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/checkpoint)

