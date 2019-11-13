// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#include "sdmt.h"

#include <gtest/gtest.h>

TEST(ParameterTest, 1st) {
    // initialize sdmt module
    SDMT::init("./config_cpp_test.xml", false);


    // register parameter initial value
    SDMT::register_int_parameter("test_para", 100);


    // get parameter from parameter.txt file
    int parameter = SDMT::get_int_parameter("test_para");

    // check whether parameter value read is same as initial
    EXPECT_EQ(parameter, 100);
    

    // finalize sdmt module
    SDMT::finalize();
}

TEST(ParameterTest, 2nd) {
    // initialize sdmt module
    SDMT::init("./config_cpp_test2.xml", false);


    // get parameter from parameter.txt file
    int parameter = SDMT::get_int_parameter("test_para");

    // check whether parameter value read is same as initial
    EXPECT_EQ(parameter, 200);
    

    // finalize sdmt module
    SDMT::finalize();
}
