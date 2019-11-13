// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#include "sdmt.h"

#include <gtest/gtest.h>

TEST(ParameterTest, Test) {
    // initialize sdmt module
    SDMT::init("./config_cpp_test.xml", false);

    // request a sdmt segment
    // define 1 dimensional integer array
    // the size of array is 1024
    SDMT::register_segment("sdmttest_int1d", SDMT_INT, SDMT_ARRAY, {1024});


    // register parameter initial value
    SDMT::register_int_parameter("~/", "test_para", 100);


    // get data segment
    int* ptr = SDMT::intptr("sdmttest_int1d");

    // write values to segment memory
    for (int i = 0; i < 1024; i++) {
        ptr[i] = i * i;
    }
    
    // start sdmt module
    SDMT::start();

    // generate checkpoint
    SDMT::checkpoint(1);

    // overwrite dummy values to segment memory
    for (int i = 0; i < 1024; i++) {
        ptr[i] = 0;
    }

    // recover checkpoint
    SDMT::recover();

    // check recovered values
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(ptr[i], i * i);
    }

    // get parameter from parameter.txt file
    int parameter = SDMT::get_int_parameter("test_para");

    // check whether parameter value read is same as initial
    EXPECT_EQ(parameter, 100);
    

    // finalize sdmt module
    SDMT::finalize();
}