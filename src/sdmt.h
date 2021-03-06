/**
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
 */

#ifndef SDMT_H_
#define SDMT_H_

#include "common.h"
#include "serialization.h"

//#include <string>
#include <vector>
#include <unordered_map>
#include <fti.h>

/**
 * @brief a singleton class that provides APIs of SDMT
 * @li assign a data chunk to be managed in simulation process<p>
 * @li make snapshot checkpoint during iterative simulation
 *  and recovery intermediate results for restart<p>
 * @li collect statistics about data reference<p>
 */
class SDMT
{
  public:
    /**
     * @brief a chunk of memory 
     *  which is the unit of allocation and management
     */
    struct Snapshot {
        /**
         * @brief create a null Snapshot
         */
        Snapshot()
            : m_id(-1),
            m_valuetype(SDMT_NUM_VT),
            m_datatype(SDMT_NUM_DT),
            m_ptr(nullptr) {}

        /**
         * @brief SDMT::Snapshot constructor
         * @param id id of Snapshot
         * @param vt value type
         * @param dt data type
         * @param dim size of each dimension
         * @param esize size of an element
         * @param ptr assigned memory
         */
        Snapshot(int32_t id,
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
         * @brief SDMT::Snapshot copy constructor
         * @param other Snapshot to copy
         */
        Snapshot(const Snapshot& other)
            : m_id(other.m_id),
            m_valuetype(other.m_valuetype),
            m_datatype(other.m_datatype),
            m_dimension(other.m_dimension),
            m_strides(other.m_strides),
            m_esize(other.m_esize),
            m_ptr(other.m_ptr) {}

        /**
         * @brief SDMT::Snapshot destructor
         */
        ~Snapshot() {}

        /** @brief id of Snapshot*/
        int32_t m_id;

        /** @brief value type of elements in Snapshot */
        SDMT_VT m_valuetype;

        /** @brief type of data structure */
        SDMT_DT m_datatype;

        /** @brief dimension of data */
        std::vector<int> m_dimension;

        /** @brief byte strides for each index */
        std::vector<int> m_strides;

        /** @brief byte size of an element in Snapshot */
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
        /** @brief path to registered parameter file */
        std::string m_param_path;
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
     * @brief hash map that mapping Snapshot'nams and Snapshot
     */
    typedef std::unordered_map<std::string, Snapshot> SnapshotMap;

    /**
     * @brief SDMT constructor
     */
    SDMT() : m_cp_info(1, 1), m_cp_idx(0), m_iter(0), m_comm(NULL) {}

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
     * @brief [static]register a Snapshot to sdmt manager
     * @param name name of the Snapshot
     * @param vt value type
     * @param dt data type
     * @param dim size of each dimension
     * @return status code
     */
    static SDMT_Code register_snapshot(std::string name,
                                SDMT_VT vt,
                                SDMT_DT dt,
                                std::vector<int> dim)
    { return get_manager().register_snapshot_(name, vt, dt, dim); }
   
    /**
     * @brief [static]register a int type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
	static SDMT_Code change_snapshot(std::string name,
                                SDMT_VT cvt,
                                SDMT_DT cdt,
                                std::vector<int> cdim)
    {return get_manager().change_snapshot_(name, cvt, cdt, cdim); }
   
    /**
     * @brief [static]register a int type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    static SDMT_Code register_int_parameter(std::string name,
				int value)
    { return get_manager().register_int_parameter_(name, value); }

    /**
     * @brief [static]get a int type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    static int get_int_parameter(std::string name)
    { return get_manager().get_int_parameter_(name); }

    /**
     * @brief [static]register a long type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    static SDMT_Code register_long_parameter(std::string name,
				long value)
    { return get_manager().register_long_parameter_(name, value); }

    /**
     * @brief [static]get a long type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    static int get_long_parameter(std::string name)
    { return get_manager().get_long_parameter_(name); }

    /**
     * @brief [static]register a float type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    static SDMT_Code register_float_parameter(std::string name,
				float value)
    { return get_manager().register_float_parameter_(name, value); }

    /**
     * @brief [static]get a float type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    static int get_float_parameter(std::string name)
    { return get_manager().get_float_parameter_(name); }

    /**
     * @brief [static]register a double type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    static SDMT_Code register_double_parameter(std::string name,
				double value)
    { return get_manager().register_double_parameter_(name, value); }

    /**
     * @brief [static]get a double type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    static int get_double_parameter(std::string name)
    { return get_manager().get_double_parameter_(name); }
 
    /**
     * @brief [static]create a checkpoint of a Snapshot
     * @param level checkpoint method
     * @return status code
     */
    static SDMT_Code checkpoint(int level)
    { return get_manager().checkpoint_(level); }

    /**
     * @brief [static]recover checkpointed Snapshots
     * @return status code
     */
    static SDMT_Code recover()
    { return get_manager().recover_(); }

    /**
     * @brief [static] check a specific snapshot exsits
     * @param name name of snapshot
     * @return true if exists
     */
    static bool exist(std::string name)
    { return get_manager().exist_(name); }

    /**
     * @brief [static] get Snapshot
     * @return Snapshot, null if name is incorrect
     */
    static Snapshot get_snapshot(std::string name)
    { return get_manager().get_snapshot_(name); }

    /**
     * @brierf get current iteration sequence
     * @return current iteration number
     */
    static int32_t& iter()
    { return get_manager().iter_(); }

    /**
     * @brierf proceeds iteration sequence
     * @return next iteration number
     */
    static int32_t next()
    { return get_manager().next_(); }

    /**
     * @brief [static]get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return integer pointer, null if name is incorrect
     */
    static int* intptr(std::string name)
    { return get_manager().intptr_(name); }

    /**
     * @brief [static]get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return long integer pointer, null if name is incorrect
     */
    static long* longptr(std::string name)
    { return get_manager().longptr_(name); }

    /**
     * @brief [static]get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return float pointer, null if name is incorrect
     */
    static float* floatptr(std::string name)
    { return get_manager().floatptr_(name); }

    /**
     * @brief [static]get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return double pointer, null if name is incorrect
     */
    static double* doubleptr(std::string name)
    { return get_manager().doubleptr_(name); }

    /**
     * @brief get MPI communicator
     * @return FTI_COMM_WORLD
     */
    static MPI_Comm comm() 
    { return get_manager().comm_(); }

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
     * @brief register a Snapshot to sdmt manager
     * @param name name of the Snapshot
     * @param vt value type
     * @param dt data type
     * @param dim size of each dimension
     * @return status code
     */
    SDMT_Code register_snapshot_(std::string name,
                            SDMT_VT vt,
                            SDMT_DT dt,
                            std::vector<int> dim);

    /**
     * @brief register a int type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
	SDMT_Code change_snapshot_(std::string name,
                            SDMT_VT cvt,
                            SDMT_DT cdt,
                            std::vector<int> cdim);

    /**
     * @brief register a int type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    SDMT_Code register_int_parameter_(std::string name,
			    int value);

    /**
     * @brief get a int type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    int get_int_parameter_(std::string name);

    /**
     * @brief register a long type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    SDMT_Code register_long_parameter_(std::string name,
			    long value);

    /**
     * @brief get a long type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    long get_long_parameter_(std::string name);


    /**
     * @brief register a float type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    SDMT_Code register_float_parameter_(std::string name,
			    float value);

    /**
     * @brief get a float type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    float get_float_parameter_(std::string name);

    /**
     * @brief register a double type parameter to be adjusted
     * @param name name of the parameter
     * @param value value of the parameter
     * @return status code
     */
    SDMT_Code register_double_parameter_(std::string name,
			    double value);

    /**
     * @brief get a double type parameter registered
     * @param name name of the parameter
     * @return parameter value
     */
    double get_double_parameter_(std::string name);

    /**
     * @brief create checkpoints of registered Snapshot
     * @param level checkpoint method
     * @return status code
     */
    SDMT_Code checkpoint_(int level);

    /**
     * @brief [static]recover checkpointed Snapshot
     * @return status code
     */
    SDMT_Code recover_();

    /**
     * @brief [static] check a specific snapshot exsits
     * @param name name of snapshot
     * @return true if exists
     */
    bool exist_(std::string name);

    /**
     * @brief get Snapshot
     * @return Snapshot, null if name is incorrect
     */
    Snapshot get_snapshot_(std::string name);

    /**
     * @brierf get current iteration sequence
     * @return current iteration number
     */
    int32_t& iter_();

    /**
     * @brierf proceeds iteration sequence
     * @return next iteration number
     */
    int32_t next_();

    /**
     * @brief get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return integer pointer, null if name is incorrect
     */
    int* intptr_(std::string name);

    /**
     * @brief get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return long integer pointer, null if name is incorrect
     */
    long* longptr_(std::string name);

    /**
     * @brief get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return float pointer, null if name is incorrect
     */
    float* floatptr_(std::string name);

    /**
     * @brief get memory pointer of registered Snapshot
     * @param name name of the Snapshot
     * @return double pointer, null if name is incorrect
     */
    double* doubleptr_(std::string name);

    /**
     * @brief get MPI communicator
     * @return FTI_COMM_WORLD
     */
    MPI_Comm comm_();

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
    SnapshotMap m_snapshot_map;

    /** @brief configurations */
    Config m_config;

    /** @brief global checkpoint info */
    CkptInfo m_cp_info;

    /** @brief incremental id of checkpoint */
    int32_t m_cp_idx;

    /** @brief iteration sequence */
    int32_t m_iter;

    /** @brief MPI communicator */
    MPI_Comm m_comm;

};

/**
 * @brief Snapshot serialization
 * @details specification of template function in serialization.h
 */
template<>
inline std::ostream& serialize::write<SDMT::Snapshot>(
                        std::ostream& os, SDMT::Snapshot& snapshot);

/**
 * @brief SDMT::Snapshot deserialization
 * @details specification of template function in serialization.h
 */
template<>
inline std::istream& serialize::read<SDMT::Snapshot>(
                        std::istream& is, SDMT::Snapshot& snapshot);

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
