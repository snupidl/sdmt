// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#include "sdmt.h"

#include <fti.h>

SDMT_Code SDMT::init_() {
    MPI_Init(nullptr, nullptr);
    FTI_Init("../checkpoint/config.fti", MPI_COMM_WORLD);

    int nbProcs, rank;
    MPI_Comm_size(FTI_COMM_WORLD, &nbProcs);
    MPI_Comm_rank(FTI_COMM_WORLD, &rank);

    FTIT_type ckptInfo;
    FTI_InitType(&ckptInfo, 2*sizeof(int));
    FTI_Protect(m_cp_idx++, &m_cp_info, 1, ckptInfo);

    return SDMT_SUCCESS;
}

SDMT_Code SDMT::start_() {
    MPI_Barrier(FTI_COMM_WORLD);
    return SDMT_SUCCESS;
}

SDMT_Code SDMT::finalize_() {
    FTI_Finalize();
    MPI_Finalize();

    return SDMT_SUCCESS;
}

SDMT_Code SDMT::register_segment_(
        std::string name,
        SDMT_VT vt,
        SDMT_DT dt,
        std::vector<int> dim) {
    // check duplicated name in segment map
    auto itr = m_sgmt_map.find(name);
    if (itr != m_sgmt_map.end()) {
        return SDMT_ERR_DUPLICATED_NAME;
    }

    // check the definition of data structure
    if (dt < SDMT_SCALAR || dt >= SDMT_NUM_DT) {
        return SDMT_ERR_WRONG_DATA_TYPE;
    }

    // check the definition of dimension
    if (dim.size() != (int)dt) {
        return SDMT_ERR_WRONG_DIMENSION;
    }

    // calculate the byte size of memory to allocate
    size_t size = 1;
    for (auto d: dim) {
        size *= d;
    }

    // allocate memory and register to checkpointing module
    void* p = nullptr;
    int esize;
    try {
        switch (vt) {
            case SDMT_INT:
                esize = sizeof(int);
                p = std::malloc(size * esize);
                FTI_Protect(m_cp_idx, p, size, FTI_INTG);
                break;
            case SDMT_LONG:
                esize = sizeof(long);
                p = std::malloc(size * esize);
                FTI_Protect(m_cp_idx, p, size, FTI_LONG);
                break;
            case SDMT_FLOAT:
                esize = sizeof(float);
                p = std::malloc(size * esize);
                FTI_Protect(m_cp_idx, p, size, FTI_SFLT);
                break;
            case SDMT_DOUBLE:
                esize = sizeof(double);
                p = std::malloc(size * esize);
                FTI_Protect(m_cp_idx, p, size, FTI_DBLE);
                break;
            default:
                return SDMT_ERR_WRONG_VALUE_TYPE;
        }
    } catch (...) {
        return SDMT_ERR_FAILED_ALLOCATION;
    }

    // register to sdmt manager
    if (p) {
        m_sgmt_map[name] = Segment(m_cp_idx++, vt, dt, dim, esize, p);
        return SDMT_SUCCESS;
    } else {
        return SDMT_ERR_FAILED_ALLOCATION;
    }
}

SDMT_Code SDMT::checkpoint_() {
    int res = FTI_Checkpoint(m_cp_info.id, m_cp_info.level);
    if (res == 0) {
        m_cp_info.level = (m_cp_info.level + 1) % 5;
        m_cp_info.id++;

        return SDMT_SUCCESS;
    }

    return SDMT_ERR_FAILED_CHECKPOINT;
}

SDMT_Code SDMT::recover_() {
    FTI_Recover();
    return SDMT_SUCCESS;
}

SDMT::Segment* SDMT::get_segment_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr == m_sgmt_map.end()) {
        return nullptr;
    }

    return &(itr->second);
}

int* SDMT::intptr_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr == m_sgmt_map.end()) {
        return nullptr;
    }

    return reinterpret_cast<int*>(itr->second.m_ptr);
}

long* SDMT::longptr_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr == m_sgmt_map.end()) {
        return nullptr;
    }

    return reinterpret_cast<long*>(itr->second.m_ptr);
}

float* SDMT::floatptr_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr == m_sgmt_map.end()) {
        return nullptr;
    }

    return reinterpret_cast<float*>(itr->second.m_ptr);
}

double* SDMT:: doubleptr_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr == m_sgmt_map.end()) {
        return nullptr;
    }

    return reinterpret_cast<double*>(itr->second.m_ptr);
}
