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

TEST(IterTest, Test) {
    // initialize sdmt module
    SDMT::init("./config_cpp_test.xml", false);

    // get iteration seqeunce
    int32_t& it = SDMT::iter();

    // increase iteration sequence
    // 0 -> 1
    ++it;

    // start sdmt module
    SDMT::start();

    // generate checkpoint
    SDMT::checkpoint(1);

    // increase iteration sequence
    // 1 -> 2
    ++it;

    // recover checkpoint
    SDMT::recover();

    // check recovered values
    EXPECT_EQ(it, 1);

    // finalize sdmt module
    SDMT::finalize();
}
