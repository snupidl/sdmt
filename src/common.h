// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>

/**
 * @brief an enum type of value type
 */
enum SDMT_VT {
    /** 4 byte integer */
    SDMT_INT,
    /** 8 byte integer */
    SDMT_LONG,
    /** 4 byte floating point */
    SDMT_FLOAT,
    /** 8 byte floating point */
    SDMT_DOUBLE,
    /** none */
    SDMT_NUM_VT
};

/**
 * @brief an enum type of data structure
 */
enum SDMT_DT {
    /** scalar value */
    SDMT_SCALAR,
    /** 1 dimensional array */
    SDMT_ARRAY,
    /** 2 dimensional matrix */
    SDMT_MATRIX,
    /** tensor */
    SDMT_TENSOR,
    /** none */
    SDMT_NUM_DT
};

/**
 * @brief an enum type of return code
 */
enum SDMT_Code {
    /** the request completed successfully */
    SDMT_SUCCESS,
    /** duplicated segment name */
    SDMT_ERR_DUPLICATED_NAME,
    /** invalid value type */
    SDMT_ERR_WRONG_VALUE_TYPE,
    /** invalid value type */
    SDMT_ERR_WRONG_DATA_TYPE,
    /** definition of dimension does not match with data type */
    SDMT_ERR_WRONG_DIMENSION,
    /** failed to allocate new memory */
    SDMT_ERR_FAILED_ALLOCATION,
    /** failed to checkpoint */
    SDMT_ERR_FAILED_CHECKPOINT
};

#endif  // COMMON_H_
