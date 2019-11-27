#include <iostream>
#include "sdmt.h"

using namespace std;


SDMT_Code sdmt_init(char *config, bool &restart) {
    cout << "[SDMT] [C API] init" << endl;
    return SDMT::init(config, restart);
}

SDMT_Code sdmt_register_segment(std::string name, SDMT_VT vt, SDMT_DT dt, std::vector<int> dim) {
    std::cout << "[SDMT] [C API] register_segment" << endl;
    return SDMT::register_segment(name, vt, dt, dim);
}

int* sdmt_intptr(std::string name) {
    std::cout << "[SDMT] [C API] intptr" << endl;
    return SDMT::intptr(name);
}

SDMT_Code sdmt_start() {
    std::cout << "[SDMT] [C API] start" << endl;
    return SDMT::start();
}

SDMT_Code sdmt_checkpoint(int level) {
    std::cout << "[SDMT] [C API] checkpoint" << endl;
    return SDMT::checkpoint(level);
}

SDMT_Code sdmt_recover() {
    std::cout << "[SDMT] [C API] recover" << endl;
    return SDMT::recover();
}

SDMT_Code sdmt_finalize() {
    cout << "[SDMT] [C API] finalize" << endl;
    return SDMT::finalize();
}

extern "C" {
    SDMT_Code sdmt_init_c_(char *config, bool* restart) {
        return sdmt_init(config, *restart);
    }

    SDMT_Code sdmt_register_segment_c_(std::string name, SDMT_VT vt, SDMT_DT dt, std::vector<int> dim) {
        return sdmt_register_segment(name, vt, dt, dim);
    }

    int* sdmt_intptr_c_(std::string name) {
        return sdmt_intptr(name);
    }

    SDMT_Code sdmt_start_c_() {
        return sdmt_start();
    }

    SDMT_Code sdmt_checkpoint_c_(int level) {
        return sdmt_checkpoint(level);
    }

    SDMT_Code sdmt_recover_c_() {
        return sdmt_recover();
    }

    SDMT_Code sdmt_finalize_c_() {
        return sdmt_finalize();
    }
}
