/**
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
 */
#include "sdmt.h"

#include <gtest/gtest.h>

TEST(RecoveryTest, Int1D) {
    // initialize sdmt module
    SDMT::init("./config_cpp_test.xml", false);

    // request a sdmt snapshot
    // define 1 dimensional integer array
    // the size of array is 1024
    SDMT::register_snapshot("sdmttest_int1d", SDMT_INT, SDMT_ARRAY, {1024});

    // get data snapshot
    int* ptr = SDMT::intptr("sdmttest_int1d");

    // write values to snapshot memory
    for (int i = 0; i < 1024; i++) {
        ptr[i] = i * i;
    }
    
    // start sdmt module
    SDMT::start();

    // generate checkpoint
    SDMT::checkpoint(1);

    // overwrite dummy values to snapshot memory
    for (int i = 0; i < 1024; i++) {
        ptr[i] = 0;
    }

    // recover checkpoint
    SDMT::recover();

    // check recovered values
    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(ptr[i], i * i);
    }

    // finalize sdmt module
    SDMT::finalize();
}
