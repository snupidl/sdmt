SET(SRCS
	fort_monte_carlo.f90
	)

ADD_EXECUTABLE(fortran_monte_carlo ${SRCS})

TARGET_LINK_LIBRARIES(fortran_monte_carlo sdmt) 
TARGET_LINK_LIBRARIES(fortran_monte_carlo ${LIBS} sdmtfortran)

add_custom_target(copy-files-ex-fortran_ex1 ALL
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_SOURCE_DIR}/example/fortran/config.xml
	${CMAKE_CURRENT_BINARY_DIR}/config.xml
)

add_custom_target(copy-files-ex-fortran_ex2 ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/example/fortran/checkpoint
    ${CMAKE_CURRENT_BINARY_DIR}/checkpoint/
)
