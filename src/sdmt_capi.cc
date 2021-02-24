#include <iostream>
#include "sdmt.h"
#include <algorithm>
#include <fstream>
using namespace std;


SDMT_Code sdmt_init(char *config, bool &restart) {
//    cout << "[SDMT] [C API] init" << endl;
	return SDMT::init(config, restart);
}

SDMT_Code sdmt_start() {
//    cout << "[SDMT] [C API] start" << endl;
    return SDMT::start();
}

SDMT_Code sdmt_finalize() {
//    cout << "[SDMT] [C API] finalize" << endl;
   return SDMT::finalize();
}

SDMT_Code sdmt_register_segment(char* name, SDMT_VT& vt, SDMT_DT& dt, std::vector<int>& dim) {
//	cout << "[SDMT] [C API] register_segment" << endl;
	return SDMT::register_segment(name, vt, dt, dim);
}

SDMT_Code sdmt_register_int_parameter(char* name, int& value){
//	cout << "[SDMT] [C API] register_int_parameter" << endl;
	return SDMT::register_int_parameter(name, value);
}

int sdmt_get_int_parameter(char* name){
//	cout << "[SDMT] [C API] get_int_parameter" << endl;
	return SDMT::get_int_parameter(name);
}

SDMT_Code sdmt_register_long_parameter(char* name, long& value){
//	cout << "[SDMT] [C API] register_long_parameter" << endl;
	return SDMT::register_long_parameter(name, value);
}

long sdmt_get_long_parameter(char* name){
//	cout << "[SDMT] [C API] get_long_parameter" << endl;
	return SDMT::get_long_parameter(name);
}

SDMT_Code sdmt_register_float_parameter(char* name, float& value){
//	cout << "[SDMT] [C API] register_float_parameter" << endl;
	return SDMT::register_float_parameter(name, value);
}

float sdmt_get_float_parameter(char* name){
//	cout << "[SDMT] [C API] get_float_parameter" << endl;
	return SDMT::get_float_parameter(name);
}

SDMT_Code sdmt_register_double_parameter(char* name, double& value){
//	cout << "[SDMT] [C API] register_double_parameter" << endl;
	return SDMT::register_double_parameter(name, value);
}

double sdmt_get_double_parameter(char* name){
//	cout << "[SDMT] [C API] get_double_parameter" << endl;
	return SDMT::get_double_parameter(name);
}

SDMT_Code sdmt_checkpoint(int& level) {
//    cout << "[SDMT] [C API] checkpoint" << endl;
    return SDMT::checkpoint(level);
}

SDMT_Code sdmt_recover() {
//    cout << "[SDMT] [C API] recover" << endl;
    return SDMT::recover();
}

bool sdmt_exist(char* name){
//	cout << "[SDMT] [C API] exist" << endl;
	return SDMT::exist(name);
}

SDMT::Segment sdmt_get_segment(char* name){
//	cout << "[SDMT] [C API] get_segment" << endl;
	return SDMT::get_segment(name);
}

int32_t sdmt_iter(){
//	cout << "[SDMT] [C API] iter" << endl;
	return SDMT :: iter();
}

int32_t sdmt_next(){
//	cout << "[SDMT] [C API] next" << endl;
	return SDMT::next();
}

MPI_Comm sdmt_comm(){
	cout << "[SDMT] [C API] comm" << endl;
	return SDMT::comm();
}

int* sdmt_intptr(char* name) {
//    cout << "[SDMT] [C API] intptr" << endl;
    
	
	return SDMT::intptr(name);
}

long* sdmt_longptr(char* name){
//	cout << "[SDMT] [C API] longptr" << endl;
	return SDMT::longptr(name);
}

float* sdmt_floatptr(char* name){
//	cout << "[SDMT] [C API] floatptr" << endl;
	return SDMT::floatptr(name);
}

double* sdmt_doubleptr(char* name){
//	cout << "[SDMT] [C API] doubleptr" << endl;
	return SDMT::doubleptr(name);
}




/*
bool sdmt_load_config(std::string config){
	cout << "[SDMT] [C API] load config" << endl;
	return SDMT::load_config(config);
}

bool sdmt_serialize(){
	cout << "[SDMT] [C API] serialize" << endl;
	return SDMT::serialize();
}

bool sdmt_deserialize(){
	cout << "[SDMT] [C API] deserialize" << endl;
	return SDMT::deserialize();
}
*/


extern "C" {
	
	typedef SDMT_VT sdmt_vt;
	typedef SDMT_DT sdmt_dt;
	typedef SDMT_Code sdmt_code;
	typedef SDMT::Segment segment;
	
	typedef int* int_p;
	typedef long* long_p;
	typedef float* float_p;
	typedef double* double_p;
	
	typedef MPI_Comm mpi_comm;

	sdmt_code sdmt_init_c_(char *config, bool* restart) {
		return sdmt_init(config, *restart);
    }

    sdmt_code sdmt_start_c_() {
        return sdmt_start();
    }
    
	sdmt_code sdmt_finalize_c_() {
        return sdmt_finalize();
    }

    sdmt_code sdmt_register_segment_c_(char* name, SDMT_VT* vt, SDMT_DT* dt, int* dim_numpara, int dim_format[]) {
		std::vector<int> dim_;
		for(int i = 0; i< *dim_numpara; ++i){
			dim_.push_back((dim_format)[i]);
		}
		return sdmt_register_segment(name, *vt, *dt, dim_);
    }

	sdmt_code sdmt_register_int_parameter_c_(char* name, int* value){
		return sdmt_register_int_parameter(name, *value);
	}
	int sdmt_get_int_parameter_c_(char* name){
		return sdmt_get_int_parameter(name);
	}
	sdmt_code sdmt_register_long_parameter_c_(char* name, long* value){
		return sdmt_register_long_parameter(name, *value);
	}
	long sdmt_get_long_parameter_c_(char* name){
		return sdmt_get_long_parameter(name);
	}
	sdmt_code sdmt_register_float_parameter_c_(char* name, float* value){
		return sdmt_register_float_parameter(name, *value);
	}
	float sdmt_get_float_parameter_c_(char* name){
		return sdmt_get_float_parameter(name);
	}
	sdmt_code sdmt_register_double_parameter_c_(char* name, double* value){
		return sdmt_register_double_parameter(name, *value);
	}
	double sdmt_get_double_parameter_c_(char* name){
		return sdmt_get_double_parameter(name);
	}
	sdmt_code sdmt_checkpoint_c_(int *level) {
		return sdmt_checkpoint(*level);
	}
	sdmt_code sdmt_recover_c_() {
		return sdmt_recover();
	}
	bool sdmt_exist_c_(char* name){
		return sdmt_exist(name);
	}
	segment sdmt_get_segment_c_(char* name){
		return sdmt_get_segment(name);
	}
	int32_t sdmt_iter_c_(){
		return sdmt_iter();
	}
	int32_t sdmt_next_c_(){
		return sdmt_next();
	}
	mpi_comm sdmt_comm_c_(){
		return sdmt_comm();
	}
	int_p sdmt_intptr_c_(char* name) {
		return sdmt_intptr(name);
    }
	long_p sdmt_longptr_c_(char* name){
		return sdmt_longptr(name);
	}
	float_p sdmt_floatptr_c_(char* name){
		return sdmt_floatptr(name);
	}
	double_p sdmt_doubleptr_c_(char* name){
		return sdmt_doubleptr(name);
	}

	int_p sdmt_intmat_c_(void* vptr){
	
	
	}
	/*
	bool sdmt_load_config_c_(std::string config){
		return sdmt_load_config(config);
	}
	bool sdmt_serialize_c_(){
		return sdmt_serialize();	
	}
	bool sdmt_deserialize_c_(){
		return sdmt_deserialize();
	}
	*/
}
