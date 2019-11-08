// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#include "sdmt.h"
#include "tinyxml2.h"

#include <fti.h>
#include <fstream>

SDMT_Code SDMT::init_(std::string config, bool restart) {
    if (!load_config_(config)) {
        return SDMT_ERR_WRONG_CONFIG;
    }

    MPI_Init(nullptr, nullptr);
    FTI_Init(m_config.m_fti_config.c_str(), MPI_COMM_WORLD);
    m_comm = FTI_COMM_WORLD;

    if (restart && FTI_Status()) {
        // recover from previous archive
        deserialize_();
    } else {
        // normal init
        // checkpoint id is assigned as follows
        // 0: checkpoint info
        // 1: iteration sequence
        FTIT_type ckptInfo;
        FTI_InitType(&ckptInfo, 2*sizeof(int));
        FTI_Protect(m_cp_idx++, &m_cp_info, 1, ckptInfo);
        FTI_Protect(m_cp_idx++, &m_iter, 1, FTI_INTG);
    }

    return SDMT_SUCCESS;
}

SDMT_Code SDMT::start_() {
    MPI_Barrier(m_comm);
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

    if (p) {
        // register to sdmt manager
        m_sgmt_map[name] = Segment(m_cp_idx++, vt, dt, dim, esize, p);

        // log current status
        serialize_();

        return SDMT_SUCCESS;
    } else {
        return SDMT_ERR_FAILED_ALLOCATION;
    }
}

SDMT_Code SDMT::checkpoint_(int level) {
    // 0 level : frequency checkpoint
    if ( level == 0 ) {
        int res = FTI_Snapshot();
        if (res == 0) {

            // log current status
            serialize_();
            return SDMT_SUCCESS;
        }       
    }
    else if ( level > 4 ) {
        return SDMT_ERR_FAILED_CHECKPOINT;
    }
    // other level : level 1~4 checkpoint
    else {
        int res = FTI_Checkpoint(m_cp_info.id, level);
        if (res == 0) {
              m_cp_info.id++;

            // log current status
            serialize_();

            return SDMT_SUCCESS;
        }
    }
    return SDMT_ERR_FAILED_CHECKPOINT;
}

SDMT_Code SDMT::recover_() {
    FTI_Recover();
    return SDMT_SUCCESS;
}

bool SDMT::exist_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr == m_sgmt_map.end()) {
        return false;
    } else {
        return true;
    }
}

SDMT::Segment SDMT::get_segment_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr == m_sgmt_map.end()) {
        return Segment();
    }

    return itr->second;
}

int32_t& SDMT::iter_() {
    return m_iter;
}

int32_t SDMT::next_() {
    return ++m_iter;
}

MPI_Comm SDMT::comm_() {
    return m_comm;
}


int* SDMT::intptr_(std::string name) {
    auto itr = m_sgmt_map.find(name);
    if (itr== m_sgmt_map.end()) {
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

bool SDMT::load_config_(std::string config) { 
    namespace xml = tinyxml2;
    xml::XMLDocument doc;
    xml::XMLError err;
    xml::XMLNode* node;
    xml::XMLElement* element;

    // load config file
    err = doc.LoadFile(config.c_str());
    if (err != 0) {
        return false;
    }

    // get root node
    node = doc.FirstChild();
    
    // get path for sdmt archive
    element = node->FirstChildElement("ArchivePath");
    if (element == nullptr) {
        return false;
    } else {
        m_config.m_archive = element->GetText();
    }

    // get path for fti_config
    element = node->FirstChildElement("FTIConfig");
    if (element == nullptr) {
        return false;
    } else {
        m_config.m_fti_config = element->GetText();
    }

    return true;
}

bool SDMT::serialize_() {
    std::ofstream archive(m_config.m_archive,
                    std::ios::out | std::ios::binary | std::ios::trunc);

    serialize::write(archive, m_sgmt_map);
    serialize::write(archive, m_cp_info);
    serialize::write(archive, m_cp_idx);

    archive.flush();
    archive.close();

    return true;
}

bool SDMT::deserialize_() {
    std::ifstream archive(m_config.m_archive,
                    std::ios::in | std::ios::binary);

    if (!archive.good()) {
        return false;
    }
    
    // recover segment map
    serialize::read(archive, m_sgmt_map);
    serialize::read(archive, m_cp_info);
    serialize::read(archive, m_cp_idx);

    // recover checkpoint info
    FTIT_type ckptInfo;
    FTI_InitType(&ckptInfo, 2*sizeof(int));
    FTI_Protect(0, &m_cp_info, 1, ckptInfo);

    // recover iteration sequence
    FTI_Protect(1, &m_iter, 1, FTI_INTG);

    for (auto& itr : m_sgmt_map) {
        Segment& sgmt = itr.second;

        // calculate the byte size of memory to allocate
        size_t size = 1;
        for (auto d: sgmt.m_dimension) {
            size *= d;
        }

        // allocate memory and register to checkpointing module
        sgmt.m_ptr = std::malloc(size * sgmt.m_esize);

        switch (sgmt.m_valuetype) {
            case SDMT_INT:
                FTI_Protect(sgmt.m_id, sgmt.m_ptr, size, FTI_INTG);
                break;
            case SDMT_LONG:
                FTI_Protect(sgmt.m_id, sgmt.m_ptr, size, FTI_LONG);
                break;
            case SDMT_FLOAT:
                FTI_Protect(sgmt.m_id, sgmt.m_ptr, size, FTI_SFLT);
                break;
            case SDMT_DOUBLE:
                FTI_Protect(sgmt.m_id, sgmt.m_ptr, size, FTI_DBLE);
                break;
            default:
                return false;
        }
    }

    // recover snapshot
    recover_();

    return true;
}

template<>
inline std::ostream& serialize::write<SDMT::Segment>(
                        std::ostream& os, SDMT::Segment& sgmt) {
    serialize::write(os, sgmt.m_id);
    serialize::write(os, sgmt.m_valuetype);
    serialize::write(os, sgmt.m_datatype);
    serialize::write(os, sgmt.m_dimension);
    serialize::write(os, sgmt.m_strides);
    serialize::write(os, sgmt.m_esize);

    return os;
}

template<>
inline std::istream& serialize::read<SDMT::Segment>(
                        std::istream& is, SDMT::Segment& sgmt) {
    serialize::read(is, sgmt.m_id);
    serialize::read(is, sgmt.m_valuetype);
    serialize::read(is, sgmt.m_datatype);
    serialize::read(is, sgmt.m_dimension);
    serialize::read(is, sgmt.m_strides);
    serialize::read(is, sgmt.m_esize);

    return is;
}

template<>
inline std::ostream& serialize::write<SDMT::CkptInfo>(
                        std::ostream& os, SDMT::CkptInfo& cp_info) {
    serialize::write(os, cp_info.id);
    serialize::write(os, cp_info.level);
    return os;
}

template<>
inline std::istream& serialize::read<SDMT::CkptInfo>(
                        std::istream& is, SDMT::CkptInfo& cp_info) {
    serialize::read(is, cp_info.id);
    serialize::read(is, cp_info.level);
    return is;
}
