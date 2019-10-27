#include "sdmt.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

PYBIND11_MODULE(sdmtpy, m) {
    m.doc() = "Simulation process and Data Management Tool";

    m.def("init", &SDMT::init)
        .def("start", &SDMT::start)
        .def("finalize", &SDMT::finalize)
        .def("checkpoint", &SDMT::checkpoint)
        .def("recover", &SDMT::recover)
        .def("register", &SDMT::register_segment);

    py::enum_<SDMT_VT>(m, "value_type")
        .value("int", SDMT_INT)
        .value("long", SDMT_LONG)
        .value("float", SDMT_FLOAT)
        .value("double", SDMT_DOUBLE);

    py::enum_<SDMT_DT>(m, "data_type")
        .value("scalar", SDMT_SCALAR)
        .value("array", SDMT_ARRAY)
        .value("matrix", SDMT_MATRIX)
        .value("tensor", SDMT_TENSOR);

    py::enum_<SDMT_Code>(m, "sdmt_code")
        .value("success", SDMT_Code::SDMT_SUCCESS)
        .value("err_duplicated_name", SDMT_ERR_DUPLICATED_NAME)
        .value("err_wrong_value_type", SDMT_ERR_WRONG_VALUE_TYPE)
        .value("err_wrong_data_type", SDMT_ERR_WRONG_DATA_TYPE)
        .value("err_wrong_dimension", SDMT_ERR_WRONG_DIMENSION)
        .value("err_failed_allocation", SDMT_ERR_FAILED_ALLOCATION)
        .value("err_failed_checkpoint", SDMT_ERR_FAILED_CHECKPOINT);
	
	m.def("intdata", [](std::string name) {
		SDMT::Segment* sgmt = SDMT::get_segment(name);
		return py::array_t<int>(
			sgmt->m_dimension,
			sgmt->m_strides,
			reinterpret_cast<int*>(sgmt->m_ptr));
	}).def("longdata", [](std::string name) {
		SDMT::Segment* sgmt = SDMT::get_segment(name);
		return py::array_t<long>(
			sgmt->m_dimension,
			sgmt->m_strides,
			reinterpret_cast<long*>(sgmt->m_ptr)); 
	}).def("floatdata", [](std::string name) {
		SDMT::Segment* sgmt = SDMT::get_segment(name);
		return py::array_t<float>(
			sgmt->m_dimension,
			sgmt->m_strides,
			reinterpret_cast<float*>(sgmt->m_ptr)); 
	}).def("doubledata", [](std::string name) {
		SDMT::Segment* sgmt = SDMT::get_segment(name);
		return py::array_t<double>(
			sgmt->m_dimension,
			sgmt->m_strides,
			reinterpret_cast<double*>(sgmt->m_ptr)); 
    });
}
