#include "python_plotter.hpp"
#include "hdf5_parser.hpp"
#include <pybind11/iostream.h>

namespace py = pybind11;
using namespace pybind11::literals;

void create_scatter_plot(const std::vector<double>& x, const std::vector<double>& y, const std::string& title, const std::string& xaxis_title, const std::string& yaxis_title) {
    try {
        py::module_ plotly_go = py::module_::import("plotly.graph_objects");
        
        py::dict trace_args;
        trace_args["x"] = x;
        trace_args["y"] = y;
        trace_args["mode"] = "markers";

        py::object trace = plotly_go.attr("Scatter")(**trace_args);
        
        py::list data;
        data.append(trace);
        
        py::object fig = plotly_go.attr("Figure")(data);
        
        fig.attr("update_layout")(
            "title"_a=title,
            "xaxis_title"_a=xaxis_title,
            "yaxis_title"_a=yaxis_title
        );
        fig.attr("show")();
    } catch (py::error_already_set &e) {
        py::print(e.what());
    }
}

PYBIND11_MODULE(python_plotter, m) {
    m.doc() = "pybind11 plotter plugin";

    py::class_<HDF5Parser::DataStorage>(m, "DataStorage")
        .def(py::init<>())
        .def_readonly("data", &HDF5Parser::DataStorage::data);

    py::class_<HDF5Parser>(m, "HDF5Parser")
        .def(py::init<const std::string&>())
        .def("parse", &HDF5Parser::parse);

    m.def("create_scatter_plot", &create_scatter_plot, "Creates a scatter plot",
          "x"_a, "y"_a, "title"_a="Scatter Plot", "xaxis_title"_a="X", "yaxis_title"_a="Y");
    
    py::add_ostream_redirect(m, "ostream_redirect");
}