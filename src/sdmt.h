// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#ifndef SDMT_H_
#define SDMT_H_

#include "common.h"
#include "serialization.h"

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
     * @brief a chunk of memory 
     *  which is the unit of allocation and management
     */
    struct Segment {
        /**
         * @brief create a null Segment
         */
        Segment()
            : m_id(-1),
            m_valuetype(SDMT_NUM_VT),
            m_datatype(SDMT_NUM_DT),
            m_ptr(nullptr) {}

        /**
         * @brief SDMT::Segment constructor
         * @param id id of Segment
         * @param vt value type
         * @param dt data type
         * @param dim size of each dimension
         * @param esize size of an element
         * @param ptr assigned memory
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
            for (unsigned int i = 0; i < dim.size(); i++) {
                int s = esize; 
                for (unsigned int j = dim.size() - 1; j > i; j--) {
                    s *= dim[j];
                }
                m_strides.push_back(s);
            }
        }

        /**
         * @brief SDMT::Segment copy constructor
         * @param other Segment to copy
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

        /** @brief id of Segment*/
        int32_t m_id;

        /** @brief value type of elements in Segment */
        SDMT_VT m_valuetype;

        /** @brief type of data structure */
        SDMT_DT m_datatype;

        /** @brief dimension of data */
        std::vector<int> m_dimension;

        /** @brief byte strides for each index */
        std::vector<int> m_strides;

        /** @brief byte size of an element in Segment */
        int32_t m_esize;

        /** @brief memory pointer of data */
        void* m_ptr;
    };

    /**
     * @brief configurations
     */
    struct Config {
        /** @brief path to archive sdmt manager */
        std::string m_archive;
        /** @brief path to fti lib configuration file */
        std::string m_fti_config;
    };

    /**
     * @brief checkpoint info
     */
    struct CkptInfo {
        /**
         * @brief CkptInfo constructor
         */
        CkptInfo(int id_, int level_) : id(id_), level(level_) {}

        /** @brief checkpoint id */
        int id;

        /** @brief checkpoint level */
        int level;
    };

    /**
     * @brief hash map that mapping name of Segment and Segment
     */
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
     * @param config path of config file
     * @param restart if archive exists, recover manager
     * @return status code
     */
    static SDMT_Code init(std::string config, bool restart)
    { return get_manager().init_(config, restart); }

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
     * @brief [static]register a Segment to sdmt manager
     * @param name name of the Segment
     * @param vt value type
     * @param dt data type
     * @param dim size of each dimension
     * @return status code
     */
    static SDMT_Code register_segment(std::string name,
                                SDMT_VT vt,
                                SDMT_DT dt,
                                std::vector<int> dim)
    { return get_manager().register_segment_(name, vt, dt, dim); }
    
    /**
     * @brief [static]create a checkpoint of a Segment
     * @return status code
     */
    static SDMT_Code checkpoint()
    { return get_manager().checkpoint_(); }

    /**
     * @brief [static]recover checkpointed Segments
     * @return status code
     */
    static SDMT_Code recover()
    { return get_manager().recover_(); }

    /**
     * @brief [static] get Segment
     * @return Segment, null if name is incorrect
     */
    static Segment get_segment(std::string name)
    { return get_manager().get_segment_(name); }

    /**
     * @brief [static]get memory pointer of registered Segment
     * @param name name of the Segment
     * @return integer pointer, null if name is incorrect
     */
    static int* intptr(std::string name)
    { return get_manager().intptr_(name); }

    /**
     * @brief [static]get memory pointer of registered Segment
     * @param name name of the Segment
     * @return long integer pointer, null if name is incorrect
     */
    static long* longptr(std::string name)
    { return get_manager().longptr_(name); }

    /**
     * @brief [static]get memory pointer of registered Segment
     * @param name name of the Segment
     * @return float pointer, null if name is incorrect
     */
    static float* floatptr(std::string name)
    { return get_manager().floatptr_(name); }

    /**
     * @brief [static]get memory pointer of registered Segment
     * @param name name of the Segment
     * @return double pointer, null if name is incorrect
     */
    static double* doubleptr(std::string name)
    { return get_manager().doubleptr_(name); }

  private:
    /**
     * @brief init sdmt module
     * @param config path of config file
     * @param restart if archive exists, recover manager
     * @return status code
     */
    SDMT_Code init_(std::string config, bool restart);

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
     * @brief register a Segment to sdmt manager
     * @param name name of the Segment
     * @param vt value type
     * @param dt data type
     * @param dim size of each dimension
     * @return status code
     */
    SDMT_Code register_segment_(std::string name,
                            SDMT_VT vt,
                            SDMT_DT dt,
                            std::vector<int> dim);

    /**
     * @brief create checkpoints of registered Segment
     * @return status code
     */
    SDMT_Code checkpoint_();

    /**
     * @brief [static]recover checkpointed Segment
     * @return status code
     */
    SDMT_Code recover_();

    /**
     * @brief get Segment
     * @return Segment, null if name is incorrect
     */
    Segment get_segment_(std::string name);

    /**
     * @brief get memory pointer of registered Segment
     * @param name name of the Segment
     * @return integer pointer, null if name is incorrect
     */
    int* intptr_(std::string name);

    /**
     * @brief get memory pointer of registered Segment
     * @param name name of the Segment
     * @return long integer pointer, null if name is incorrect
     */
    long* longptr_(std::string name);

    /**
     * @brief get memory pointer of registered Segment
     * @param name name of the Segment
     * @return float pointer, null if name is incorrect
     */
    float* floatptr_(std::string name);

    /**
     * @brief get memory pointer of registered Segment
     * @param name name of the Segment
     * @return double pointer, null if name is incorrect
     */
    double* doubleptr_(std::string name);

    /**
     * @brief load configuration file
     * @param config path of config file
     * @return true if success
     */
    bool load_config_(std::string path);

    /**
     * @brief serialize sdmt manager whenever an update occurs
     * @return true if success
     */
    bool serialize_();

    /**
     * @brief deserialize sdmt manager whenever an update occurs
     * @return true if success
     */
    bool deserialize_();
    
    /** @brief hash map of Segmen */
    SegmentMap m_sgmt_map;

    /** @brief configurations */
    Config m_config;

    /** @brief global checkpoint info */
    CkptInfo m_cp_info;

    /** @brief incremental id of checkpoint */
    int32_t m_cp_idx;
};

/**
 * @brief Segment serialization
 * @details specification of template function in serialization.h
 */
template<>
inline std::ostream& serialize::write<SDMT::Segment>(
                        std::ostream& os, SDMT::Segment& sgmt);

/**
 * @brief SDMT::Segment deserialization
 * @details specification of template function in serialization.h
 */
template<>
inline std::istream& serialize::read<SDMT::Segment>(
                        std::istream& is, SDMT::Segment& sgmt);

/**
 * @brief SDMT::CkptInfo serialization
 * @details specification of template function in serialization.h
 */
template<>
inline std::ostream& serialize::write<SDMT::CkptInfo>(
                        std::ostream& os, SDMT::CkptInfo& cp_info);

/**
 * @brief SDMT::CkptInfo deserialization
 * @details specification of template function in serialization.h
 */
template<>
inline std::istream& serialize::read<SDMT::CkptInfo>(
                        std::istream& is, SDMT::CkptInfo& cp_info);

#endif  // SDMT_H_
