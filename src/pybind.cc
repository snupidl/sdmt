/**
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
 */

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
        .def("register", &SDMT::register_snapshot)
	.def("change", &SDMT::change_snapshot)
	.def("compress_lossy", &SDMT::compress_snapshot_lossy)
	.def("compress_huf", &SDMT::compress_snapshot_huf)
	.def("compress_rle", &SDMT::compress_snapshot_rle)
	.def("decompress_huf", &SDMT::decompress_snapshot_huf)
	.def("decompress_rle", &SDMT::decompress_snapshot_rle)
		.def("register_int", &SDMT::register_int_parameter)
		.def("register_long", &SDMT::register_long_parameter)
		.def("register_float", &SDMT::register_float_parameter)
		.def("register_double", &SDMT::register_double_parameter)
		.def("get", &SDMT::get_snapshot)
		.def("get_int", &SDMT::get_int_parameter)
		.def("get_long", &SDMT::get_long_parameter)
		.def("get_float", &SDMT::get_float_parameter)
		.def("get_double", &SDMT::get_double_parameter)
		.def("change_snapshot", &SDMT::change_snapshot)
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

	py::class_<SDMT::Snapshot>(m, "snapshot", py::buffer_protocol())
		.def_buffer([](SDMT::Snapshot& snapshot) -> py::buffer_info {
			if (snapshot.m_valuetype == SDMT_INT) {
				return py::buffer_info(
					snapshot.m_ptr,
					sizeof(int),
					py::format_descriptor<int>::format(),
					snapshot.m_dimension.size(),
					snapshot.m_dimension,
					snapshot.m_strides);
			} else if (snapshot.m_valuetype == SDMT_LONG) {
				return py::buffer_info(
					snapshot.m_ptr,
					sizeof(long),
					py::format_descriptor<long>::format(),
					snapshot.m_dimension.size(),
					snapshot.m_dimension,
					snapshot.m_strides);
			} else if (snapshot.m_valuetype == SDMT_FLOAT) {
				return py::buffer_info(
					snapshot.m_ptr,
					sizeof(float),
					py::format_descriptor<float>::format(),
					snapshot.m_dimension.size(),
					snapshot.m_dimension,
					snapshot.m_strides);
			} else if (snapshot.m_valuetype == SDMT_DOUBLE) {
				return py::buffer_info(
					snapshot.m_ptr,
					sizeof(double),
					py::format_descriptor<double>::format(),
					snapshot.m_dimension.size(),
					snapshot.m_dimension,
					snapshot.m_strides);
			} else {
				return py::buffer_info();
			}
		});
}
