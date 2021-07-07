CMAKE_MINIMUM_REQUIRED (VERSION 3.5)
PROJECT(DSMT CXX)

### For Fortran build
enable_language(Fortran)
IF(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
	SET(dialect "-ffree-form -std=f2008 -fimplicit-none")
	SET(bounds "-fbounds-check")
ENDIF()
IF(CMAKE_Fortran_COMPILER_ID MATCHES "Intel")
	SET(dialect "-stand f08 -free -implicitnone")
	SET(bounds "-check bounds")
ENDIF()
IF(CMAKE_Fortran_COMPILER_ID MATCHES "PGI")
	SET(dialect "-Mfreeform -Mdclchk -Mstandard -Mallocatable=03")
	SET(bounds "-C")
ENDIF()

SET(CMAKE_FORTRAN_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} ${bounds}")
SET(CMAKE_FORTRAN_FLAGS "${CMAKE_Fortran_FLAGS} $dialect}")

# Set default build type
IF(NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "Build type not set - defaulting to Release")
    SET(
        CMAKE_BUILD_TYPE "Release"
        CACHE
        STRING
        "Choose the type of build from: Debug Release RelWithDebInfo MinSizeRel Coverage."
        FORCE)
ENDIF()

# Use C++11 as standard
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_STANDARD 11)

# Where to find cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
include(AppendProperty)

###############################################################################
# Find necessary libraries and include dirs
###############################################################################

# Find GTest headers and libs
ADD_SUBDIRECTORY(thirdparty/googletest)
INCLUDE_DIRECTORIES(SYSTEM thirdparty/googletest/googletest/include)

# Pybind11
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/thirdparty/pybind11/include)

# Find FTI headers and libs
INCLUDE_DIRECTORIES(SYSTEM thirdparty/fti/include)
find_library(FTI_LIB NAMES fti PATHS ${CMAKE_SOURCE_DIR}/thirdparty/fti/lib NO_DEFAULT_PATH)
SET(LIBS ${LIBS} ${FTI_LIB})

# Find OpenMPI headers and libs
project(FTI C CXX Fortran)
find_package(MPI REQUIRED)

INCLUDE_DIRECTORIES(SYSTEM /usr/include)
INCLUDE_DIRECTORIES(${MPI_Fortran_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${MPI_C_INCLUDE_PATH})
SET(LIBS ${LIBS} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})

# Enable c++11 (a.k.a. c++0x)
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -msse4.2 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D__STRICT_ANSI__ -fPIC")
endif()

# Include dir
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(SRCS
    src/sdmt
    src/tinyxml2
    )
ADD_LIBRARY(sdmt SHARED ${SRCS})
TARGET_LINK_LIBRARIES(sdmt PUBLIC ${LIBS})

SET(PYTHON_SRCS
    src/pybind.cc
    )

ADD_SUBDIRECTORY(thirdparty/pybind11)
PYBIND11_ADD_MODULE(sdmtpy MODULE ${PYTHON_SRCS})
TARGET_LINK_LIBRARIES(sdmtpy PUBLIC sdmt)

SET(FORTRAN_SRCS
	src/sdmt_capi.cc
	src/sdmt_mod.f90
	)

ADD_LIBRARY(sdmtfortran SHARED ${SRCS} ${FORTRAN_SRCS})
TARGET_LINK_LIBRARIES(sdmtfortran PUBLIC sdmt ${LIBS})

# installation config
install(TARGETS sdmt sdmtpy sdmtfortran DESTINATION lib)
install(FILES "src/sdmt.h" DESTINATION include)
install(FILES "src/common.h" DESTINATION include)
install(FILES "src/tinyxml2.h" DESTINATION include)
install(FILES "src/serialization.h" DESTINATION include)
install(FILES "thirdparty/fti/include/fti.h" DESTINATION include)
install(FILES "thirdparty/fti/include/fti-intern.h" DESTINATION include)
install(FILES "thirdparty/fti/lib/libfti.a" DESTINATION lib)
install(FILES "thirdparty/fti/lib/libfti.so" DESTINATION lib)

# Add test sub-directory
ENABLE_TESTING()
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(example)

# sdmt_graph_tool
SET(CMAKE_CXX_STANDARD 17)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
SET (sdmt_graph_tool
	src/sdmt_graph_tool/analyzer.h
	src/sdmt_graph_tool/communication.h
	src/sdmt_graph_tool/global_file_IO.h
       	src/sdmt_graph_tool/global_file_IO.cpp
	src/sdmt_graph_tool/global_graph.h
	src/sdmt_graph_tool/global_graph_stat.h
	src/sdmt_graph_tool/GPSPE_main.cpp
	src/sdmt_graph_tool/graph_class.h
	src/sdmt_graph_tool/graph_stat_tools.h
	src/sdmt_graph_tool/partitioning_strategies.h
	src/sdmt_graph_tool/TextTable.h
	src/sdmt_graph_tool/tools.h
)
find_library(
	MPI_CXX
	NAMES libmpi.so
	HINTS /usr/local/lib
	REQUIRED
)
find_package(Boost REQUIRED COMPONENTS system iostreams serialization filesystem program_options)
include_directories(${Boost_INCLUDE_DIR})

#find_package(MPI REQUIRED)
#include_directories(thirdparty/openmpi/include)
include_directories("/usr/local/include")

add_executable(GPSPE ${sdmt_graph_tool})
target_link_libraries(GPSPE ${MPI_CXX} ${Boost_LIBRARIES})

INSTALL(FILES models/sdmt_graph_tool_xgboost_model
	DESTINATION /usr/local/sdmt/sdmt_graph_tool/models/)
INSTALL(FILES models/sdmt_graph_tool_xgboost_scaler
        DESTINATION /usr/local/sdmt/sdmt_graph_tool/models/)
INSTALL(FILES python/sdmt/sdmt_graph_tool_select_PS.py
        DESTINATION /usr/local/sdmt/sdmt_graph_tool/py_codes/)
INSTALL(FILES jar/sdmt_graph_tool_united_code_extract.jar
        DESTINATION /usr/local/sdmt/sdmt_graph_tool/java/)
# sdmt_graph_tool end

###############################################################################
# Summary
###############################################################################
MESSAGE(STATUS "")
MESSAGE(STATUS "********** Summary **********")
MESSAGE(STATUS "General:")
MESSAGE(STATUS "  CMake version         : ${CMAKE_VERSION}")
MESSAGE(STATUS "  CMake command         : ${CMAKE_COMMAND}")
MESSAGE(STATUS "  System                : ${CMAKE_SYSTEM_NAME}")
MESSAGE(STATUS "  C++ compiler          : ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS "  C++ compiler version  : ${CMAKE_CXX_COMPILER_VERSION}")
MESSAGE(STATUS "  CXX flags             : ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "  Build type            : ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "  CMAKE_PREFIX_PATH     : ${CMAKE_PREFIX_PATH}")
MESSAGE(STATUS "  CMAKE_INSTALL_PREFIX  : ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "")

get_property(ALL_INCLUDES DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
MESSAGE(STATUS "  All includes          : ${ALL_INCLUDES}")
MESSAGE(STATUS "  All libraries         : ${LIBS}")
MESSAGE(STATUS "")
