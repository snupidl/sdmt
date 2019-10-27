// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#ifndef SDMT_H_
#define SDMT_H_

#include "common.h"

//#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief a singleton class that provides APIs of SDMT
 * @li assign a data chunk to be managed in simulation process<p>
 * @li make snapshot checkpoint during iterative simulation
 *  and recovery intermediate results for restart<p>
 * @li collect statistics about data reference<p>
 * @author Ilju Lee, ijlee@kdb.snu.ac.kr
 * @author Jinyon Kim, jykim@kdb.snu.ac.kr
 * @author Hyerim Jeon, hrjeon@kdb.snu.ac.kr
 */
class SDMT
{
  public:
    /**
     * @breif a chunk of memory 
     *  which is the unit of allocation and management
     */
    struct Segment {
        /**
         * @brief create a null segment
         */
        Segment()
            : m_id(-1),
            m_valuetype(SDMT_NUM_VT),
            m_datatype(SDMT_NUM_DT),
            m_ptr(nullptr) {}

        /**
         * @brief SDMT::Segment constructor
         * @param int32_t id: id of segment
         * @param SDMT_VT vt: value type
         * @param SDMT_DT dt: data type
         * @param std::vector<int> dim: size of each dimension
         * @param void* ptr: assigned memory
         */
        Segment(int32_t id,
                SDMT_VT vt,
                SDMT_DT dt,
                std::vector<int> dim,
                int32_t esize,
                void* ptr)
            : m_id(id),
            m_valuetype(vt),
            m_datatype(dt),
            m_dimension(dim),
            m_esize(esize),
            m_ptr(ptr) {
            for (int i = 0; i < dim.size(); i++) {
                int s = esize; 
                for (int j = dim.size() - 1; j > i; j--) {
                    s *= dim[j];
                }
                m_strides.push_back(s);
            }
        }

        /**
         * @brief SDMT::Segment copy constructor
         * @param Segment& other: segment to copy
         */
        Segment(const Segment& other)
            : m_id(other.m_id),
            m_valuetype(other.m_valuetype),
            m_datatype(other.m_datatype),
            m_dimension(other.m_dimension),
            m_strides(other.m_strides),
            m_esize(other.m_esize),
            m_ptr(other.m_ptr) {}

        /**
         * @brief SDMT::Segment destructor
         */
        ~Segment() {}

        int32_t             m_id;
        SDMT_VT             m_valuetype;
        SDMT_DT             m_datatype;
        std::vector<int>    m_dimension;
        std::vector<int>    m_strides;
        int32_t             m_esize;
        void*               m_ptr;
    };

    /**
     * @brief checkpoint info
     */
    struct CkptInfo {
        /**
         * @brief CkptInfo constructor
         */
        CkptInfo(int id_, int level_) : id(id_), level(level_) {}

        int id;
        int level;
    };

    typedef std::unordered_map<std::string, Segment> SegmentMap;

    /**
     * @brief SDMT constructor
     */
    SDMT() : m_cp_info(1, 1), m_cp_idx(1) {}

    /**
     * @brief [static]get SDMT manager singleton
     * @return static SDMT manager
     */
    static SDMT& get_manager() {
        static SDMT manager;
        return manager;
    }

    /**
     * @brief [static]init sdmt(simulation process and data management)
     * @return status code
     */
    static SDMT_Code init()
    { return get_manager().init_(); }

    /**
     * @brief [static]start sdmt(simulation process and data management)
     * @return status code
     */
    static SDMT_Code start()
    { return get_manager().start_(); }

    /**
     * @brief [static]finalize sdmt(simulation process and data management)
     * @return status code
     */
    static SDMT_Code finalize()
    { return get_manager().finalize_(); }

    /**
     * @brief [static]register a segment to sdmt manager
     * @param std::string name: name of the segment
     * @param SDMT_VT vt: value type
     * @param SDMT_DT dt: data type
     * @param std::vector<int> dim: size of each dimension
     * @return status code
     */
    static SDMT_Code register_segment(std::string name,
                                SDMT_VT vt,
                                SDMT_DT dt,
                                std::vector<int> dim)
    { return get_manager().register_segment_(name, vt, dt, dim); }
    
    /**
     * @brief [static]create a checkpoint of a segment
     * @return status code
     */
    static SDMT_Code checkpoint()
    { return get_manager().checkpoint_(); }

    /**
     * @brief [static]recover checkpointed segments
     * @return status code
     */
    static SDMT_Code recover()
    { return get_manager().recover_(); }

    /**
     * @brief [static] get segment
     * @return segment, null if name is incorrect
     */
    static Segment* get_segment(std::string name)
    { return get_manager().get_segment_(name); }

    /**
     * @brief [static]get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return integer pointer, null if name is incorrect
     */
    static int* intptr(std::string name)
    { return get_manager().intptr_(name); }

    /**
     * @brief [static]get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return long integer pointer, null if name is incorrect
     */
    static long* longptr(std::string name)
    { return get_manager().longptr_(name); }

    /**
     * @brief [static]get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return float pointer, null if name is incorrect
     */
    static float* floatptr(std::string name)
    { return get_manager().floatptr_(name); }

    /**
     * @brief [static]get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return double pointer, null if name is incorrect
     */
    static double* doubleptr(std::string name)
    { return get_manager().doubleptr_(name); }

  private:

    /**
     * @brief init sdmt module
     * @return status code
     */
    SDMT_Code init_();

    /**
     * @brief start simulation process and data management
     * @return status code
     */
    SDMT_Code start_();
    
    /**
     * @brief finalize simulation process and data management
     * @return status code
     */
    SDMT_Code finalize_();

    /**
     * @brief register a segment to sdmt manager
     * @param std::string name: name of the segment
     * @param SDMT_VT vt: value type
     * @param SDMT_DT dt: data type
     * @param std::vector<int> dim: size of each dimension
     * @return status code
     */
    SDMT_Code register_segment_(std::string name,
                            SDMT_VT vt,
                            SDMT_DT dt,
                            std::vector<int> dim);

    /**
     * @brief create checkpoints of registered segments
     * @return status code
     */
    SDMT_Code checkpoint_();

    /**
     * @brief [static]recover checkpointed segments
     * @return status code
     */
    SDMT_Code recover_();

    /**
     * @brief get segment
     * @return segment, null if name is incorrect
     */
    Segment* get_segment_(std::string name);

    /**
     * @brief get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return integer pointer, null if name is incorrect
     */
    int* intptr_(std::string name);

    /**
     * @brief get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return long integer pointer, null if name is incorrect
     */
    long* longptr_(std::string name);

    /**
     * @brief get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return float pointer, null if name is incorrect
     */
    float* floatptr_(std::string name);

    /**
     * @brief get memory pointer of registered segment
     * @param std::string name: name of the segment
     * @return double pointer, null if name is incorrect
     */
    double* doubleptr_(std::string name);

    SegmentMap m_sgmt_map;
    CkptInfo m_cp_info;
    int32_t m_cp_idx;
};

#endif  // SDMT_H_
