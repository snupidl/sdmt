CMAKE_MINIMUM_REQUIRED (VERSION 3.5)
PROJECT(DSMT CXX)

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

INCLUDE_DIRECTORIES(SYSTEM /usr/include)

# Where to find cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
INCLUDE(${CMAKE_MODULE_PATH}/FindPackageCustom.cmake)

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
INCLUDE_DIRECTORIES(SYSTEM thirdparty/openmpi/include)
find_library(MPI_LIB NAMES mpi PATHS ${CMAKE_SOURCE_DIR}/thirdparty/openmpi/lib NO_DEFAULT_PATH)
find_library(MPICXX_LIB NAMES mpi_cxx PATHS ${CMAKE_SOURCE_DIR}/thirdparty/openmpi/lib NO_DEFAULT_PATH)
SET(LIBS ${LIBS} ${MPI_LIB})
SET(LIBS ${LIBS} ${MPICXX_LIB})

# Enable c++11 (a.k.a. c++0x)
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -msse4.2 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D__STRICT_ANSI__ -fPIC")
endif()

# Include dir
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(SRCS
    src/sdmt
    )
ADD_LIBRARY(sdmt SHARED ${SRCS})

SET(PYTHON_SRCS
    src/pybind.cc
    )

ADD_SUBDIRECTORY(thirdparty/pybind11)
PYBIND11_ADD_MODULE(sdmtpy MODULE ${PYTHON_SRCS})
TARGET_LINK_LIBRARIES(sdmtpy PUBLIC sdmt ${FTI_LIB})

# installation config
install(TARGETS sdmt sdmtpy DESTINATION lib)
install(FILES "src/sdmt.h" DESTINATION include)
install(FILES "src/common.h" DESTINATION include)

# Add test sub-directory
ENABLE_TESTING()
ADD_SUBDIRECTORY(test)

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
