SET(checkpoint_SRC
	fortran_checkpoint.f90
	)

ADD_EXECUTABLE(fortran_checkpoint ${checkpoint_SRCS})

TARGET_LINK_LIBRARIES(fortran_checkpoint sdmt) 
TARGET_LINK_LIBRARIES(fortran_checkpoint ${LIBS} sdmtfortran)

add_custom_target(copy-files-ex-fortran_test1 ALL
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_SOURCE_DIR}/test/fortran/config_fortran_test.xml
	${CMAKE_CURRENT_BINARY_DIR}/config_fortran_test.xml
)

add_custom_target(copy-files-ex-fortran_test2 ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/test/fortran/checkpoint
    ${CMAKE_CURRENT_BINARY_DIR}/checkpoint/
)
