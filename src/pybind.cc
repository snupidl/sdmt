// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

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
        .def("register", &SDMT::register_segment)
		.def("get", &SDMT::get_segment)
        .def("exist", &SDMT::exist)
        .def("iter", &SDMT::iter)
        .def("next", &SDMT::next);

    py::enum_<SDMT_VT>(m, "vt")
        .value("int", SDMT_INT)
        .value("long", SDMT_LONG)
        .value("float", SDMT_FLOAT)
        .value("double", SDMT_DOUBLE);

    py::enum_<SDMT_DT>(m, "dt")
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

	py::class_<SDMT::Segment>(m, "segment", py::buffer_protocol())
		.def_buffer([](SDMT::Segment& sgmt) -> py::buffer_info {
			if (sgmt.m_valuetype == SDMT_INT) {
				return py::buffer_info(
					sgmt.m_ptr,
					sizeof(int),
					py::format_descriptor<int>::format(),
					sgmt.m_dimension.size(),
					sgmt.m_dimension,
					sgmt.m_strides);
			} else if (sgmt.m_valuetype == SDMT_LONG) {
				return py::buffer_info(
					sgmt.m_ptr,
					sizeof(long),
					py::format_descriptor<long>::format(),
					sgmt.m_dimension.size(),
					sgmt.m_dimension,
					sgmt.m_strides);
			} else if (sgmt.m_valuetype == SDMT_FLOAT) {
				return py::buffer_info(
					sgmt.m_ptr,
					sizeof(float),
					py::format_descriptor<float>::format(),
					sgmt.m_dimension.size(),
					sgmt.m_dimension,
					sgmt.m_strides);
			} else if (sgmt.m_valuetype == SDMT_DOUBLE) {
				return py::buffer_info(
					sgmt.m_ptr,
					sizeof(double),
					py::format_descriptor<double>::format(),
					sgmt.m_dimension.size(),
					sgmt.m_dimension,
					sgmt.m_strides);
			} else {
				return py::buffer_info();
			}
		});
}
